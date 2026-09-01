#pragma once

#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** CST816S INT (Prefer extract / Screen demo). */
#define BOARD_TOUCH_INT_GPIO 11
/** Datasheet TP_TRST; shared with AW9523 RSTN. Do not drive as touch-only. */
#define BOARD_TOUCH_TRST_GPIO 12

#define BOARD_TOUCH_X_MAX 239
#define BOARD_TOUCH_Y_MAX 283

typedef struct {
    int16_t x;
    int16_t y;
    bool pressed;
} board_touch_point_t;

/**
 * Add CST816S on the shared I2C bus (must call after i2c_bus_init / LCD init).
 * Leaves GPIO12 alone (AW9523 already pulsed it). INT = GPIO11 pull-up.
 */
esp_err_t board_touch_init(void);

/**
 * Poll one touch sample. Coordinates mapped to LCD space (Prefer offset_rotation=2).
 * Returns true if a contact was reported.
 */
bool board_touch_read(board_touch_point_t *out);

#ifdef __cplusplus
}
#endif
