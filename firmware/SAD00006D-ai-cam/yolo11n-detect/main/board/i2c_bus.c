#include "i2c_bus.h"

#include "driver/i2c_master.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "i2c_bus";
static i2c_master_bus_handle_t s_bus;
static i2c_master_dev_handle_t s_aw9523;
static i2c_master_dev_handle_t s_cst816s;
static bool s_inited;

static i2c_master_dev_handle_t *handle_slot(uint8_t addr)
{
    if (addr == I2C_ADDR_AW9523) {
        return &s_aw9523;
    }
    if (addr == I2C_ADDR_CST816S) {
        return &s_cst816s;
    }
    return NULL;
}

esp_err_t i2c_bus_init(void)
{
    if (s_inited) {
        return ESP_OK;
    }

    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_SDA_IO,
        .scl_io_num = I2C_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    esp_err_t err = i2c_new_master_bus(&bus_cfg, &s_bus);
    if (err != ESP_OK) {
        return err;
    }

    err = i2c_bus_add_device(I2C_ADDR_AW9523);
    if (err != ESP_OK) {
        return err;
    }
    s_inited = true;
    return ESP_OK;
}

esp_err_t i2c_bus_add_device(uint8_t addr)
{
    i2c_master_dev_handle_t *slot = handle_slot(addr);
    if (slot == NULL) {
        ESP_LOGE(TAG, "unsupported addr 0x%02x", addr);
        return ESP_ERR_INVALID_ARG;
    }
    if (*slot != NULL) {
        return ESP_OK;
    }
    if (s_bus == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = addr,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    return i2c_master_bus_add_device(s_bus, &dev_cfg, slot);
}

static i2c_master_dev_handle_t resolve_dev(uint8_t addr)
{
    i2c_master_dev_handle_t *slot = handle_slot(addr);
    if (slot == NULL) {
        return NULL;
    }
    return *slot;
}

esp_err_t i2c_bus_write(uint8_t addr, uint8_t reg, uint8_t *data, size_t len)
{
    i2c_master_dev_handle_t dev = resolve_dev(addr);
    if (dev == NULL || data == NULL || len == 0 || len > 8) {
        ESP_LOGE(TAG, "write bad args addr=0x%02x len=%u", addr, (unsigned)len);
        return ESP_ERR_INVALID_ARG;
    }
    uint8_t buf[9];
    buf[0] = reg;
    memcpy(buf + 1, data, len);
    return i2c_master_transmit(dev, buf, len + 1, 100);
}

esp_err_t i2c_bus_write_byte(uint8_t addr, uint8_t reg, uint8_t data)
{
    return i2c_bus_write(addr, reg, &data, 1);
}

esp_err_t i2c_bus_read(uint8_t addr, uint8_t reg, uint8_t *data, size_t len)
{
    i2c_master_dev_handle_t dev = resolve_dev(addr);
    if (dev == NULL || data == NULL || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }
    return i2c_master_transmit_receive(dev, &reg, 1, data, len, 100);
}
