#include "detect_pipeline.h"

#include "board/board_camera.h"
#include "board/board_display_power.h"
#include "board/board_lcd.h"
#include "board/board_sd.h"
#include "board/detection_save.h"
#include "board/vespa_labels.h"
#include "vespa_detect.hpp"
#include "dl_image_define.hpp"
#include "dl_image_draw.hpp"
#include "dl_image_jpeg.hpp"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "pipeline/pipeline_perf.h"
#include "sdkconfig.h"
#include <cstdio>
#include <cmath>
#include <list>
#include <type_traits>

extern const uint8_t bus_jpg_start[] asm("_binary_bus_jpg_start");
extern const uint8_t bus_jpg_end[] asm("_binary_bus_jpg_end");

static const char *TAG = "detect_pipe";

static constexpr float SAVE_SCORE_THR = CONFIG_PIPELINE_SAVE_SCORE_THR_X100 / 100.0f;
static constexpr int PIPELINE_SLOT_COUNT = 2;
static constexpr int INFER_QUEUE_LEN = 2;
static constexpr int OUTPUT_QUEUE_LEN = 2;
static constexpr size_t MAX_RESULTS_PER_FRAME = 16;
static constexpr uint32_t PERF_LOG_EVERY = 10;

static constexpr int CORE_CAPTURE = 1;
static constexpr int CORE_INFER = 0;
static constexpr int CORE_OUTPUT = 1;

typedef struct {
    uint8_t *infer_bgr;
    uint8_t *rgb565_copy;
    size_t rgb565_len;
    uint16_t frame_w;
    uint16_t frame_h;
    uint32_t seq;
    int64_t e2e_t0_us;
} pipeline_slot_t;

typedef struct {
    int category;
    float score;
    int box[4];
} detect_result_pod_t;

typedef struct {
    uint8_t slot_idx;
    uint32_t seq;
    size_t result_count;
    detect_result_pod_t results[MAX_RESULTS_PER_FRAME];
} pipeline_output_msg_t;

static_assert(std::is_trivially_copyable_v<pipeline_output_msg_t>,
              "pipeline_output_msg_t must be FreeRTOS-queue safe (no std::vector)");

static pipeline_slot_t s_slots[PIPELINE_SLOT_COUNT];
static QueueHandle_t s_infer_queue;
static QueueHandle_t s_output_queue;
static QueueHandle_t s_free_slots;
static VespaDetect *s_detect;
static float s_conf_thr = 0.70f;
static uint32_t s_seq;
static uint32_t s_consecutive_failures;
static bool s_safe_mode;

static const char *class_name(int category)
{
    return vespa_labels_class_name(category);
}

static void log_results(const char *label, const std::list<dl::detect::result_t> &detect_results)
{
    if (detect_results.empty()) {
        ESP_LOGW(TAG, "%s: 0 detections", label);
        return;
    }
    for (const auto &res : detect_results) {
        ESP_LOGI(TAG,
                 "%s [category: %d (%s), score: %f, x1: %d, y1: %d, x2: %d, y2: %d]",
                 label,
                 res.category,
                 class_name(res.category),
                 res.score,
                 res.box[0],
                 res.box[1],
                 res.box[2],
                 res.box[3]);
    }
}

static void draw_frame(const dl::image::img_t &img, const std::list<dl::detect::result_t> &detect_results)
{
    const int img_x = (BOARD_LCD_H_RES - (int)img.width) / 2;
    /* Place the image at the bottom of the screen so the status bar stays at top. */
    const int img_y = BOARD_LCD_V_RES - (int)img.height;

    /* Status text only when top score meets boot-selected confidence. */
    const float scale = 3.5f;
    const int text_h = (int)ceilf(7 * scale);
    int status_y = img_y - text_h - 2;
    if (status_y < 0) {
        status_y = 0;
    }
    board_lcd_fill_rect(0, status_y, BOARD_LCD_H_RES, text_h, 0x0000);
    if (!detect_results.empty() && detect_results.front().score >= s_conf_thr) {
        const auto &top = detect_results.front();
        char status[48];
        snprintf(status, sizeof(status), "%s %.2f", class_name(top.category), top.score);
        board_lcd_fill_rect(0, status_y, BOARD_LCD_H_RES, text_h, 0x1082);
        board_lcd_draw_text_scaled(4, status_y, status, 0xFFFF, 0x1082, scale);
    }

    const std::vector<uint8_t> box_color = {0, 255, 0};
    for (const auto &res : detect_results) {
        if (res.score < s_conf_thr) {
            continue;
        }
        dl::image::draw_hollow_rectangle(
            img, res.box[0], res.box[1], res.box[2], res.box[3], box_color, 2);
    }

    const int src_is_bgr = (img.pix_type == dl::image::DL_IMAGE_PIX_TYPE_BGR888);
    board_lcd_blit_rgb888(img_x, img_y, img.width, img.height, (const uint8_t *)img.data, src_is_bgr);

    for (const auto &res : detect_results) {
        if (res.score < s_conf_thr) {
            continue;
        }
        board_lcd_draw_rect(img_x + res.box[0],
                            img_y + res.box[1],
                            img_x + res.box[2],
                            img_y + res.box[3],
                            0x07E0);
    }
}

static const dl::detect::result_t *best_save_candidate(
    const std::list<dl::detect::result_t> &detect_results)
{
    const dl::detect::result_t *best = nullptr;
    for (const auto &res : detect_results) {
        if (res.score >= SAVE_SCORE_THR && (best == nullptr || res.score > best->score)) {
            best = &res;
        }
    }
    return best;
}

static void note_frame_failure(const char *stage)
{
    s_consecutive_failures++;
    if (s_consecutive_failures >= (uint32_t)CONFIG_PIPELINE_FAIL_THRESHOLD) {
        if (!s_safe_mode) {
            ESP_LOGW(TAG, "safe mode: %u consecutive failures (last at %s)", (unsigned)s_consecutive_failures, stage);
            s_safe_mode = true;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void note_frame_success(void)
{
    if (s_consecutive_failures > 0) {
        ESP_LOGI(TAG, "pipeline recovered after %u failures", (unsigned)s_consecutive_failures);
    }
    s_consecutive_failures = 0;
    s_safe_mode = false;
}

static bool take_free_slot(uint8_t *out_idx, TickType_t timeout)
{
    return xQueueReceive(s_free_slots, out_idx, timeout) == pdTRUE;
}

static void release_slot(uint8_t idx)
{
    xQueueSend(s_free_slots, &idx, portMAX_DELAY);
}

static void capture_prep_task(void *arg)
{
    (void)arg;

    while (true) {
        uint8_t slot_idx = 0;
        if (!take_free_slot(&slot_idx, portMAX_DELAY)) {
            continue;
        }

        pipeline_slot_t *slot = &s_slots[slot_idx];
        slot->seq = ++s_seq;
        slot->e2e_t0_us = pipe_perf_now_us();
        pipe_perf_set_seq(slot->seq);

        int64_t frame_t0 = slot->e2e_t0_us;

        board_camera_frame_t frame = {0};
        if (board_camera_acquire(&frame) != ESP_OK) {
            release_slot(slot_idx);
            note_frame_failure("capture");
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        int64_t t_cap = pipe_perf_now_us();
        pipe_perf_record(PIPE_STAGE_CAPTURE, t_cap - frame_t0);

        board_camera_crop_timing_t crop_timing{};
        if (board_camera_crop_bgr888(&frame, slot->infer_bgr, &crop_timing) != ESP_OK) {
            board_camera_release(&frame);
            release_slot(slot_idx);
            note_frame_failure("crop");
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        pipe_perf_record(PIPE_STAGE_CONVERT, crop_timing.convert_us);
        pipe_perf_record(PIPE_STAGE_RESIZE, crop_timing.resize_us);

        slot->frame_w = frame.fb->width;
        slot->frame_h = frame.fb->height;
        slot->rgb565_len = frame.fb->len;
        if (board_camera_copy_rgb565(&frame, slot->rgb565_copy, slot->rgb565_len) != ESP_OK) {
            board_camera_release(&frame);
            release_slot(slot_idx);
            continue;
        }

        board_camera_release(&frame);

        if (xQueueSend(s_infer_queue, &slot_idx, pdMS_TO_TICKS(100)) != pdTRUE) {
            release_slot(slot_idx);
            note_frame_failure("infer_queue");
        }
    }
}

static void infer_task(void *arg)
{
    (void)arg;

    while (true) {
        uint8_t slot_idx = 0;
        if (xQueueReceive(s_infer_queue, &slot_idx, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        pipeline_slot_t *slot = &s_slots[slot_idx];
        dl::image::img_t live_img = {.data = slot->infer_bgr,
                                     .width = BOARD_CAM_INFER_W,
                                     .height = BOARD_CAM_INFER_H,
                                     .pix_type = dl::image::DL_IMAGE_PIX_TYPE_BGR888};

        int64_t t0 = pipe_perf_now_us();
        auto &detect_results = s_detect->run(live_img);
        int64_t t1 = pipe_perf_now_us();
        pipe_perf_record(PIPE_STAGE_INFER, t1 - t0);

        pipeline_output_msg_t out_msg{};
        out_msg.slot_idx = slot_idx;
        out_msg.seq = slot->seq;
        out_msg.result_count = 0;
        for (const auto &res : detect_results) {
            if (out_msg.result_count >= MAX_RESULTS_PER_FRAME) {
                break;
            }
            if (res.box.size() < 4) {
                continue;
            }
            detect_result_pod_t &pod = out_msg.results[out_msg.result_count++];
            pod.category = res.category;
            pod.score = res.score;
            pod.box[0] = res.box[0];
            pod.box[1] = res.box[1];
            pod.box[2] = res.box[2];
            pod.box[3] = res.box[3];
        }

        if (xQueueSend(s_output_queue, &out_msg, pdMS_TO_TICKS(100)) != pdTRUE) {
            release_slot(slot_idx);
        }
    }
}

static void output_task(void *arg)
{
    (void)arg;

    while (true) {
        board_display_power_poll();

        pipeline_output_msg_t msg;
        if (xQueueReceive(s_output_queue, &msg, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        int64_t frame_t0 = pipe_perf_now_us();
        pipe_perf_set_seq(msg.seq);

        pipeline_slot_t *slot = &s_slots[msg.slot_idx];
        dl::image::img_t live_img = {.data = slot->infer_bgr,
                                     .width = BOARD_CAM_INFER_W,
                                     .height = BOARD_CAM_INFER_H,
                                     .pix_type = dl::image::DL_IMAGE_PIX_TYPE_BGR888};

        std::list<dl::detect::result_t> detect_results;
        for (size_t i = 0; i < msg.result_count; i++) {
            const detect_result_pod_t &pod = msg.results[i];
            dl::detect::result_t res;
            res.category = pod.category;
            res.score = pod.score;
            res.box = {pod.box[0], pod.box[1], pod.box[2], pod.box[3]};
            detect_results.push_back(std::move(res));
        }
        if (!detect_results.empty()) {
            log_results("live", detect_results);
        }

        if (board_display_is_on()) {
            int64_t lcd_t0 = pipe_perf_now_us();
            draw_frame(live_img, detect_results);
            int64_t lcd_t1 = pipe_perf_now_us();
            pipe_perf_record(PIPE_STAGE_LCD, lcd_t1 - lcd_t0);
            /* Do not kick idle timer here: live frames would reset it forever. */
        }

        const dl::detect::result_t *save_hit = best_save_candidate(detect_results);
        if (save_hit != nullptr && !s_safe_mode) {
            det_save_try_rgb565(save_hit->category,
                                save_hit->score,
                                slot->rgb565_copy,
                                slot->rgb565_len,
                                slot->frame_w,
                                slot->frame_h);
        }

        int64_t frame_t1 = pipe_perf_now_us();
        pipe_perf_record(PIPE_STAGE_OUTPUT, frame_t1 - frame_t0);
        pipe_perf_record(PIPE_STAGE_E2E, frame_t1 - slot->e2e_t0_us);

        note_frame_success();
        release_slot(msg.slot_idx);
        pipe_perf_log_if_due(PERF_LOG_EVERY);
    }
}

esp_err_t detect_pipeline_init(int model_type, float conf_thr)
{
    pipe_perf_reset();
    s_conf_thr = conf_thr;

    vespa_labels_set_model(model_type);

    int infer_w = 224;
    int infer_h = 224;
    vespa_labels_infer_size(model_type, &infer_w, &infer_h);
    board_camera_set_infer_size(infer_w, infer_h);

    const size_t infer_bytes = (size_t)infer_w * (size_t)infer_h * 3;
    for (int i = 0; i < PIPELINE_SLOT_COUNT; i++) {
        s_slots[i].infer_bgr =
            (uint8_t *)heap_caps_malloc(infer_bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        s_slots[i].rgb565_copy =
            (uint8_t *)heap_caps_malloc(BOARD_CAM_MAX_RGB565_BYTES, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (s_slots[i].infer_bgr == NULL || s_slots[i].rgb565_copy == NULL) {
            ESP_LOGE(TAG, "slot %d PSRAM alloc failed", i);
            return ESP_ERR_NO_MEM;
        }
    }

    s_infer_queue = xQueueCreate(INFER_QUEUE_LEN, sizeof(uint8_t));
    s_output_queue = xQueueCreate(OUTPUT_QUEUE_LEN, sizeof(pipeline_output_msg_t));
    s_free_slots = xQueueCreate(PIPELINE_SLOT_COUNT, sizeof(uint8_t));
    if (s_infer_queue == NULL || s_output_queue == NULL || s_free_slots == NULL) {
        ESP_LOGE(TAG, "queue create failed");
        return ESP_ERR_NO_MEM;
    }

    for (uint8_t i = 0; i < PIPELINE_SLOT_COUNT; i++) {
        xQueueSend(s_free_slots, &i, portMAX_DELAY);
    }

    auto mt = static_cast<VespaDetect::model_type_t>(model_type);
    s_detect = new VespaDetect(mt, false);

    ESP_LOGI(TAG,
             "pipeline slots=%d infer_core=%d capture/output_core=%d model=%d infer=%dx%d score=%.2f conf=%.2f nms=%.2f",
             PIPELINE_SLOT_COUNT,
             CORE_INFER,
             CORE_CAPTURE,
             model_type,
             infer_w,
             infer_h,
             CONFIG_PIPELINE_SCORE_THR_X100 / 100.0f,
             conf_thr,
             CONFIG_PIPELINE_NMS_THR_X100 / 100.0f);
    return ESP_OK;
}

esp_err_t detect_pipeline_smoke(void)
{
    dl::image::jpeg_img_t jpeg_img = {.data = (void *)bus_jpg_start,
                                      .data_len = (size_t)(bus_jpg_end - bus_jpg_start)};
    auto jpeg = dl::image::sw_decode_jpeg(jpeg_img, dl::image::DL_IMAGE_PIX_TYPE_RGB888);
    auto &jpeg_results = s_detect->run(jpeg);
    log_results("jpeg-smoke", jpeg_results);
    draw_frame(jpeg, jpeg_results);
    vTaskDelay(pdMS_TO_TICKS(2500));
    heap_caps_free(jpeg.data);

    board_camera_discard_frames(5);
    if (board_camera_grab_rgb888(s_slots[0].infer_bgr) == ESP_OK) {
        dl::image::img_t live_img = {.data = s_slots[0].infer_bgr,
                                     .width = BOARD_CAM_INFER_W,
                                     .height = BOARD_CAM_INFER_H,
                                     .pix_type = dl::image::DL_IMAGE_PIX_TYPE_BGR888};
        log_results("camera-smoke", s_detect->run(live_img));
    } else {
        ESP_LOGW(TAG, "camera-smoke: grab failed");
    }

    board_lcd_fill(0x0000);
    board_display_kick_idle_timer();
    
    // Clear any stale camera frames before pipeline starts
    board_camera_discard_frames(3);
    
    return ESP_OK;
}

void detect_pipeline_start(void)
{
    xTaskCreatePinnedToCore(capture_prep_task, "cap_prep", 8192, NULL, 5, NULL, CORE_CAPTURE);
    xTaskCreatePinnedToCore(infer_task, "infer", 12288, NULL, 6, NULL, CORE_INFER);
    xTaskCreatePinnedToCore(output_task, "output", 8192, NULL, 4, NULL, CORE_OUTPUT);
}
