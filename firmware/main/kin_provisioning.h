#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define KIN_PROV_SOFTAP_SSID "KinPod-Setup"
#define KIN_PROV_DEFAULT_IP  "192.168.4.1"

/**
 * @brief Starts SoftAP Wi-Fi provisioning mode and renders setup guide on AMOLED.
 */
esp_err_t kin_provisioning_start(void);

/**
 * @brief Stops provisioning mode and restores active avatar.
 */
esp_err_t kin_provisioning_stop(void);

/**
 * @brief Returns true if KinPod is currently in provisioning mode.
 */
bool kin_provisioning_is_active(void);

/**
 * @brief Handles touch tap events on the provisioning screen.
 * @param x Touch X coordinate
 * @param y Touch Y coordinate
 * @return true if touch was consumed, false otherwise.
 */
bool kin_provisioning_handle_touch(uint16_t x, uint16_t y);

/**
 * @brief Loads stored Wi-Fi and server configuration from NVS into target buffers.
 */
esp_err_t kin_provisioning_load_config(char *ssid_out, size_t ssid_len,
                                       char *pass_out, size_t pass_len,
                                       char *ws_url_out, size_t ws_url_len);

/**
 * @brief Saves Wi-Fi and server configuration to NVS.
 */
esp_err_t kin_provisioning_save_config(const char *ssid, const char *pass, const char *ws_url);

#ifdef __cplusplus
}
#endif
