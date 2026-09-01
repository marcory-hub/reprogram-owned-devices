#pragma once

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BOARD_SD_MOUNT_POINT "/sdcard"

/** Mount SD via SPI. Non-fatal: logs warning and returns err on failure. */
esp_err_t board_sd_init(void);

bool board_sd_is_mounted(void);

/** Create a subdirectory under the mount point if missing. */
esp_err_t board_sd_ensure_dir(const char *name);

/**
 * Free space on the mounted SD card as a percentage 0-100.
 * Returns -1 if unmounted or stat failed.
 */
int board_sd_free_pct(void);

#ifdef __cplusplus
}
#endif
