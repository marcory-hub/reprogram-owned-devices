#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BOARD_LCD_H_RES 240
#define BOARD_LCD_V_RES 284
#define BOARD_LCD_STATUS_H 36

esp_err_t board_lcd_init(void);

/** Enable or disable panel draw paths (backlight handled separately). */
void board_lcd_set_draw_enabled(bool enabled);
bool board_lcd_is_draw_enabled(void);

/** ST7789 panel sleep/wake (backlight unchanged). */
esp_err_t board_lcd_set_panel_on(bool on);

/** Fill entire panel with RGB565 color (host endian; swapped for SPI internally). */
void board_lcd_fill(uint16_t rgb565);

/** Fill axis-aligned rect (inclusive x2/y2 clipped). */
void board_lcd_fill_rect(int x, int y, int w, int h, uint16_t rgb565);

/**
 * Blit 888 image (row-major) to panel at (dst_x, dst_y).
 * src_is_bgr: 0 = RGB888 (jpeg-smoke), 1 = BGR888 (live camera / ESP-DL).
 * Clipped to panel bounds. ST7789 MADCTL is BGR; this packs BGR565.
 */
void board_lcd_blit_rgb888(int dst_x, int dst_y, int w, int h, const uint8_t *rgb888, int src_is_bgr);

/** Draw 1px RGB565 hollow rect (screen coords). */
void board_lcd_draw_rect(int x1, int y1, int x2, int y2, uint16_t rgb565);

/** Draw ASCII string into status bar region (y in 0..STATUS_H-1). */
void board_lcd_draw_text(int x, int y, const char *text, uint16_t fg, uint16_t bg);
/**
 * Draw ASCII string into status bar region with integer scaling.
 * scale: 1 = native 5x7, 5 = five times larger.
 */
void board_lcd_draw_text_scaled(int x, int y, const char *text, uint16_t fg, uint16_t bg, float scale);

#ifdef __cplusplus
}
#endif
