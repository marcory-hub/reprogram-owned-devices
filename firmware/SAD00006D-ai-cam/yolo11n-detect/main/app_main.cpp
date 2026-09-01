#include "board/board_camera.h"
#include "board/board_display_power.h"
#include "board/board_lcd.h"
#include "board/board_sd.h"
#include "board/board_touch.h"
#include "board/detection_save.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pipeline/detect_pipeline.h"
#include "sdkconfig.h"
#include "ui/confidence_picker.h"
#include "ui/model_picker.h"

const char *TAG = "yolo11n";

static void log_boot_diagnostics(void)
{
    esp_reset_reason_t reason = esp_reset_reason();
    ESP_LOGI(TAG, "reset reason: %d", (int)reason);
    ESP_LOGI(TAG,
             "heap internal=%u psram=%u",
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    ESP_LOGI(TAG,
             "config: score=%.2f nms=%.2f save_score=%.2f crop_pct=%d flash_16mb",
             CONFIG_PIPELINE_SCORE_THR_X100 / 100.0f,
             CONFIG_PIPELINE_NMS_THR_X100 / 100.0f,
             CONFIG_PIPELINE_SAVE_SCORE_THR_X100 / 100.0f,
             BOARD_CAM_CENTER_CROP_PCT);
    ESP_LOGI(TAG, "display auto-off=%d ms", CONFIG_BOARD_DISPLAY_AUTO_OFF_MS);
#if CONFIG_BOARD_SD_SAVE_ENABLE
    ESP_LOGI(TAG, "sd save cooldown=%d ms", CONFIG_BOARD_SD_SAVE_COOLDOWN_MS);
#endif
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "build tag conf-picker-20260825");
    ESP_LOGI(TAG, "kconfig default model=%d (picker overrides)", (int)CONFIG_DEFAULT_VESPA_DETECT_MODEL);
    log_boot_diagnostics();

    ESP_ERROR_CHECK(board_lcd_init());
    ESP_ERROR_CHECK(board_display_power_init());
    ESP_ERROR_CHECK(board_touch_init());

    const int model_choice = model_picker_run();
    ESP_LOGI(TAG, "selected model=%d", model_choice);

    const float conf_thr = confidence_picker_run();
    ESP_LOGI(TAG, "selected conf_thr=%.2f", conf_thr);

#if CONFIG_BOARD_SD_SAVE_ENABLE
    vTaskDelay(pdMS_TO_TICKS(50));
    if (board_sd_init() == ESP_OK) {
        det_save_init();
        int free_pct = board_sd_free_pct();
        if (free_pct >= 0) {
            ESP_LOGI(TAG, "sd free space: %d%%", free_pct);
        }
    }
#endif

    ESP_ERROR_CHECK(board_camera_init());
    ESP_ERROR_CHECK(detect_pipeline_init(model_choice, conf_thr));
    ESP_ERROR_CHECK(detect_pipeline_smoke());

    detect_pipeline_start();

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
