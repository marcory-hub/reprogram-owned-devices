#include "model_picker.h"

#include "board/board_display_power.h"
#include "board/board_lcd.h"
#include "board/board_touch.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "model_picker";

#define PICKER_TIMEOUT_MS 60000
#define PICKER_POLL_MS 40
#define PICKER_DEFAULT_MODEL 2 /* ESPDET_PICO_VVEL_192x256 2-class */

/* Matches VespaDetect::model_type_t */
#define MODEL_4_CLASS 0
#define MODEL_2_CLASS 2
#define MODEL_1_CLASS 3

typedef struct {
    int x;
    int y;
    int w;
    int h;
    int model;
    const char *label;
} picker_btn_t;

/* Full-width stacked rows: X-axis touch flip cannot swap 4 vs 2. */
static const picker_btn_t s_btns[] = {
    {.x = 20, .y = 48, .w = 200, .h = 56, .model = MODEL_4_CLASS, .label = "4 class"},
    {.x = 20, .y = 116, .w = 200, .h = 56, .model = MODEL_2_CLASS, .label = "2 class"},
    {.x = 20, .y = 184, .w = 200, .h = 56, .model = MODEL_1_CLASS, .label = "1 class"},
};

static void draw_button(const picker_btn_t *b, bool highlight)
{
    const uint16_t border = highlight ? 0xFFE0 : 0xFFFF; /* yellow / white */
    const uint16_t fill = highlight ? 0x2945 : 0x1082;
    board_lcd_fill_rect(b->x, b->y, b->w, b->h, fill);
    board_lcd_draw_rect(b->x, b->y, b->x + b->w - 1, b->y + b->h - 1, border);

    const float scale = 2.5f;
    const int text_w = (int)(strlen(b->label) * 6 * scale);
    const int text_h = (int)(7 * scale);
    int tx = b->x + (b->w - text_w) / 2;
    int ty = b->y + (b->h - text_h) / 2;
    if (tx < b->x + 2) {
        tx = b->x + 2;
    }
    if (ty < b->y + 2) {
        ty = b->y + 2;
    }
    board_lcd_draw_text_scaled(tx, ty, b->label, 0xFFFF, fill, scale);
}

static void draw_screen(int remain_s)
{
    board_lcd_fill(0x0000);
    board_lcd_draw_text_scaled(28, 12, "select model", 0xFFFF, 0x0000, 2.0f);

    for (size_t i = 0; i < sizeof(s_btns) / sizeof(s_btns[0]); i++) {
        draw_button(&s_btns[i], false);
    }

    char line[40];
    snprintf(line, sizeof(line), "auto 2 class %ds", remain_s);
    board_lcd_draw_text_scaled(16, 252, line, 0xC618, 0x0000, 1.5f);
}

static int hit_test(int x, int y)
{
    for (size_t i = 0; i < sizeof(s_btns) / sizeof(s_btns[0]); i++) {
        const picker_btn_t *b = &s_btns[i];
        if (x >= b->x && x < b->x + b->w && y >= b->y && y < b->y + b->h) {
            return (int)i;
        }
    }
    return -1;
}

int model_picker_run(void)
{
    ESP_LOGI(TAG, "boot picker (timeout %d ms, default model=%d)", PICKER_TIMEOUT_MS, PICKER_DEFAULT_MODEL);
    board_display_kick_idle_timer();

    const int64_t t0 = esp_timer_get_time();
    int last_remain = -1;
    int selected = -1;
    bool was_pressed = false;
    int64_t last_kick_ms = 0;

    draw_screen(PICKER_TIMEOUT_MS / 1000);

    while (true) {
        const int64_t elapsed_ms = (esp_timer_get_time() - t0) / 1000;
        if (elapsed_ms >= PICKER_TIMEOUT_MS) {
            ESP_LOGI(TAG, "timeout -> model %d (2 class)", PICKER_DEFAULT_MODEL);
            selected = PICKER_DEFAULT_MODEL;
            break;
        }

        if (elapsed_ms - last_kick_ms >= 10000) {
            board_display_kick_idle_timer();
            last_kick_ms = elapsed_ms;
        }

        int remain_s = (int)((PICKER_TIMEOUT_MS - elapsed_ms + 999) / 1000);
        if (remain_s != last_remain) {
            last_remain = remain_s;
            char line[40];
            snprintf(line, sizeof(line), "auto 2 class %ds", remain_s);
            board_lcd_fill_rect(0, 246, BOARD_LCD_H_RES, 38, 0x0000);
            board_lcd_draw_text_scaled(16, 252, line, 0xC618, 0x0000, 1.5f);
        }

        board_touch_point_t tp;
        bool pressed = board_touch_read(&tp);
        if (pressed && !was_pressed) {
            int idx = hit_test(tp.x, tp.y);
            ESP_LOGI(TAG, "touch raw (%d,%d) hit=%d", tp.x, tp.y, idx);
            if (idx >= 0) {
                draw_button(&s_btns[idx], true);
                selected = s_btns[idx].model;
                ESP_LOGI(TAG, "chose %s model=%d", s_btns[idx].label, selected);
                vTaskDelay(pdMS_TO_TICKS(250));
                break;
            }
        }
        was_pressed = pressed;

        vTaskDelay(pdMS_TO_TICKS(PICKER_POLL_MS));
    }

    board_lcd_fill(0x0000);
    const char *msg = "loading...";
    if (selected == MODEL_4_CLASS) {
        msg = "4 class";
    } else if (selected == MODEL_2_CLASS) {
        msg = "2 class";
    } else if (selected == MODEL_1_CLASS) {
        msg = "1 class";
    }
    board_lcd_draw_text_scaled(40, 120, msg, 0xFFE0, 0x0000, 3.0f);
    vTaskDelay(pdMS_TO_TICKS(500));
    return selected;
}
