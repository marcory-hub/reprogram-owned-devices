#include "board_touch.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "i2c_bus.h"

static const char *TAG = "board_touch";

/* Registers from LovyanGFX Touch_CST816S (Prefer Screen demo). */
#define CST816S_REG_TOUCH 0x02
#define CST816S_REG_IRQCTL 0xFA
#define CST816S_REG_ISSPULSE 0xED
#define CST816S_CHIPID_REG 0xA7

static bool s_inited;
static bool s_chip_ready;

esp_err_t board_touch_init(void)
{
    if (s_inited) {
        return ESP_OK;
    }

    esp_err_t err = i2c_bus_add_device(I2C_ADDR_CST816S);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "add CST816S failed: 0x%x", (unsigned)err);
        return err;
    }

    gpio_config_t io = {
        .pin_bit_mask = 1ULL << BOARD_TOUCH_INT_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    err = gpio_config(&io);
    if (err != ESP_OK) {
        return err;
    }

    /* Chip may ignore I2C until first touch (low-power). Best-effort init. */
    uint8_t id[3] = {0};
    if (i2c_bus_write_byte(I2C_ADDR_CST816S, 0x00, 0x00) == ESP_OK &&
        i2c_bus_read(I2C_ADDR_CST816S, CST816S_CHIPID_REG, id, 3) == ESP_OK) {
        (void)i2c_bus_write_byte(I2C_ADDR_CST816S, CST816S_REG_IRQCTL, 0x20);
        (void)i2c_bus_write_byte(I2C_ADDR_CST816S, CST816S_REG_ISSPULSE, 20);
        s_chip_ready = true;
        ESP_LOGI(TAG, "CST816S id=%02x %02x %02x", id[0], id[1], id[2]);
    } else {
        ESP_LOGW(TAG, "CST816S not answering yet (normal until first touch)");
        s_chip_ready = false;
    }

    s_inited = true;
    return ESP_OK;
}

static bool ensure_chip(void)
{
    if (s_chip_ready) {
        return true;
    }
    uint8_t id[3] = {0};
    if (i2c_bus_write_byte(I2C_ADDR_CST816S, 0x00, 0x00) != ESP_OK) {
        return false;
    }
    if (i2c_bus_read(I2C_ADDR_CST816S, CST816S_CHIPID_REG, id, 3) != ESP_OK) {
        return false;
    }
    (void)i2c_bus_write_byte(I2C_ADDR_CST816S, CST816S_REG_IRQCTL, 0x20);
    (void)i2c_bus_write_byte(I2C_ADDR_CST816S, CST816S_REG_ISSPULSE, 20);
    s_chip_ready = true;
    ESP_LOGI(TAG, "CST816S woke id=%02x %02x %02x", id[0], id[1], id[2]);
    return true;
}

bool board_touch_read(board_touch_point_t *out)
{
    if (out == NULL || !s_inited) {
        return false;
    }
    out->pressed = false;
    out->x = 0;
    out->y = 0;

    if (!ensure_chip()) {
        return false;
    }

    uint8_t data[6] = {0};
    if (i2c_bus_read(I2C_ADDR_CST816S, CST816S_REG_TOUCH, data, sizeof(data)) != ESP_OK) {
        return false;
    }

    uint8_t points = data[0] & 0x0Fu;
    if (points == 0 || points > 1) {
        return false;
    }

    int raw_x = (int)((data[1] & 0x0F) << 8) | data[2];
    int raw_y = (int)((data[3] & 0x0F) << 8) | data[4];

    /*
     * Map CST816S raw → LCD pixels.
     * On SAD00006D with our ST7789 path: X matches, Y is inverted
     * (top "4 class" was selecting bottom "1 class").
     */
    int x = raw_x;
    int y = BOARD_TOUCH_Y_MAX - raw_y;
    if (x < 0) {
        x = 0;
    } else if (x > BOARD_TOUCH_X_MAX) {
        x = BOARD_TOUCH_X_MAX;
    }
    if (y < 0) {
        y = 0;
    } else if (y > BOARD_TOUCH_Y_MAX) {
        y = BOARD_TOUCH_Y_MAX;
    }

    out->x = (int16_t)x;
    out->y = (int16_t)y;
    out->pressed = true;
    return true;
}
