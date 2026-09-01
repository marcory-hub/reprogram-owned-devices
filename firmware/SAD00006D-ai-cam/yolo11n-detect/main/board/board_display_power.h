#pragma once

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t board_display_power_init(void);

bool board_display_is_on(void);

esp_err_t board_display_set_on(bool on);

/** Reset auto-off countdown (call on boot and manual ON only; not per frame). */
void board_display_kick_idle_timer(void);

/** Apply pending button/auto-off/USB edge work; call once per main-loop iteration. */
void board_display_power_poll(void);

bool board_display_usb_host_connected(void);

#ifdef __cplusplus
}
#endif
