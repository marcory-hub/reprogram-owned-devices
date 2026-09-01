set(src_dirs        ./ ./board ./pipeline ./ui)

set(include_dirs    ./ ./board ./pipeline ./ui)

set(requires        vespa_detect driver esp_lcd esp32-camera fatfs sdmmc esp_driver_sdspi)

idf_component_register(SRC_DIRS ${src_dirs} INCLUDE_DIRS ${include_dirs} REQUIRES ${requires} EMBED_FILES "bus.jpg")
