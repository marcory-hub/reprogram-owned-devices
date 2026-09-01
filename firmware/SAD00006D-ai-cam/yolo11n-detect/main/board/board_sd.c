#include "board_sd.h"

#include "driver/gpio.h"
#include "driver/spi_common.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "esp_log.h"
#include <sys/stat.h>
#include <string.h>

static const char *TAG = "board_sd";

#define PIN_NUM_MISO 6
#define PIN_NUM_MOSI 8
#define PIN_NUM_CLK  7

static sdmmc_card_t *s_card;
static bool s_mounted;

esp_err_t board_sd_init(void)
{
#if !CONFIG_BOARD_SD_SAVE_ENABLE
    return ESP_OK;
#else
    if (s_mounted) {
        return ESP_OK;
    }

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024,
    };

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI3_HOST;
    host.max_freq_khz = SDMMC_FREQ_DEFAULT;

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };

    esp_err_t ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "spi_bus_initialize failed: %s", esp_err_to_name(ret));
        return ret;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.host_id = host.slot;
#if CONFIG_BOARD_SD_CS_GPIO >= 0
    slot_config.gpio_cs = (gpio_num_t)CONFIG_BOARD_SD_CS_GPIO;
#else
    slot_config.gpio_cs = GPIO_NUM_NC;
#endif

    ret = esp_vfs_fat_sdspi_mount(BOARD_SD_MOUNT_POINT, &host, &slot_config, &mount_config, &s_card);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "mount failed: %s (detection saves disabled)", esp_err_to_name(ret));
        ESP_LOGW(TAG, "check: FAT32 card inserted, AW9523 power up before mount, pins MISO=6 CLK=7 MOSI=8");
        spi_bus_free(host.slot);
        return ret;
    }

    s_mounted = true;
    ESP_LOGI(TAG, "mounted %s", BOARD_SD_MOUNT_POINT);
    sdmmc_card_print_info(stdout, s_card);
    return ESP_OK;
#endif
}

bool board_sd_is_mounted(void)
{
#if !CONFIG_BOARD_SD_SAVE_ENABLE
    return false;
#else
    return s_mounted;
#endif
}

esp_err_t board_sd_ensure_dir(const char *name)
{
    if (!s_mounted || name == NULL || name[0] == '\0') {
        return ESP_ERR_INVALID_STATE;
    }

    char path[64];
    int n = snprintf(path, sizeof(path), "%s/%s", BOARD_SD_MOUNT_POINT, name);
    if (n <= 0 || n >= (int)sizeof(path)) {
        return ESP_ERR_INVALID_ARG;
    }

    struct stat st;
    if (stat(path, &st) == 0) {
        return S_ISDIR(st.st_mode) ? ESP_OK : ESP_FAIL;
    }

    if (mkdir(path, 0775) != 0) {
        ESP_LOGW(TAG, "mkdir %s failed", path);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "created %s", path);
    return ESP_OK;
}

int board_sd_free_pct(void)
{
#if !CONFIG_BOARD_SD_SAVE_ENABLE
    return -1;
#else
    if (!s_mounted) {
        return -1;
    }

    uint64_t total_bytes = 0;
    uint64_t free_bytes = 0;
    esp_err_t err = esp_vfs_fat_info(BOARD_SD_MOUNT_POINT, &total_bytes, &free_bytes);
    if (err != ESP_OK || total_bytes == 0) {
        return -1;
    }

    return (int)((free_bytes * 100ULL) / total_bytes);
#endif
}
