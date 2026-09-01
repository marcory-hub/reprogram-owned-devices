#include "board_camera.h"

#include "esp_camera.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "img_converters.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "board_cam";

#define CAMERA_PWDN_PIN -1
#define CAMERA_RESET_PIN -1
#define CAMERA_XCLK_PIN 39
#define CAMERA_SCL_PIN 40
#define CAMERA_SDA_PIN 41
#define CAMERA_D0_PIN 20
#define CAMERA_D1_PIN 18
#define CAMERA_D2_PIN 17
#define CAMERA_D3_PIN 19
#define CAMERA_D4_PIN 21
#define CAMERA_D5_PIN 47
#define CAMERA_D6_PIN 38
#define CAMERA_D7_PIN 46
#define CAMERA_VSYNC_PIN 42
#define CAMERA_HREF_PIN 45
#define CAMERA_PCLK_PIN 48

#define BOARD_CAM_MAX_W 400
#define BOARD_CAM_MAX_H 296
#define BOARD_CAM_FULL_BGR_BYTES ((size_t)BOARD_CAM_MAX_W * BOARD_CAM_MAX_H * 3)
#define BILINEAR_FRAC_BITS 8
#define BILINEAR_FRAC_SCALE (1 << BILINEAR_FRAC_BITS)

static uint8_t *s_full_bgr = NULL;
static int s_infer_w = 256;
static int s_infer_h = 192;

void board_camera_set_infer_size(int w, int h)
{
    if (w < 2) {
        w = 2;
    } else if (w > BOARD_CAM_INFER_W_MAX) {
        w = BOARD_CAM_INFER_W_MAX;
    }
    if (h < 2) {
        h = 2;
    } else if (h > BOARD_CAM_INFER_H_MAX) {
        h = BOARD_CAM_INFER_H_MAX;
    }
    s_infer_w = w;
    s_infer_h = h;
    ESP_LOGI(TAG, "infer size set to %dx%d", s_infer_w, s_infer_h);
}

uint16_t board_camera_infer_w(void)
{
    return (uint16_t)s_infer_w;
}

uint16_t board_camera_infer_h(void)
{
    return (uint16_t)s_infer_h;
}

static void sample_bgr888(const uint8_t *src, int sw, int sh, int x, int y, uint8_t *b, uint8_t *g, uint8_t *r)
{
    if (x < 0) {
        x = 0;
    } else if (x >= sw) {
        x = sw - 1;
    }
    if (y < 0) {
        y = 0;
    } else if (y >= sh) {
        y = sh - 1;
    }
    const uint8_t *p = src + (y * sw + x) * 3;
    *b = p[0];
    *g = p[1];
    *r = p[2];
}

static inline uint8_t blend_channel(uint8_t v00, uint8_t v10, uint8_t v01, uint8_t v11, int wx, int wy)
{
    const int w00 = (BILINEAR_FRAC_SCALE - wx) * (BILINEAR_FRAC_SCALE - wy);
    const int w10 = wx * (BILINEAR_FRAC_SCALE - wy);
    const int w01 = (BILINEAR_FRAC_SCALE - wx) * wy;
    const int w11 = wx * wy;
    const int sum = v00 * w00 + v10 * w10 + v01 * w01 + v11 * w11;
    return (uint8_t)((sum + (BILINEAR_FRAC_SCALE * BILINEAR_FRAC_SCALE / 2)) / (BILINEAR_FRAC_SCALE * BILINEAR_FRAC_SCALE));
}

/**
 * Center-crop a rectangle matching infer W:H aspect, then bilinear
 * scale to infer size. Square 224 and rect 256x192 both use this path.
 */
static void bgr888_center_bilinear_to_infer(const uint8_t *src, int sw, int sh, uint8_t *dst)
{
    const int infer_w = s_infer_w;
    const int infer_h = s_infer_h;
    /* Fixed-point aspect: crop_w / crop_h == infer_w / infer_h */
    int crop_w;
    int crop_h;
    if ((int64_t)sw * infer_h > (int64_t)sh * infer_w) {
        /* Frame wider than target: limit by height. */
        crop_h = (sh * BOARD_CAM_CENTER_CROP_PCT) / 100;
        if (crop_h < 2) {
            crop_h = sh;
        }
        crop_w = (int)(((int64_t)crop_h * infer_w) / infer_h);
        if (crop_w > sw) {
            crop_w = sw;
            crop_h = (int)(((int64_t)crop_w * infer_h) / infer_w);
        }
    } else {
        /* Frame taller or equal: limit by width. */
        crop_w = (sw * BOARD_CAM_CENTER_CROP_PCT) / 100;
        if (crop_w < 2) {
            crop_w = sw;
        }
        crop_h = (int)(((int64_t)crop_w * infer_h) / infer_w);
        if (crop_h > sh) {
            crop_h = sh;
            crop_w = (int)(((int64_t)crop_h * infer_w) / infer_h);
        }
    }
    if (crop_w < 2) {
        crop_w = 2;
    }
    if (crop_h < 2) {
        crop_h = 2;
    }

    const int x0 = (sw - crop_w) / 2;
    const int y0 = (sh - crop_h) / 2;
    const int scale_x_fixed = (crop_w << BILINEAR_FRAC_BITS) / infer_w;
    const int scale_y_fixed = (crop_h << BILINEAR_FRAC_BITS) / infer_h;

    for (int y = 0; y < infer_h; y++) {
        const int fy_fixed =
            ((y0 << BILINEAR_FRAC_BITS) + (((y << 1) + 1) * scale_y_fixed) / 2) - (BILINEAR_FRAC_SCALE / 2);
        const int y1 = fy_fixed >> BILINEAR_FRAC_BITS;
        const int wy = fy_fixed & (BILINEAR_FRAC_SCALE - 1);

        for (int x = 0; x < infer_w; x++) {
            const int fx_fixed =
                ((x0 << BILINEAR_FRAC_BITS) + (((x << 1) + 1) * scale_x_fixed) / 2) - (BILINEAR_FRAC_SCALE / 2);
            const int x1 = fx_fixed >> BILINEAR_FRAC_BITS;
            const int wx = fx_fixed & (BILINEAR_FRAC_SCALE - 1);

            uint8_t b00, g00, r00, b10, g10, r10, b01, g01, r01, b11, g11, r11;
            sample_bgr888(src, sw, sh, x1, y1, &b00, &g00, &r00);
            sample_bgr888(src, sw, sh, x1 + 1, y1, &b10, &g10, &r10);
            sample_bgr888(src, sw, sh, x1, y1 + 1, &b01, &g01, &r01);
            sample_bgr888(src, sw, sh, x1 + 1, y1 + 1, &b11, &g11, &r11);

            uint8_t *o = dst + (y * infer_w + x) * 3;
            o[0] = blend_channel(b00, b10, b01, b11, wx, wy);
            o[1] = blend_channel(g00, g10, g01, g11, wx, wy);
            o[2] = blend_channel(r00, r10, r01, r11, wx, wy);
        }
    }
}

esp_err_t board_camera_init(void)
{
    if (s_full_bgr == NULL) {
        s_full_bgr = (uint8_t *)heap_caps_malloc(BOARD_CAM_FULL_BGR_BYTES, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (s_full_bgr == NULL) {
            ESP_LOGE(TAG, "full-frame PSRAM alloc %u failed", (unsigned)BOARD_CAM_FULL_BGR_BYTES);
            return ESP_ERR_NO_MEM;
        }
    }

    camera_config_t config = {
        .pin_pwdn = CAMERA_PWDN_PIN,
        .pin_reset = CAMERA_RESET_PIN,
        .pin_xclk = CAMERA_XCLK_PIN,
        .pin_sccb_sda = CAMERA_SDA_PIN,
        .pin_sccb_scl = CAMERA_SCL_PIN,
        .pin_d7 = CAMERA_D7_PIN,
        .pin_d6 = CAMERA_D6_PIN,
        .pin_d5 = CAMERA_D5_PIN,
        .pin_d4 = CAMERA_D4_PIN,
        .pin_d3 = CAMERA_D3_PIN,
        .pin_d2 = CAMERA_D2_PIN,
        .pin_d1 = CAMERA_D1_PIN,
        .pin_d0 = CAMERA_D0_PIN,
        .pin_vsync = CAMERA_VSYNC_PIN,
        .pin_href = CAMERA_HREF_PIN,
        .pin_pclk = CAMERA_PCLK_PIN,
        .xclk_freq_hz = 10000000,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,
        .pixel_format = PIXFORMAT_RGB565,
        .frame_size = FRAMESIZE_CIF,
        .jpeg_quality = 12,
        .fb_count = 2,
        .fb_location = CAMERA_FB_IN_PSRAM,
        .grab_mode = CAMERA_GRAB_LATEST,
    };

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_camera_init failed: 0x%x", (unsigned)err);
        return err;
    }

    sensor_t *s = esp_camera_sensor_get();
    if (s == NULL) {
        ESP_LOGE(TAG, "esp_camera_sensor_get null");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG,
             "sensor PID=0x%x (GC2145=0x2145) infer=%dx%d",
             (unsigned)s->id.PID,
             s_infer_w,
             s_infer_h);
    return ESP_OK;
}

void board_camera_discard_frames(int count)
{
    for (int i = 0; i < count; i++) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (fb != NULL) {
            esp_camera_fb_return(fb);
        }
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}

esp_err_t board_camera_acquire(board_camera_frame_t *out)
{
    if (out == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    out->fb = esp_camera_fb_get();
    if (out->fb == NULL) {
        ESP_LOGW(TAG, "fb_get failed");
        return ESP_FAIL;
    }
    if (out->fb->format != PIXFORMAT_RGB565 || out->fb->width < 16 || out->fb->height < 16) {
        ESP_LOGE(TAG, "bad frame fmt=%d %dx%d", (int)out->fb->format, out->fb->width, out->fb->height);
        esp_camera_fb_return(out->fb);
        out->fb = NULL;
        return ESP_FAIL;
    }
    return ESP_OK;
}

esp_err_t board_camera_crop_bgr888(const board_camera_frame_t *frame,
                                   uint8_t *dst,
                                   board_camera_crop_timing_t *timing_out)
{
    if (frame == NULL || frame->fb == NULL || dst == NULL || s_full_bgr == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    camera_fb_t *fb = frame->fb;
    const size_t rgb_bytes = (size_t)fb->width * fb->height * 3;
    if (rgb_bytes > BOARD_CAM_FULL_BGR_BYTES) {
        ESP_LOGE(TAG, "frame too large for static buffer (%dx%d)", fb->width, fb->height);
        return ESP_ERR_INVALID_SIZE;
    }

    int64_t t0 = esp_timer_get_time();
    if (!fmt2rgb888(fb->buf, fb->len, fb->format, s_full_bgr)) {
        ESP_LOGE(TAG, "fmt2rgb888 failed");
        return ESP_FAIL;
    }
    int64_t t1 = esp_timer_get_time();

    bgr888_center_bilinear_to_infer(s_full_bgr, fb->width, fb->height, dst);
    int64_t t2 = esp_timer_get_time();

    if (timing_out != NULL) {
        timing_out->convert_us = t1 - t0;
        timing_out->resize_us = t2 - t1;
    }
    return ESP_OK;
}

esp_err_t board_camera_copy_rgb565(const board_camera_frame_t *frame, uint8_t *dst, size_t dst_len)
{
    if (frame == NULL || frame->fb == NULL || dst == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (frame->fb->format != PIXFORMAT_RGB565) {
        return ESP_ERR_INVALID_ARG;
    }
    if (dst_len < frame->fb->len) {
        return ESP_ERR_INVALID_SIZE;
    }
    memcpy(dst, frame->fb->buf, frame->fb->len);
    return ESP_OK;
}

void board_camera_release(board_camera_frame_t *frame)
{
    if (frame == NULL || frame->fb == NULL) {
        return;
    }
    esp_camera_fb_return(frame->fb);
    frame->fb = NULL;
}

esp_err_t board_camera_grab_rgb888(uint8_t *dst)
{
    board_camera_frame_t frame = {0};
    esp_err_t err = board_camera_acquire(&frame);
    if (err != ESP_OK) {
        return err;
    }
    err = board_camera_crop_bgr888(&frame, dst, NULL);
    board_camera_release(&frame);
    return err;
}
