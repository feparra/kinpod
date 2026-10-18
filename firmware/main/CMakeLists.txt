idf_component_register(
    SRCS "main.c" "kin_i2c_mgr.c" "kin_power.c" "board_kinpod_2.06.c"
    INCLUDE_DIRS "."
    REQUIRES driver freertos nvs_flash spi_flash esp_psram esp_driver_i2c esp_lcd waveshare__esp32_s3_touch_amoled_2_06 waveshare__esp_lcd_sh8601
    EMBED_FILES "boot_splash.raw"
)
