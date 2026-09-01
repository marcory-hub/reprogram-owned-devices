#ifndef AW9523_H
#define AW9523_H

#include "esp_err.h"
#include <stdbool.h>

#include "i2c_bus.h"

#define AW9523_ADDR         I2C_ADDR_AW9523
#define AW9523_RSTN_PIN     12      /* Shared net with datasheet TP_TRST; AW9523 owns reset pulse. */

// 寄存器地址定义
#define REG_ID              0x00
#define REG_INPUT_PORT0     0x00
#define REG_INPUT_PORT1     0x01
#define REG_OUT_PORT0       0x02
#define REG_OUT_PORT1       0x03
#define REG_CONFIG_PORT1    0x13
#define REG_CONFIG_PORT0    0x12
#define All_Config_addr     0x11

/**
 * @brief 初始化 AW9523 芯片状态
 */
esp_err_t aw9523_init(void);

/**
 * @brief 设置 P1_1 引脚的 LED 状态
 * @param state true 为亮 (255亮度), false 为灭 (0亮度)
 */
esp_err_t aw9523_set_p1_gpio(bool state);

/**
 * @brief 控制 LCD 背光的函数
 * @param level true 打开背光, false 关闭背光
 */
esp_err_t lcd_backlight_ctrl(bool level);

/** Read AW9523 input port bytes (P0, P1). For USB/charge probe logging. */
esp_err_t aw9523_read_inputs(uint8_t *port0, uint8_t *port1);

#endif

