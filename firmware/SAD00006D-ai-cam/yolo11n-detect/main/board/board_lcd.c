#include "board_lcd.h"

#include "aw9523.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_bus.h"
#include "freertos/semphr.h"
#include <string.h>
#include <math.h>

static const char *TAG = "board_lcd";

#define LCD_HOST SPI2_HOST
#define PIN_NUM_MOSI 13
#define PIN_NUM_CLK 14
#define PIN_NUM_CS 10
#define PIN_NUM_DC 9
#define PIN_NUM_RST -1

static esp_lcd_panel_handle_t s_panel;
static SemaphoreHandle_t s_color_done;
static bool s_draw_enabled = true;
static bool s_panel_on = true;

#define LCD_STRIP_H_FILL 20
#define LCD_STRIP_H_BLIT 8

/** Pre-allocated DMA draw buffers (owned by output task). */
static uint16_t *s_dma_line;
static uint16_t *s_dma_fill_strip;
static uint16_t *s_dma_blit_strip;
static uint16_t *s_dma_vline;

static bool lcd_on_color_done(esp_lcd_panel_io_handle_t io,
                              esp_lcd_panel_io_event_data_t *edata,
                              void *user_ctx)
{
    (void)io;
    (void)edata;
    (void)user_ctx;
    BaseType_t hp = pdFALSE;
    xSemaphoreGiveFromISR(s_color_done, &hp);
    return hp == pdTRUE;
}

/** Color DMA is async. Wait before reusing or freeing the pixel buffer. */
static void lcd_bitmap(int x1, int y1, int x2, int y2, const void *color)
{
    if (!s_panel || !color || x2 <= x1 || y2 <= y1) {
        return;
    }
    esp_err_t err = esp_lcd_panel_draw_bitmap(s_panel, x1, y1, x2, y2, color);
    if (err != ESP_OK) {
        return;
    }
    xSemaphoreTake(s_color_done, portMAX_DELAY);
}

static esp_err_t board_lcd_alloc_dma_buffers(void)
{
    s_dma_line = heap_caps_malloc(BOARD_LCD_H_RES * sizeof(uint16_t), MALLOC_CAP_DMA);
    s_dma_fill_strip = heap_caps_malloc(BOARD_LCD_H_RES * LCD_STRIP_H_FILL * sizeof(uint16_t), MALLOC_CAP_DMA);
    s_dma_blit_strip = heap_caps_malloc(BOARD_LCD_H_RES * LCD_STRIP_H_BLIT * sizeof(uint16_t), MALLOC_CAP_DMA);
    s_dma_vline = heap_caps_malloc(BOARD_LCD_V_RES * sizeof(uint16_t), MALLOC_CAP_DMA);
    if (!s_dma_line || !s_dma_fill_strip || !s_dma_blit_strip || !s_dma_vline) {
        ESP_LOGE(TAG, "DMA buffer alloc failed");
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

/* 5x7 glyphs for needed ASCII; bit0 = top row left. */
static const uint8_t FONT5X7[][5] = {
    [' '] = {0x00, 0x00, 0x00, 0x00, 0x00},
    ['.'] = {0x00, 0x00, 0x00, 0x00, 0x40},
    ['0'] = {0x3E, 0x51, 0x49, 0x45, 0x3E},
    ['1'] = {0x00, 0x42, 0x7F, 0x40, 0x00},
    ['2'] = {0x42, 0x61, 0x51, 0x49, 0x46},
    ['3'] = {0x21, 0x41, 0x45, 0x4B, 0x31},
    ['4'] = {0x18, 0x14, 0x12, 0x7F, 0x10},
    ['5'] = {0x27, 0x45, 0x45, 0x45, 0x39},
    ['6'] = {0x3C, 0x4A, 0x49, 0x49, 0x30},
    ['7'] = {0x01, 0x71, 0x09, 0x05, 0x03},
    ['8'] = {0x36, 0x49, 0x49, 0x49, 0x36},
    ['9'] = {0x06, 0x49, 0x49, 0x29, 0x1E},
    ['a'] = {0x20, 0x54, 0x54, 0x54, 0x78},
    ['c'] = {0x38, 0x44, 0x44, 0x44, 0x20},
    ['d'] = {0x7F, 0x20, 0x40, 0x40, 0x38},
    ['e'] = {0x38, 0x54, 0x54, 0x54, 0x18},
    ['f'] = {0x00, 0x08, 0x7E, 0x09, 0x02},
    ['g'] = {0x08, 0x54, 0x54, 0x54, 0x3C},
    ['i'] = {0x00, 0x41, 0x7F, 0x40, 0x00},
    ['l'] = {0x00, 0x41, 0x7F, 0x40, 0x00},
    ['m'] = {0x7C, 0x04, 0x18, 0x04, 0x78},
    ['n'] = {0x7C, 0x08, 0x04, 0x04, 0x78},
    ['o'] = {0x38, 0x44, 0x44, 0x44, 0x38},
    ['p'] = {0x7C, 0x14, 0x14, 0x14, 0x08},
    ['r'] = {0x7C, 0x08, 0x04, 0x04, 0x08},
    ['s'] = {0x48, 0x54, 0x54, 0x54, 0x24},
    ['t'] = {0x04, 0x3F, 0x44, 0x40, 0x20},
    ['u'] = {0x3C, 0x40, 0x40, 0x20, 0x7C},
    ['v'] = {0x3C, 0x40, 0x40, 0x40, 0x3C},
};

static inline uint16_t rgb888_to_rgb565_swapped(uint8_t r, uint8_t g, uint8_t b)
{
    uint16_t c = (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
    return (uint16_t)((c >> 8) | (c << 8));
}

static void lcd_st7789_custom_init(esp_lcd_panel_io_handle_t io)
{
    esp_lcd_panel_io_tx_param(io, 0x01, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(120));
    esp_lcd_panel_io_tx_param(io, 0x11, NULL, 0);
    vTaskDelay(pdMS_TO_TICKS(120));

    uint8_t data = 0x55;
    esp_lcd_panel_io_tx_param(io, 0x3A, &data, 1);
    data = 0x08;
    esp_lcd_panel_io_tx_param(io, 0x36, &data, 1);

    uint8_t proch[5] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
    esp_lcd_panel_io_tx_param(io, 0xB2, proch, 5);
    data = 0x05;
    esp_lcd_panel_io_tx_param(io, 0xB7, &data, 1);
    data = 0x21;
    esp_lcd_panel_io_tx_param(io, 0xBB, &data, 1);
    data = 0x2C;
    esp_lcd_panel_io_tx_param(io, 0xC0, &data, 1);
    data = 0x01;
    esp_lcd_panel_io_tx_param(io, 0xC2, &data, 1);
    data = 0x15;
    esp_lcd_panel_io_tx_param(io, 0xC3, &data, 1);
    data = 0x0F;
    esp_lcd_panel_io_tx_param(io, 0xC6, &data, 1);

    uint8_t d0[] = {0xA4, 0xA1};
    esp_lcd_panel_io_tx_param(io, 0xD0, d0, 2);
    data = 0xA1;
    esp_lcd_panel_io_tx_param(io, 0xD6, &data, 1);

    uint8_t gamma_pos[14] = {0xF0, 0x05, 0x0E, 0x08, 0x0A, 0x17, 0x39, 0x54, 0x4E, 0x37, 0x12, 0x12, 0x21, 0x37};
    esp_lcd_panel_io_tx_param(io, 0xE0, gamma_pos, 14);
    uint8_t gamma_neg[14] = {0xF0, 0x10, 0x14, 0x0D, 0x0B, 0x05, 0x39, 0x44, 0x4D, 0x38, 0x14, 0x14, 0x2E, 0x35};
    esp_lcd_panel_io_tx_param(io, 0xE1, gamma_neg, 14);
    uint8_t e4[] = {0x23, 0x00, 0x00};
    esp_lcd_panel_io_tx_param(io, 0xE4, e4, 3);

    esp_lcd_panel_io_tx_param(io, 0x21, NULL, 0);
    esp_lcd_panel_io_tx_param(io, 0x29, NULL, 0);
    esp_lcd_panel_io_tx_param(io, 0x2C, NULL, 0);
}

esp_err_t board_lcd_init(void)
{
    s_color_done = xSemaphoreCreateBinary();
    if (s_color_done == NULL) {
        return ESP_ERR_NO_MEM;
    }

    ESP_ERROR_CHECK(i2c_bus_init());
    ESP_ERROR_CHECK(aw9523_init());

    spi_bus_config_t bus_config = {
        .sclk_io_num = PIN_NUM_CLK,
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = -1,
        .max_transfer_sz = BOARD_LCD_H_RES * 40 * 2,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &bus_config, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_handle_t io = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_NUM_DC,
        .cs_gpio_num = PIN_NUM_CS,
        .pclk_hz = 40 * 1000 * 1000,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = lcd_on_color_done,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io));

    lcd_st7789_custom_init(io);

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io, &panel_config, &s_panel));
    esp_lcd_panel_set_gap(s_panel, 0, 0);

    ESP_ERROR_CHECK(board_lcd_alloc_dma_buffers());

    board_lcd_fill(0x0000);
    ESP_LOGI(TAG, "ST7789 ready %dx%d", BOARD_LCD_H_RES, BOARD_LCD_V_RES);
    return ESP_OK;
}

void board_lcd_set_draw_enabled(bool enabled)
{
    s_draw_enabled = enabled;
}

bool board_lcd_is_draw_enabled(void)
{
    return s_draw_enabled;
}

esp_err_t board_lcd_set_panel_on(bool on)
{
    if (!s_panel) {
        return ESP_ERR_INVALID_STATE;
    }
    if (on == s_panel_on) {
        return ESP_OK;
    }
    esp_err_t err = esp_lcd_panel_disp_on_off(s_panel, on);
    if (err != ESP_OK) {
        return err;
    }
    if (on) {
        vTaskDelay(pdMS_TO_TICKS(120));
    }
    s_panel_on = on;
    return ESP_OK;
}


void board_lcd_fill_rect(int x, int y, int w, int h, uint16_t rgb565)
{
    if (!s_draw_enabled || !s_panel_on || !s_panel || w <= 0 || h <= 0) {
        return;
    }
    if (x < 0) {
        w += x;
        x = 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }
    if (x + w > BOARD_LCD_H_RES) {
        w = BOARD_LCD_H_RES - x;
    }
    if (y + h > BOARD_LCD_V_RES) {
        h = BOARD_LCD_V_RES - y;
    }
    if (w <= 0 || h <= 0) {
        return;
    }
    uint16_t swapped = (uint16_t)((rgb565 >> 8) | (rgb565 << 8));
    if (!s_dma_line) {
        return;
    }
    for (int i = 0; i < w; i++) {
        s_dma_line[i] = swapped;
    }
    for (int row = 0; row < h; row++) {
        lcd_bitmap(x, y + row, x + w, y + row + 1, s_dma_line);
    }
}

void board_lcd_fill(uint16_t rgb565)
{
    if (!s_draw_enabled || !s_panel_on || !s_panel) {
        return;
    }
    const int strip_h = LCD_STRIP_H_FILL;
    if (!s_dma_fill_strip) {
        return;
    }
    uint16_t swapped = (uint16_t)((rgb565 >> 8) | (rgb565 << 8));
    for (int i = 0; i < BOARD_LCD_H_RES * strip_h; i++) {
        s_dma_fill_strip[i] = swapped;
    }
    for (int y = 0; y < BOARD_LCD_V_RES; y += strip_h) {
        int h = strip_h;
        if (y + h > BOARD_LCD_V_RES) {
            h = BOARD_LCD_V_RES - y;
        }
        lcd_bitmap(0, y, BOARD_LCD_H_RES, y + h, s_dma_fill_strip);
    }
}

void board_lcd_blit_rgb888(int dst_x, int dst_y, int w, int h, const uint8_t *rgb888, int src_is_bgr)
{
    if (!s_draw_enabled || !s_panel_on || !s_panel || !rgb888 || w <= 0 || h <= 0) {
        return;
    }
    if (!s_dma_blit_strip) {
        return;
    }

    /* Clip to panel; support infer frames wider/taller than the LCD (e.g. 256x192). */
    int src_x0 = 0;
    int src_y0 = 0;
    int draw_w = w;
    int draw_h = h;
    int out_x = dst_x;
    int out_y = dst_y;
    if (out_x < 0) {
        src_x0 = -out_x;
        draw_w += out_x;
        out_x = 0;
    }
    if (out_y < 0) {
        src_y0 = -out_y;
        draw_h += out_y;
        out_y = 0;
    }
    if (out_x + draw_w > BOARD_LCD_H_RES) {
        draw_w = BOARD_LCD_H_RES - out_x;
    }
    if (out_y + draw_h > BOARD_LCD_V_RES) {
        draw_h = BOARD_LCD_V_RES - out_y;
    }
    if (draw_w <= 0 || draw_h <= 0) {
        return;
    }

    const int strip_h = LCD_STRIP_H_BLIT;
    for (int row = 0; row < draw_h; row += strip_h) {
        int rows = strip_h;
        if (row + rows > draw_h) {
            rows = draw_h - row;
        }
        for (int yy = 0; yy < rows; yy++) {
            const uint8_t *src = rgb888 + ((src_y0 + row + yy) * w + src_x0) * 3;
            uint16_t *dst = s_dma_blit_strip + yy * draw_w;
            for (int x = 0; x < draw_w; x++) {
                /* MADCTL BGR: pack B into RGB565 red bits. RGB888 needs R/B swap; BGR888 does not. */
                if (src_is_bgr) {
                    dst[x] = rgb888_to_rgb565_swapped(src[0], src[1], src[2]);
                } else {
                    dst[x] = rgb888_to_rgb565_swapped(src[2], src[1], src[0]);
                }
                src += 3;
            }
        }
        lcd_bitmap(out_x, out_y + row, out_x + draw_w, out_y + row + rows, s_dma_blit_strip);
    }
}

static void draw_hline(int x1, int x2, int y, uint16_t swapped)
{
    if (y < 0 || y >= BOARD_LCD_V_RES) {
        return;
    }
    if (x1 > x2) {
        int t = x1;
        x1 = x2;
        x2 = t;
    }
    if (x1 < 0) {
        x1 = 0;
    }
    if (x2 >= BOARD_LCD_H_RES) {
        x2 = BOARD_LCD_H_RES - 1;
    }
    if (x1 > x2) {
        return;
    }
    int w = x2 - x1 + 1;
    if (!s_dma_line || w > BOARD_LCD_H_RES) {
        return;
    }
    for (int i = 0; i < w; i++) {
        s_dma_line[i] = swapped;
    }
    lcd_bitmap(x1, y, x2 + 1, y + 1, s_dma_line);
}

static void draw_vline(int x, int y1, int y2, uint16_t swapped)
{
    if (x < 0 || x >= BOARD_LCD_H_RES) {
        return;
    }
    if (y1 > y2) {
        int t = y1;
        y1 = y2;
        y2 = t;
    }
    if (y1 < 0) {
        y1 = 0;
    }
    if (y2 >= BOARD_LCD_V_RES) {
        y2 = BOARD_LCD_V_RES - 1;
    }
    if (y1 > y2) {
        return;
    }
    int h = y2 - y1 + 1;
    if (!s_dma_vline || h > BOARD_LCD_V_RES) {
        return;
    }
    for (int i = 0; i < h; i++) {
        s_dma_vline[i] = swapped;
    }
    lcd_bitmap(x, y1, x + 1, y2 + 1, s_dma_vline);
}

void board_lcd_draw_rect(int x1, int y1, int x2, int y2, uint16_t rgb565)
{
    if (!s_draw_enabled || !s_panel_on || !s_panel) {
        return;
    }
    uint16_t swapped = (uint16_t)((rgb565 >> 8) | (rgb565 << 8));
    draw_hline(x1, x2, y1, swapped);
    draw_hline(x1, x2, y2, swapped);
    draw_vline(x1, y1, y2, swapped);
    draw_vline(x2, y1, y2, swapped);
}

void board_lcd_draw_text(int x, int y, const char *text, uint16_t fg, uint16_t bg)
{
    if (!text || !s_draw_enabled || !s_panel_on || !s_panel) {
        return;
    }
    uint16_t fg_s = (uint16_t)((fg >> 8) | (fg << 8));
    uint16_t bg_s = (uint16_t)((bg >> 8) | (bg << 8));
    const int gw = 6; /* 5 + 1 gap */
    const int gh = 8;
    uint16_t glyph[5 * 7];
    for (const char *p = text; *p; p++) {
        unsigned char ch = (unsigned char)*p;
        const uint8_t *cols = FONT5X7[ch];
        /* fallback blank if unset (all zero and not space) */
        for (int col = 0; col < 5; col++) {
            uint8_t bits = cols[col];
            for (int row = 0; row < 7; row++) {
                glyph[row * 5 + col] = (bits & (1 << row)) ? fg_s : bg_s;
            }
        }
        if (x + 5 > BOARD_LCD_H_RES || y + 7 > BOARD_LCD_V_RES) {
            break;
        }
        lcd_bitmap(x, y, x + 5, y + 7, glyph);
        x += gw;
        (void)gh;
        (void)bg_s;
    }
}

void board_lcd_draw_text_scaled(int x, int y, const char *text, uint16_t fg, uint16_t bg, float scale)
{
    if (!text || !s_draw_enabled || !s_panel_on || !s_panel || scale <= 0.0f) {
        return;
    }
    /* Character native size: 5x7, gap 1 pixel -> advance 6 */
    const int char_w = 5;
    const int char_h = 7;
    const int advance = 6;
    size_t len = strlen(text);
    if (len == 0) {
        return;
    }
    /* Compute total pixel extents using floor/ceil to handle fractional scale. */
    int total_w = (int)ceilf((float)len * (float)advance * scale);
    int total_h = (int)ceilf((float)char_h * scale);
    (void)total_w;
    (void)total_h;

    for (size_t i = 0; i < len; i++) {
        unsigned char ch = (unsigned char)text[i];
        const uint8_t *cols = FONT5X7[ch];
        for (int col = 0; col < char_w; col++) {
            uint8_t bits = cols[col];
            for (int row = 0; row < char_h; row++) {
                if (!(bits & (1 << row))) {
                    continue;
                }
                /* Map source pixel column/row to scaled destination span [start,end). */
                float src_x = (float)(i * advance + col) * scale;
                float src_x_next = (float)(i * advance + col + 1) * scale;
                float src_y = (float)row * scale;
                float src_y_next = (float)(row + 1) * scale;
                int dst_x0 = (int)floorf(src_x) + x;
                int dst_x1 = (int)floorf(src_x_next) + x;
                int dst_y0 = (int)floorf(src_y) + y;
                int dst_y1 = (int)floorf(src_y_next) + y;
                int w = dst_x1 - dst_x0;
                int h = dst_y1 - dst_y0;
                if (w <= 0) {
                    w = 1;
                }
                if (h <= 0) {
                    h = 1;
                }
                board_lcd_fill_rect(dst_x0, dst_y0, w, h, fg);
            }
        }
    }
}
