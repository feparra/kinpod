idf_component_register(
    SRCS "main.c" "kin_i2c_mgr.c" "kin_power.c" "board_kinpod_2.06.c" "kin_avatar.c" "kin_audio.c" "kin_wifi.c" "kin_ws.c" "kin_control_center.c" "kin_provisioning.c" "kin_pairing.c" "kin_approval.c"
    INCLUDE_DIRS "."
    REQUIRES driver freertos nvs_flash spi_flash esp_psram esp_driver_i2c esp_lcd esp_wifi esp_event esp_netif esp_http_server espressif__cjson cjson espressif__esp_websocket_client waveshare__esp32_s3_touch_amoled_2_06 waveshare__esp_lcd_sh8601 espressif__esp_lcd_touch espressif__esp_lcd_touch_ft5x06 lwip esp-tls esp_http_client spiffs
    EMBED_FILES "boot_splash.raw" "avatar_astro.raw" "avatar_crab.raw" "avatar_hermes.raw" "avatar_muse.raw" "avatar_octopus.raw" "avatar_dots.raw" "crab_idle.raw" "crab_blink.raw" "crab_listen.raw" "crab_think.raw" "crab_speak.raw"
)

