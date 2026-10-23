#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the Human-In-The-Loop approval subsystem.
 */
esp_err_t kin_approval_init(void);

/**
 * @brief Returns true if an approval modal is currently active on screen.
 */
bool kin_approval_is_active(void);

/**
 * @brief Displays the approval modal on the AMOLED screen.
 * @param action_id Unique request ID.
 * @param agent Name of requesting agent (e.g. "openclaw", "claude").
 * @param action Name of action (e.g. "deploy_prod", "transfer_funds").
 * @param details Human readable details of what will be done.
 * @param risk Severity level ("LOW", "MEDIUM", "HIGH", "CRITICAL").
 */
esp_err_t kin_approval_show(const char *action_id, const char *agent, const char *action, const char *details, const char *risk);

/**
 * @brief Dismisses the approval modal and restores the active Kin avatar.
 */
esp_err_t kin_approval_hide(void);

/**
 * @brief Handles touch tap coordinates on the approval modal.
 */
void kin_approval_handle_touch(uint16_t x, uint16_t y);

#ifdef __cplusplus
}
#endif
