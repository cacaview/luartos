# SD Card component for ESP32

set(SDCARD_SRCS
    "sdcard_driver.c"
    "sdcard_lua_bindings.c"
)

idf_component_register(
    SRCS ${SDCARD_SRCS}
    INCLUDE_DIRS "."
    REQUIRES driver esp_common fatfs spi_flash esp_driver_sdspi esp_driver_spi lua
)

# Add compiler definitions
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    SDCARD_SPI_HOST=SPI3_HOST
    SDCARD_CS_GPIO=9
    SDCARD_SCK_GPIO=11
    SDCARD_MOSI_GPIO=42
    SDCARD_MISO_GPIO=41
)
