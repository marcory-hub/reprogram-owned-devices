#include "board_display_power.h"

#include "aw9523.h"
#include "board_lcd.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

static const char *TAG = "display_pwr";

#define BUTTON_GPIO            GPIO_NUM_0
#define DEBOUNCE_US            (50 * 1000)
#define BUTTON_IGNORE_US       (500 * 1000)

static bool s_display_on = true;
static bool s_pending_toggle;
static bool s_pending_auto_off;
static bool s_usb_was_connected;
static bool s_auto_off_paused_logged;
static int64_t s_init_us;
static int64_t s_last_uart_rx_us;
static int64_t s_idle_deadline_us; /* wall-clock backup if esp_timer flag is missed */
static esp_timer_handle_t s_debounce_timer;
static esp_timer_handle_t s_auto_off_timer;

static void auto_off_timer_stop(void)
{
    if (s_auto_off_timer != NULL) {
        esp_timer_stop(s_auto_off_timer);
    }
}

static void auto_off_timer_start(void)
{
#if CONFIG_BOARD_DISPLAY_AUTO_OFF_MS > 0
    if (s_auto_off_timer == NULL || !s_display_on) {
        return;
    }
    const int64_t timeout_us = (int64_t)CONFIG_BOARD_DISPLAY_AUTO_OFF_MS * 1000LL;
    s_idle_deadline_us = esp_timer_get_time() + timeout_us;
    esp_timer_stop(s_auto_off_timer);
    esp_timer_start_once(s_auto_off_timer, (uint64_t)timeout_us);
    ESP_LOGI(TAG, "auto-off armed for %d ms", CONFIG_BOARD_DISPLAY_AUTO_OFF_MS);
#endif
}

#if CONFIG_BOARD_DISPLAY_USB_SENSE_GPIO
static bool usb_sense_gpio(void)
{
    int pin = CONFIG_BOARD_DISPLAY_USB_SENSE_GPIO_NUM;
    if (pin < 0) {
        return false;
    }
    return gpio_get_level((gpio_num_t)pin) != 0;
}
#endif

#if CONFIG_BOARD_DISPLAY_USB_SENSE_UART_HOST
static bool usb_sense_uart_host(void)
{
    size_t buffered = 0;
    if (uart_get_buffered_data_len(UART_NUM_0, &buffered) == ESP_OK && buffered > 0) {
        s_last_uart_rx_us = esp_timer_get_time();
    }
    if (s_last_uart_rx_us <= 0) {
        return false;
    }
    int64_t idle_us = (int64_t)CONFIG_BOARD_DISPLAY_USB_UART_RX_IDLE_MS * 1000LL;
    return (esp_timer_get_time() - s_last_uart_rx_us) < idle_us;
}
#endif

bool board_display_usb_host_connected(void)
{
#if !CONFIG_BOARD_DISPLAY_USB_PAUSE_AUTO_OFF
    return false;
#elif CONFIG_BOARD_DISPLAY_USB_SENSE_DISABLED
    return false;
#elif CONFIG_BOARD_DISPLAY_USB_SENSE_GPIO
    return usb_sense_gpio();
#elif CONFIG_BOARD_DISPLAY_USB_SENSE_UART_HOST
    return usb_sense_uart_host();
#else
    return false;
#endif
}

static void apply_display_off(const char *reason)
{
    if (!s_display_on) {
        return;
    }
    s_display_on = false;
    s_idle_deadline_us = 0;
    board_lcd_set_draw_enabled(false);
    esp_err_t err = lcd_backlight_ctrl(false);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "backlight off failed: %s", esp_err_to_name(err));
    }
#if CONFIG_BOARD_DISPLAY_PANEL_SLEEP
    err = board_lcd_set_panel_on(false);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "panel sleep failed: %s", esp_err_to_name(err));
    }
#endif
    auto_off_timer_stop();
    s_pending_auto_off = false;
    ESP_LOGI(TAG, "display: OFF (%s)", reason);
}

static void apply_display_on(const char *reason)
{
    if (s_display_on) {
        return;
    }
    s_display_on = true;
    esp_err_t err = ESP_OK;
#if CONFIG_BOARD_DISPLAY_PANEL_SLEEP
    err = board_lcd_set_panel_on(true);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "panel wake failed: %s", esp_err_to_name(err));
    }
#endif
    board_lcd_set_draw_enabled(true);
    err = lcd_backlight_ctrl(true);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "backlight on failed: %s", esp_err_to_name(err));
    }
    board_lcd_fill(0x0000);
    ESP_LOGI(TAG, "display: ON (%s)", reason);
    board_display_kick_idle_timer();
}

static void debounce_timer_cb(void *arg)
{
    (void)arg;
    s_pending_toggle = true;
}

static void IRAM_ATTR button_isr(void *arg)
{
    (void)arg;
    if (s_debounce_timer != NULL) {
        esp_timer_stop(s_debounce_timer);
        esp_timer_start_once(s_debounce_timer, DEBOUNCE_US);
    }
}

static void auto_off_timer_cb(void *arg)
{
    (void)arg;
    s_pending_auto_off = true;
}

static void log_aw9523_probe(void)
{
    uint8_t p0 = 0;
    uint8_t p1 = 0;
    if (aw9523_read_inputs(&p0, &p1) == ESP_OK) {
        ESP_LOGI(TAG, "AW9523 inputs probe P0=0x%02X P1=0x%02X (USB vs battery compare on device)", p0, p1);
    }
}

esp_err_t board_display_set_on(bool on)
{
    if (on) {
        apply_display_on("set");
    } else {
        apply_display_off("set");
    }
    return ESP_OK;
}

bool board_display_is_on(void)
{
    return s_display_on;
}

void board_display_kick_idle_timer(void)
{
#if CONFIG_BOARD_DISPLAY_AUTO_OFF_MS > 0
    if (!s_display_on) {
        auto_off_timer_stop();
        s_idle_deadline_us = 0;
        return;
    }
    s_auto_off_paused_logged = false;
    s_pending_auto_off = false;
    auto_off_timer_start();
#else
    s_idle_deadline_us = 0;
#endif
}

void board_display_power_poll(void)
{
    int64_t now = esp_timer_get_time();

    if (s_pending_toggle && (now - s_init_us) >= BUTTON_IGNORE_US) {
        s_pending_toggle = false;
        if (s_display_on) {
            apply_display_off("button");
        } else {
            apply_display_on("button");
        }
    }

#if CONFIG_BOARD_DISPLAY_AUTO_OFF_MS > 0
    /* Deadline backup: works even if the esp_timer callback was missed. */
    if (s_display_on && s_idle_deadline_us > 0 && now >= s_idle_deadline_us) {
        s_pending_auto_off = true;
    }
#endif

    if (s_pending_auto_off) {
        s_pending_auto_off = false;
        if (s_display_on) {
            /* Always allow auto-off while USB/powerbank is attached. */
            apply_display_off("auto");
        }
    }

    /* USB connect state is diagnostics only; it must not stop auto-off. */
    s_usb_was_connected = board_display_usb_host_connected();
    (void)s_auto_off_paused_logged;
}

esp_err_t board_display_power_init(void)
{
    s_init_us = esp_timer_get_time();
    s_display_on = true;
    s_usb_was_connected = board_display_usb_host_connected();

    log_aw9523_probe();

#if CONFIG_BOARD_DISPLAY_USB_SENSE_GPIO && CONFIG_BOARD_DISPLAY_USB_SENSE_GPIO_NUM >= 0
    gpio_config_t usb_cfg = {
        .pin_bit_mask = 1ULL << CONFIG_BOARD_DISPLAY_USB_SENSE_GPIO_NUM,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&usb_cfg));
#endif

    gpio_config_t btn_cfg = {
        .pin_bit_mask = 1ULL << BUTTON_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    ESP_ERROR_CHECK(gpio_config(&btn_cfg));

    const esp_timer_create_args_t debounce_args = {
        .callback = debounce_timer_cb,
        .name = "btn_db",
        .dispatch_method = ESP_TIMER_TASK,
    };
    ESP_ERROR_CHECK(esp_timer_create(&debounce_args, &s_debounce_timer));

    const esp_timer_create_args_t auto_off_args = {
        .callback = auto_off_timer_cb,
        .name = "auto_off",
        .dispatch_method = ESP_TIMER_TASK,
    };
    ESP_ERROR_CHECK(esp_timer_create(&auto_off_args, &s_auto_off_timer));

    esp_err_t isr_err = gpio_install_isr_service(0);
    if (isr_err != ESP_OK && isr_err != ESP_ERR_INVALID_STATE) {
        return isr_err;
    }
    ESP_ERROR_CHECK(gpio_isr_handler_add(BUTTON_GPIO, button_isr, NULL));

    board_lcd_set_draw_enabled(true);
    ESP_ERROR_CHECK(lcd_backlight_ctrl(true));

    /* Start the countdown once at boot; button ON also restarts it. */
    board_display_kick_idle_timer();

    ESP_LOGI(TAG, "GPIO0 display toggle ready (auto-off %d ms)", CONFIG_BOARD_DISPLAY_AUTO_OFF_MS);
    return ESP_OK;
}
