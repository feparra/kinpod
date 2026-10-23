#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Callback invoked when user taps "Configurar Wi-Fi" in Control Center.
 */
typedef void (*kin_cc_wifi_setup_cb_t)(void);

/**
 * @brief Initializes the Control Center subsystem and allocates PSRAM frame buffer.
 */
esp_err_t kin_control_center_init(kin_cc_wifi_setup_cb_t wifi_setup_cb);

/**
 * @brief Displays the Apple watchOS-style Control Center overlay on the AMOLED panel.
 */
esp_err_t kin_control_center_show(void);

/**
 * @brief Hides the Control Center and restores the active Kin character avatar.
 */
esp_err_t kin_control_center_hide(void);

/**
 * @brief Returns true if Control Center is currently displayed on screen.
 */
bool kin_control_center_is_open(void);

/**
 * @brief Handles touch tap events within the Control Center.
 * @param x Touch X coordinate (0 - 410)
 * @param y Touch Y coordinate (0 - 502)
 * @return true if touch was consumed by Control Center, false otherwise.
 */
bool kin_control_center_handle_touch(uint16_t x, uint16_t y);

#ifdef __cplusplus
}
#endif
