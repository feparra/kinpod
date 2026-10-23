#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define KIN_DEFAULT_WS_URL "ws://192.168.1.77:8000/ws/kinpod"

/**
 * @brief Initializes the WebSocket client and starts connection to KinBridge server.
 */
esp_err_t kin_ws_init(const char *ws_url);

/**
 * @brief Returns true if WebSocket connection is established with KinBridge.
 */
bool kin_ws_is_connected(void);

/**
 * @brief Sends PTT start event with active Kin avatar identifier.
 */
esp_err_t kin_ws_send_ptt_start(const char *kin_id);

/**
 * @brief Streams raw PCM audio chunk to KinBridge over binary WebSocket frame.
 */
esp_err_t kin_ws_send_audio(const void *pcm_data, size_t len);

/**
 * @brief Sends PTT stop event to signal end of user voice input.
 */
esp_err_t kin_ws_send_ptt_stop(void);

/**
 * @brief Sends Kin switch event when user swipes between avatars.
 */
esp_err_t kin_ws_send_kin_switch(const char *kin_id);

/**
 * @brief Sends telemetry packet (battery voltage, RSSI).
 */
esp_err_t kin_ws_send_telemetry(uint16_t vbat_mv, int8_t rssi);

/**
 * @brief Sends Human-In-The-Loop approval response back to KinBridge.
 */
esp_err_t kin_ws_send_approval_response(const char *action_id, bool approved);

#ifdef __cplusplus
}
#endif
