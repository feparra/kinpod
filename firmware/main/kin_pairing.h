#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define KIN_DEFAULT_RELAY_URL "https://relay.thekinpod.com"

/**
 * @brief Initializes the pairing subsystem and allocates frame buffer in PSRAM.
 */
esp_err_t kin_pairing_init(void);

/**
 * @brief Displays the 4-digit numeric keypad pairing screen.
 */
esp_err_t kin_pairing_show(void);

/**
 * @brief Hides the pairing screen and restores the active Kin avatar.
 */
esp_err_t kin_pairing_hide(void);

/**
 * @brief Returns true if the pairing screen is currently displayed.
 */
bool kin_pairing_is_active(void);

/**
 * @brief Handles touch down / tap events while the pairing screen is open.
 * @param x Horizontal coordinate (0 to 410).
 * @param y Vertical coordinate (0 to 502).
 * @return True if event was consumed.
 */
bool kin_pairing_handle_touch(uint16_t x, uint16_t y);

#ifdef __cplusplus
}
#endif
