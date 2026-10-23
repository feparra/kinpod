#include "kin_ws.h"
#include "kin_audio.h"
#include "kin_avatar.h"
#include "kin_approval.h"
#include "esp_websocket_client.h"
#include "cJSON.h"
#include "esp_log.h"
#include "esp_crt_bundle.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "KinWS";

static esp_websocket_client_handle_t s_client = NULL;
static bool s_is_connected = false;
static bool s_pending_ptt_stop = false;

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            ESP_LOGI(TAG, "KinPod Connected to KinBridge Server at %s [OK]", KIN_DEFAULT_WS_URL);
            s_is_connected = true;
            kin_ws_send_kin_switch(kin_avatar_get_profile(kin_avatar_get_active())->key);

            if (s_pending_ptt_stop) {
                ESP_LOGI(TAG, "Flushing pending ptt_stop to KinBridge after reconnect");
                s_pending_ptt_stop = false;
                kin_ws_send_ptt_stop();
            }
            break;
        case WEBSOCKET_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "KinPod Disconnected from KinBridge Server");
            s_is_connected = false;
            break;
        case WEBSOCKET_EVENT_DATA:
            if (data->op_code == 0x01) { // Text JSON frame
                char *json_str = (char *)malloc(data->data_len + 1);
                if (json_str) {
                    memcpy(json_str, data->data_ptr, data->data_len);
                    json_str[data->data_len] = '\0';
                    cJSON *root = cJSON_Parse(json_str);
                    if (root) {
                        cJSON *evt = cJSON_GetObjectItem(root, "event");
                        if (evt && evt->valuestring) {
                            if (strcmp(evt->valuestring, "reply") == 0) {
                                cJSON *text = cJSON_GetObjectItem(root, "text");
                                cJSON *trans = cJSON_GetObjectItem(root, "transcript");
                                ESP_LOGI(TAG, ">>> Agent [%s] Response: \"%s\" (Transcript: \"%s\") <<<",
                                         kin_avatar_get_profile(kin_avatar_get_active())->name,
                                         text ? text->valuestring : "",
                                         trans ? trans->valuestring : "");
                                kin_avatar_set_state(KIN_STATE_SPEAKING, 75);
                            } else if (strcmp(evt->valuestring, "tts_end") == 0) {
                                ESP_LOGI(TAG, "Agent finished speaking -> Restore IDLE");
                                kin_avatar_set_state(KIN_STATE_IDLE, 0);
                            } else if (strcmp(evt->valuestring, "error") == 0) {
                                cJSON *msg_item = cJSON_GetObjectItem(root, "message");
                                ESP_LOGW(TAG, "Server error event: %s", msg_item ? msg_item->valuestring : "unknown");
                                kin_avatar_set_state(KIN_STATE_IDLE, 0);
                            } else if (strcmp(evt->valuestring, "kin_switch") == 0) {
                                cJSON *target_kin = cJSON_GetObjectItem(root, "kin");
                                if (target_kin && target_kin->valuestring) {
                                    kin_avatar_set_by_name(target_kin->valuestring);
                                }
                            } else if (strcmp(evt->valuestring, "approval_request") == 0) {
                                cJSON *act_id = cJSON_GetObjectItem(root, "action_id");
                                cJSON *ag = cJSON_GetObjectItem(root, "agent");
                                cJSON *act = cJSON_GetObjectItem(root, "action");
                                cJSON *det = cJSON_GetObjectItem(root, "details");
                                cJSON *rsk = cJSON_GetObjectItem(root, "risk");

                                kin_approval_show(
                                    act_id ? act_id->valuestring : "",
                                    ag ? ag->valuestring : "Agent",
                                    act ? act->valuestring : "Action",
                                    det ? det->valuestring : "",
                                    rsk ? rsk->valuestring : "HIGH"
                                );
                            }
                        }
                        cJSON_Delete(root);
                    }
                    free(json_str);
                }
            } else if (data->op_code == 0x02) { // Binary 16kHz PCM audio stream from Agent TTS
                kin_avatar_set_state(KIN_STATE_SPEAKING, 85);
                kin_audio_play((const uint8_t *)data->data_ptr, data->data_len);
            }
            break;
        case WEBSOCKET_EVENT_ERROR:
            ESP_LOGE(TAG, "WebSocket error event: %.*s", (int)data->data_len, data->data_ptr ? (char *)data->data_ptr : "");
            break;
    }
}

esp_err_t kin_ws_init(const char *ws_url) {
    const char *target_url = ws_url ? ws_url : KIN_DEFAULT_WS_URL;
    ESP_LOGI(TAG, "Initializing WebSocket connection to: %s", target_url);

    if (s_client) {
        ESP_LOGI(TAG, "Cleaning up previous WebSocket client before reconnecting...");
        esp_websocket_client_stop(s_client);
        esp_websocket_client_destroy(s_client);
        s_client = NULL;
        s_is_connected = false;
    }

    esp_websocket_client_config_t ws_cfg = {
        .uri = target_url,
        .buffer_size = 16384,
        .reconnect_timeout_ms = 3000,
        .network_timeout_ms = 10000,
    };

    s_client = esp_websocket_client_init(&ws_cfg);
    if (!s_client) {
        ESP_LOGE(TAG, "Failed to initialize WebSocket client");
        return ESP_FAIL;
    }

    esp_websocket_register_events(s_client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)s_client);
    esp_err_t ret = esp_websocket_client_start(s_client);
    return ret;
}

bool kin_ws_is_connected(void) {
    return s_is_connected && s_client && esp_websocket_client_is_connected(s_client);
}

esp_err_t kin_ws_send_ptt_start(const char *kin_id) {
    s_pending_ptt_stop = false;
    if (!kin_ws_is_connected()) return ESP_ERR_INVALID_STATE;
    char payload[128];
    snprintf(payload, sizeof(payload), "{\"event\":\"ptt_start\",\"kin\":\"%s\"}", kin_id ? kin_id : "crab");
    int len = strlen(payload);
    int sent = esp_websocket_client_send_text(s_client, payload, len, pdMS_TO_TICKS(500));
    return sent == len ? ESP_OK : ESP_FAIL;
}

esp_err_t kin_ws_send_audio(const void *pcm_data, size_t len) {
    if (!kin_ws_is_connected() || !pcm_data || len == 0) return ESP_ERR_INVALID_STATE;
    int sent = esp_websocket_client_send_bin(s_client, (const char *)pcm_data, len, pdMS_TO_TICKS(20));
    return sent == (int)len ? ESP_OK : ESP_FAIL;
}

esp_err_t kin_ws_send_ptt_stop(void) {
    if (!kin_ws_is_connected()) {
        s_pending_ptt_stop = true;
        ESP_LOGW(TAG, "WebSocket offline when ptt_stop called; queued for reconnect");
        return ESP_OK;
    }
    const char *payload = "{\"event\":\"ptt_stop\"}";
    int len = strlen(payload);
    int sent = esp_websocket_client_send_text(s_client, payload, len, pdMS_TO_TICKS(500));
    return sent == len ? ESP_OK : ESP_FAIL;
}

esp_err_t kin_ws_send_kin_switch(const char *kin_id) {
    if (!kin_ws_is_connected()) return ESP_ERR_INVALID_STATE;
    char payload[128];
    snprintf(payload, sizeof(payload), "{\"event\":\"kin_switch\",\"kin\":\"%s\"}", kin_id ? kin_id : "crab");
    int len = strlen(payload);
    int sent = esp_websocket_client_send_text(s_client, payload, len, pdMS_TO_TICKS(500));
    return sent == len ? ESP_OK : ESP_FAIL;
}

esp_err_t kin_ws_send_telemetry(uint16_t vbat_mv, int8_t rssi) {
    if (!kin_ws_is_connected()) return ESP_ERR_INVALID_STATE;
    char payload[128];
    snprintf(payload, sizeof(payload), "{\"event\":\"telemetry\",\"battery_mv\":%u,\"rssi\":%d}", vbat_mv, rssi);
    int len = strlen(payload);
    int sent = esp_websocket_client_send_text(s_client, payload, len, pdMS_TO_TICKS(500));
    return sent == len ? ESP_OK : ESP_FAIL;
}

esp_err_t kin_ws_send_approval_response(const char *action_id, bool approved) {
    if (!kin_ws_is_connected() || !action_id) return ESP_ERR_INVALID_STATE;
    char payload[256];
    snprintf(payload, sizeof(payload),
             "{\"event\":\"approval_response\",\"action_id\":\"%s\",\"approved\":%s,\"actor\":\"kinpod_touch\"}",
             action_id, approved ? "true" : "false");
    int len = strlen(payload);
    int sent = esp_websocket_client_send_text(s_client, payload, len, pdMS_TO_TICKS(500));
    return sent == len ? ESP_OK : ESP_FAIL;
}
