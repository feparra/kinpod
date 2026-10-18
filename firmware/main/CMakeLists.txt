idf_component_register(
    SRCS "main.c" "kin_i2c_mgr.c" "kin_power.c" "board_kinpod_2.06.c"
    INCLUDE_DIRS "."
    REQUIRES driver freertos nvs_flash spi_flash esp_psram esp_driver_i2c esp_lcd espressif__esp_lcd_co5300
)
