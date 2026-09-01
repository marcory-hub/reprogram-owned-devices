#include "detection_save.h"

#include "board_sd.h"
#include "vespa_labels.h"
#include "esp_camera.h"
#include "esp_heap_caps.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "img_converters.h"
#include "pipeline/pipeline_perf.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
#include <inttypes.h>

static const char *TAG = "det_save";

#define DET_SAVE_QUEUE_LEN 1
#define DET_SAVE_PATH_MAX  128

static uint32_t s_save_counter = 0;
static const char *s_counter_filename = BOARD_SD_MOUNT_POINT "/detections/.seq";

typedef struct {
    uint8_t *rgb565_data;
    size_t rgb565_len;
    uint16_t width;
    uint16_t height;
    char path[DET_SAVE_PATH_MAX];
    int category;
    float score;
    uint32_t seq;
} det_save_job_t;

static QueueHandle_t s_queue;
static int64_t s_last_save_ms;
static bool s_low_space_logged;

static const char *det_class_name(int category)
{
    return vespa_labels_class_name(category);
}

static uint32_t build_primary_path(char *path, size_t path_len, int64_t uptime_ms, int category, float score)
{
    /* Legacy uptime_ms parameter kept for compatibility with earlier callers.
     * The caller must provide a reserved sequence number via the global counter
     * before calling this function; we extract it atomically here to preserve
     * the filename format and avoid races.
     */
    uint32_t seq = __atomic_add_fetch(&s_save_counter, 1, __ATOMIC_RELAXED);
    snprintf(path,
             path_len,
             BOARD_SD_MOUNT_POINT "/detections/%s_%.2f_%04u.jpg",
             det_class_name(category),
             score,
             (unsigned)seq);
    return seq;
}

static void resolve_unique_path(char *path, size_t path_len)
{
    struct stat st;
    if (stat(path, &st) != 0) {
        return;
    }

    char stem[DET_SAVE_PATH_MAX - 8];
    strncpy(stem, path, sizeof(stem) - 1);
    stem[sizeof(stem) - 1] = '\0';

    char *dot = strrchr(stem, '.');
    if (dot != NULL) {
        *dot = '\0';
    }

    for (int suffix = 2; suffix < 100; suffix++) {
        snprintf(path, path_len, "%s_%d.jpg", stem, suffix);
        if (stat(path, &st) != 0) {
            return;
        }
    }
}

static void det_save_worker(void *arg)
{
    (void)arg;
    det_save_job_t job;

    while (true) {
        if (xQueueReceive(s_queue, &job, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        int64_t t0 = pipe_perf_now_us();

        camera_fb_t fb = {
            .buf = job.rgb565_data,
            .len = job.rgb565_len,
            .width = job.width,
            .height = job.height,
            .format = PIXFORMAT_RGB565,
        };

        uint8_t *jpg_buf = NULL;
        size_t jpg_len = 0;
        if (!frame2jpg(&fb, CONFIG_BOARD_SD_JPEG_QUALITY, &jpg_buf, &jpg_len)) {
            ESP_LOGW(TAG, "frame2jpg failed");
            heap_caps_free(job.rgb565_data);
            continue;
        }

        int64_t t1 = pipe_perf_now_us();
        pipe_perf_record(PIPE_STAGE_JPEG, t1 - t0);

        FILE *file = fopen(job.path, "wb");
        if (file == NULL) {
            ESP_LOGW(TAG, "fopen failed: %s", job.path);
        } else {
            size_t written = fwrite(jpg_buf, 1, jpg_len, file);
            fclose(file);
            if (written != jpg_len) {
                ESP_LOGW(TAG, "short write %s (%u/%u)", job.path, (unsigned)written, (unsigned)jpg_len);
            } else {
                ESP_LOGI(TAG,
                         "saved %s %s %.2f",
                         job.path,
                         det_class_name(job.category),
                         job.score);
                /* Persist the last-used sequence to avoid overwriting across reboots. */
                FILE *cf = fopen(s_counter_filename, "w");
                if (cf != NULL) {
                    fprintf(cf, "%" PRIu32 "\n", job.seq);
                    fclose(cf);
                } else {
                    ESP_LOGW(TAG, "failed to write counter %s", s_counter_filename);
                }
            }
        }

        free(jpg_buf);
        heap_caps_free(job.rgb565_data);
    }
}

void det_save_init(void)
{
#if !CONFIG_BOARD_SD_SAVE_ENABLE
    return;
#else
    if (s_queue != NULL) {
        return;
    }

    s_queue = xQueueCreate(DET_SAVE_QUEUE_LEN, sizeof(det_save_job_t));
    if (s_queue == NULL) {
        ESP_LOGE(TAG, "queue create failed");
        return;
    }

    if (board_sd_is_mounted()) {
        board_sd_ensure_dir("detections");
        /* Load persisted counter if present. */
        FILE *cf = fopen(s_counter_filename, "r");
        if (cf != NULL) {
            unsigned long val = 0;
            if (fscanf(cf, "%lu", &val) == 1) {
                __atomic_store_n(&s_save_counter, (uint32_t)val, __ATOMIC_RELAXED);
                ESP_LOGI(TAG, "loaded save counter %u", (unsigned)s_save_counter);
            }
            fclose(cf);
        }
    }

    xTaskCreate(det_save_worker, "det_save", 6144, NULL, 2, NULL);
#endif
}

bool det_save_try_rgb565(int category,
                         float score,
                         const uint8_t *rgb565,
                         size_t rgb565_len,
                         uint16_t width,
                         uint16_t height)
{
#if !CONFIG_BOARD_SD_SAVE_ENABLE
    (void)category;
    (void)score;
    (void)rgb565;
    (void)rgb565_len;
    (void)width;
    (void)height;
    return false;
#else
    if (!board_sd_is_mounted() || rgb565 == NULL || rgb565_len == 0) {
        return false;
    }

    int free_pct = board_sd_free_pct();
    if (free_pct >= 0 && free_pct < 10) {
        if (!s_low_space_logged) {
            ESP_LOGW(TAG, "SD free space below 10%% (%d%%), saves disabled", free_pct);
            s_low_space_logged = true;
        }
        return false;
    }
    s_low_space_logged = false;

    int64_t now_ms = esp_timer_get_time() / 1000;
    if (s_last_save_ms > 0 &&
        (now_ms - s_last_save_ms) < (int64_t)CONFIG_BOARD_SD_SAVE_COOLDOWN_MS) {
        return false;
    }

    if (s_queue == NULL || uxQueueMessagesWaiting(s_queue) > 0) {
        ESP_LOGW(TAG, "save queue busy, skip");
        return false;
    }

    int64_t t0 = pipe_perf_now_us();

    uint8_t *copy = (uint8_t *)heap_caps_malloc(rgb565_len, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (copy == NULL) {
        ESP_LOGW(TAG, "PSRAM alloc %u failed", (unsigned)rgb565_len);
        return false;
    }
    memcpy(copy, rgb565, rgb565_len);

    det_save_job_t job = {
        .rgb565_data = copy,
        .rgb565_len = rgb565_len,
        .width = width,
        .height = height,
        .category = category,
        .score = score,
    };

    job.seq = build_primary_path(job.path, sizeof(job.path), now_ms, category, score);
    resolve_unique_path(job.path, sizeof(job.path));

    if (xQueueSend(s_queue, &job, 0) != pdTRUE) {
        heap_caps_free(copy);
        ESP_LOGW(TAG, "queue send failed");
        return false;
    }

    int64_t t1 = pipe_perf_now_us();
    pipe_perf_record(PIPE_STAGE_SD_QUEUE, t1 - t0);

    s_last_save_ms = now_ms;
    return true;
#endif
}
