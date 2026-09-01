#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Start save worker and ensure output directory exists. */
void det_save_init(void);

/**
 * Queue async SD write from a copied RGB565 frame buffer.
 * Worker performs JPEG encode and fwrite off the hot path.
 * Returns true if a job was queued.
 */
bool det_save_try_rgb565(int category,
                         float score,
                         const uint8_t *rgb565,
                         size_t rgb565_len,
                         uint16_t width,
                         uint16_t height);

#ifdef __cplusplus
}
#endif
