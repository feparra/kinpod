#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define KIN_DEFAULT_WIFI_SSID "YOUR_WIFI_SSID"
#define KIN_DEFAULT_WIFI_PASS "YOUR_WIFI_PASS"

/**
 * @brief Initializes Wi-Fi station mode and connects to the specified AP.
 * @param ssid Wi-Fi network SSID (NULL to use default).
 * @param pass Wi-Fi network password (NULL to use default).
 */
esp_err_t kin_wifi_init(const char *ssid, const char *pass);

/**
 * @brief Returns true if Wi-Fi is connected and an IP address has been assigned.
 */
bool kin_wifi_is_connected(void);

/**
 * @brief Gets current Wi-Fi RSSI in dBm.
 */
int8_t kin_wifi_get_rssi(void);

/**
 * @brief Gets current connected SSID string.
 */
esp_err_t kin_wifi_get_connected_ssid(char *ssid_out, size_t max_len);

#ifdef __cplusplus
}
#endif
