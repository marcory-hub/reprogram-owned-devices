#ifndef I2C_BUS_H
#define I2C_BUS_H

#include "esp_err.h"
#include <stddef.h>
#include <stdint.h>

#define I2C_MASTER_FREQ_HZ 400000
#define I2C_SDA_IO 16
#define I2C_SCL_IO 15

/** AW9523 expander (backlight / board IO). */
#define I2C_ADDR_AW9523 0x5A
/** CST816S capacitive touch (Prefer Screen demo). */
#define I2C_ADDR_CST816S 0x15

esp_err_t i2c_bus_init(void);

/**
 * Register a 7-bit I2C device on the shared master bus.
 * Safe to call more than once for the same address (idempotent).
 */
esp_err_t i2c_bus_add_device(uint8_t addr);

esp_err_t i2c_bus_write(uint8_t addr, uint8_t reg, uint8_t *data, size_t len);
esp_err_t i2c_bus_read(uint8_t addr, uint8_t reg, uint8_t *data, size_t len);
esp_err_t i2c_bus_write_byte(uint8_t addr, uint8_t reg, uint8_t data);

#endif
