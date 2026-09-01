#pragma once

#include "esp_camera.h"
#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Max supported infer dims (covers 256x192 and 224x224). */
#define BOARD_CAM_INFER_W_MAX 256
#define BOARD_CAM_INFER_H_MAX 224
#define BOARD_CAM_CENTER_CROP_PCT 100

/** Max CIF RGB565 frame bytes used for SD save staging. */
#define BOARD_CAM_MAX_RGB565_BYTES (400 * 296 * 2)

/**
 * Active infer size (set before detect_pipeline_init / camera crop).
 * Defaults to 256x192 until board_camera_set_infer_size() is called.
 */
void board_camera_set_infer_size(int w, int h);
uint16_t board_camera_infer_w(void);
uint16_t board_camera_infer_h(void);

/** Compat macros for call sites that still use compile-time names. */
#define BOARD_CAM_INFER_W board_camera_infer_w()
#define BOARD_CAM_INFER_H board_camera_infer_h()

typedef struct {
    camera_fb_t *fb;
} board_camera_frame_t;

typedef struct {
    int64_t convert_us;
    int64_t resize_us;
} board_camera_crop_timing_t;

esp_err_t board_camera_init(void);
void board_camera_discard_frames(int count);

/** Hold latest camera frame until board_camera_release(). */
esp_err_t board_camera_acquire(board_camera_frame_t *out);

/**
 * Center-crop (matching infer aspect) and scale held frame to
 * board_camera_infer_w/h BGR888 for inference.
 * Uses a static PSRAM full-frame buffer (no per-frame malloc).
 * Optional timing out params in microseconds.
 */
esp_err_t board_camera_crop_bgr888(const board_camera_frame_t *frame,
                                   uint8_t *dst,
                                   board_camera_crop_timing_t *timing_out);

void board_camera_release(board_camera_frame_t *frame);

/**
 * Copy held RGB565 frame into dst (for async SD save after release).
 * dst_len must be >= fb->len.
 */
esp_err_t board_camera_copy_rgb565(const board_camera_frame_t *frame, uint8_t *dst, size_t dst_len);

/**
 * Grab latest RGB565 frame, center aspect crop, bilinear to infer size.
 * Output is BGR888 (fmt2rgb888 order) for ESP-DL DL_IMAGE_PIX_TYPE_BGR888.
 */
esp_err_t board_camera_grab_rgb888(uint8_t *dst);

#ifdef __cplusplus
}
#endif
