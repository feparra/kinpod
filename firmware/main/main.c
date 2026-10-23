#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_psram.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "kin_i2c_mgr.h"
#include "kin_power.h"
#include "board_kinpod_2.06.h"
#include "kin_avatar.h"
#include "kin_audio.h"
#include "kin_wifi.h"
#include "kin_ws.h"
#include "kin_control_center.h"
#include "kin_provisioning.h"
#include "kin_pairing.h"
#include "kin_approval.h"

static const char *TAG = "KinPod-Main";

static void on_wifi_setup_requested(void) {
    ESP_LOGI(TAG, "Wi-Fi setup requested -> Launching Provisioning SoftAP");
    kin_control_center_hide();
    kin_provisioning_start();
}

static void ptt_audio_stream_cb(const int16_t *pcm_data, size_t samples_count, bool is_final) {
    if (is_final) {
        kin_ws_send_ptt_stop();
    } else if (pcm_data && samples_count > 0) {
        kin_ws_send_audio(pcm_data, samples_count * sizeof(int16_t));
    }
}

static void network_manager_task(void *pvParameters) {
    ESP_LOGI(TAG, "Network Manager Task started on Core 0.");

    // Load credentials from NVS if previously configured via SoftAP
    char ssid[64] = {0};
    char pass[64] = {0};
    char ws_url[256] = {0};
    kin_provisioning_load_config(ssid, sizeof(ssid), pass, sizeof(pass), ws_url, sizeof(ws_url));

    const char *target_ssid = (strlen(ssid) > 0) ? ssid : NULL;
    const char *target_pass = (strlen(pass) > 0) ? pass : NULL;
    const char *target_ws   = (strlen(ws_url) > 0) ? ws_url : NULL;

    // 1. Initialize Wi-Fi in Station mode
    esp_err_t ret = kin_wifi_init(target_ssid, target_pass);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Wi-Fi init failed: %s", esp_err_to_name(ret));
        vTaskDelete(NULL);
        return;
    }

    // 2. Wait until connected to AP and IP acquired
    int retries = 0;
    while (!kin_wifi_is_connected()) {
        vTaskDelay(pdMS_TO_TICKS(500));
        retries++;
        if (retries % 10 == 0) {
            ESP_LOGI(TAG, "Connecting to Wi-Fi SSID '%s'...", target_ssid ? target_ssid : KIN_DEFAULT_WIFI_SSID);
        }
    }

    ESP_LOGI(TAG, "Wi-Fi connected! Starting WebSocket connection to KinBridge (%s)...", target_ws ? target_ws : KIN_DEFAULT_WS_URL);

    // 3. Initialize WebSocket client to KinBridge
    kin_ws_init(target_ws);

    // 4. Periodic telemetry loop (every 10s)
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        if (kin_ws_is_connected()) {
            kin_power_telemetry_t telem = {0};
            uint16_t vbat = 3950;
            if (kin_power_get_telemetry(&telem) == ESP_OK) {
                vbat = telem.vbat_mv;
            }
            int8_t rssi = kin_wifi_get_rssi();
            kin_ws_send_telemetry(vbat, rssi);
        }
    }
}

static void power_monitor_task(void *pvParameters) {
    kin_power_telemetry_t telem = {0};
    while (1) {
        if (kin_power_get_telemetry(&telem) == ESP_OK) {
            ESP_LOGI(TAG, "Telemetry: VBat=%u mV | SOC=%u%% | Charging=%s | Active Kin: %s | Wi-Fi: %s (%d dBm)",
                     telem.vbat_mv, telem.battery_pct, telem.is_charging ? "YES" : "NO",
                     kin_avatar_get_profile(kin_avatar_get_active())->name,
                     kin_wifi_is_connected() ? "CONNECTED" : "OFFLINE",
                     kin_wifi_get_rssi());
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

static void touch_gesture_task(void *pvParameters) {
    uint16_t x = 0, y = 0;
    bool was_touched = false;
    uint16_t start_x = 0, start_y = 0;
    uint16_t last_x = 0, last_y = 0;
    TickType_t start_time = 0;

    ESP_LOGI(TAG, "Touch Gesture Task started: Swipe LEFT/RIGHT for Kin, DOWN for Control Center, UP to close.");

    while (1) {
        bool is_touched = kin_touch_get_point(&x, &y);

        if (is_touched) {
            if (!was_touched) {
                // Touch down
                was_touched = true;
                start_x = x;
                start_y = y;
                last_x = x;
                last_y = y;
                start_time = xTaskGetTickCount();

                if (kin_approval_is_active()) {
                    ESP_LOGI(TAG, "Approval Modal Touch-Down detected at (%u, %u)", x, y);
                    kin_approval_handle_touch(x, y);
                }
            } else {
                // Dragging
                last_x = x;
                last_y = y;
            }
        } else {
            if (was_touched) {
                was_touched = false;
                int16_t dx = (int16_t)last_x - (int16_t)start_x;
                int16_t dy = (int16_t)last_y - (int16_t)start_y;
                TickType_t duration = xTaskGetTickCount() - start_time;

                ESP_LOGI(TAG, "Touch Event: start=(%u,%u) end=(%u,%u) dx=%d dy=%d dur=%lu ms",
                         start_x, start_y, last_x, last_y, dx, dy, (uint32_t)pdTICKS_TO_MS(duration));

                if (kin_approval_is_active()) {
                    // Human-In-The-Loop Approval Modal: fallback on release if not already handled
                    kin_approval_handle_touch(last_x, last_y);
                } else if (kin_provisioning_is_active()) {
                    // Provisioning setup mode is active on screen
                    if (dy < -45 && abs(dy) > (abs(dx) * 4 / 3)) {
                        ESP_LOGI(TAG, "^^^ SWIPE UP DETECTED -> CANCEL PROVISIONING ^^^");
                        kin_audio_haptic_buzz(20);
                        kin_provisioning_stop();
                    } else if (abs(dx) < 30 && abs(dy) < 30 && duration < pdMS_TO_TICKS(400)) {
                        kin_provisioning_handle_touch(last_x, last_y);
                    }
                } else if (kin_pairing_is_active()) {
                    // 4-Digit Pairing Keypad is active on screen
                    if (dy < -45 && abs(dy) > (abs(dx) * 4 / 3)) {
                        ESP_LOGI(TAG, "^^^ SWIPE UP DETECTED -> CANCEL PAIRING ^^^");
                        kin_audio_haptic_buzz(20);
                        kin_pairing_hide();
                    } else if (abs(dx) < 30 && abs(dy) < 30 && duration < pdMS_TO_TICKS(400)) {
                        kin_pairing_handle_touch(last_x, last_y);
                    }
                } else if (kin_control_center_is_open()) {
                    // Control Center is open: check for swipe UP to close or tap
                    if (dy < -45 && abs(dy) > (abs(dx) * 4 / 3)) {
                        ESP_LOGI(TAG, "^^^ SWIPE UP DETECTED -> CLOSE CONTROL CENTER ^^^");
                        kin_audio_haptic_buzz(20);
                        kin_control_center_hide();
                    } else if (abs(dx) < 30 && abs(dy) < 30 && duration < pdMS_TO_TICKS(400)) {
                        kin_control_center_handle_touch(last_x, last_y);
                    }
                } else {
                    // Normal Avatar mode: swipe DOWN opens Control Center, LEFT/RIGHT switches Kin
                    if (dy > 45 && dy > (abs(dx) * 4 / 3)) {
                        ESP_LOGI(TAG, "vvv SWIPE DOWN DETECTED -> OPEN CONTROL CENTER vvv");
                        kin_audio_haptic_buzz(30);
                        kin_control_center_show();
                    } else if (abs(dx) > 45 && abs(dx) > (abs(dy) * 4 / 3)) {
                        if (dx < 0) {
                            ESP_LOGI(TAG, ">>> SWIPE LEFT DETECTED -> NEXT KIN >>>");
                            kin_avatar_next();
                        } else {
                            ESP_LOGI(TAG, "<<< SWIPE RIGHT DETECTED -> PREV KIN <<<");
                            kin_avatar_prev();
                        }
                        // Notify KinBridge of newly selected Kin
                        kin_ws_send_kin_switch(kin_avatar_get_profile(kin_avatar_get_active())->key);
                    } else if (abs(dx) < 30 && abs(dy) < 30 && duration < pdMS_TO_TICKS(350)) {
                        ESP_LOGI(TAG, "TAP DETECTED at (%u, %u) on [%s]",
                                 last_x, last_y, kin_avatar_get_profile(kin_avatar_get_active())->name);
                    }
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(25)); // 40 Hz polling rate
    }
}

static void serial_cmd_task(void *pvParameters) {
    char line[512];
    ESP_LOGI(TAG, "Serial Command Channel active (Commands: SET_WS <url>, SET_WIFI <ssid> <pass>, GET_CONFIG)");

    while (1) {
        if (fgets(line, sizeof(line), stdin) != NULL) {
            size_t len = strlen(line);
            while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == '\n' || line[len - 1] == ' ')) {
                line[--len] = '\0';
            }
            if (len == 0) continue;

            if (strncmp(line, "SET_WS ", 7) == 0) {
                const char *url = line + 7;
                while (*url == ' ') url++;
                ESP_LOGI(TAG, "Agent Serial Command SET_WS: '%s'", url);
                kin_provisioning_save_config(NULL, NULL, url);
                printf("\r\n[KINPOD_OK] WS_URL_SAVED: %s\r\n", url);
                fflush(stdout);
                kin_ws_init(url);
            } else if (strncmp(line, "SET_WIFI ", 9) == 0) {
                char ssid[64] = {0};
                char pass[64] = {0};
                if (sscanf(line + 9, "%63s %63s", ssid, pass) >= 1) {
                    ESP_LOGI(TAG, "Agent Serial Command SET_WIFI: '%s'", ssid);
                    kin_provisioning_save_config(ssid, pass, NULL);
                    printf("\r\n[KINPOD_OK] WIFI_SAVED: %s\r\n", ssid);
                    fflush(stdout);
                }
            } else if (strcmp(line, "GET_CONFIG") == 0) {
                char s[64] = {0}, p[64] = {0}, w[256] = {0};
                kin_provisioning_load_config(s, sizeof(s), p, sizeof(p), w, sizeof(w));
                printf("\r\n[KINPOD_CONFIG] ssid='%s' ws='%s'\r\n", s, w);
                fflush(stdout);
            } else if (strcmp(line, "PAIR") == 0) {
                ESP_LOGI(TAG, "Agent Serial Command: Show 4-Digit Pairing Keypad");
                kin_pairing_show();
                printf("\r\n[KINPOD_OK] PAIRING_KEYPAD_ACTIVE\r\n");
                fflush(stdout);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "=================================================");
    ESP_LOGI(TAG, "  KinPod (KinOS) v6.0 Firmware — Phase 5 Voice   ");
    ESP_LOGI(TAG, "  Hardware: Waveshare ESP32-S3 Touch AMOLED 2.06 ");
    ESP_LOGI(TAG, "=================================================");

    // 1. Initialize NVS (Non-Volatile Storage)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Chip & Memory Audit
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    uint32_t flash_size = 0;
    esp_flash_get_size(NULL, &flash_size);
    size_t psram_size = esp_psram_get_size();

    ESP_LOGI(TAG, "ESP32-S3 Cores: %d | Revision: %d", chip_info.cores, chip_info.revision);
    ESP_LOGI(TAG, "Flash Memory Size: %lu MB (Quad SPI)", flash_size / (1024 * 1024));
    ESP_LOGI(TAG, "Octal PSRAM Size:  %zu MB (High-Speed DMA)", psram_size / (1024 * 1024));

    // 3. Initialize Shared I2C0 Bus and Power Subsystem (PMU AXP2101)
    ESP_LOGI(TAG, "Initializing PMU AXP2101 & ALDO2 (3.3V Power Rail for AMOLED)...");
    ret = kin_power_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "CRITICAL: Power subsystem failed to initialize: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "PMU AXP2101 OK: AMOLED Display Power Rail ALDO2 is 3.3V ACTIVE");
    }

    // 4. Initialize AMOLED Display (CO5300 QSPI: SCLK=11, CS=12, D0-D3=4-7, RST=8)
    ret = kin_display_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Display initialization failed: %s", esp_err_to_name(ret));
    } else {
        // Clear display & draw official KinPod Boot Splash on the AMOLED panel!
        kin_display_clear();
        kin_display_draw_splash();
    }

    // 5. Initialize Touchscreen CST9220 / FT5x06
    ret = kin_touch_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Touch init warning: %s", esp_err_to_name(ret));
    }

    // 6. Initialize Kin Avatar Subsystem
    kin_avatar_init();

    // 7. Initialize Apple-style Control Center subsystem in PSRAM with Wi-Fi setup callback
    kin_control_center_init(on_wifi_setup_requested);

    // 7b. Initialize 4-Digit PIN Pairing subsystem in PSRAM
    kin_pairing_init();

    // 8. Initialize Kin Audio Subsystem (ES7210 Mic + ES8311 Speaker + NS4150 Amp)
    ret = kin_audio_init();
    if (ret == ESP_OK) {
        // Play acoustic boot chime through the chest speaker!
        kin_audio_play_chime();
        // Start Push-To-Talk monitor task with live WebSocket streaming callback
        kin_audio_start_ptt_task(ptt_audio_stream_cb);
    } else {
        ESP_LOGE(TAG, "Audio subsystem initialization failed: %s", esp_err_to_name(ret));
    }

    // 9. Hold boot splash briefly (2.0s) then transition into active Kin avatar
    vTaskDelay(pdMS_TO_TICKS(2000));
    kin_approval_init();
    kin_avatar_set_active(KIN_AVATAR_CRAB); // Default to OpenClaw Crab (#EF4444)

    // 10. Start Touch Gesture Task on Core 1
    xTaskCreatePinnedToCore(
        touch_gesture_task,
        "touch_gesture",
        4096,
        NULL,
        6,
        NULL,
        1
    );

    // 11. Start Telemetry Monitor Task on Core 1
    xTaskCreatePinnedToCore(
        power_monitor_task,
        "power_monitor",
        3072,
        NULL,
        5,
        NULL,
        1
    );

    // 11. Start Network Manager Task on Core 0 (Wi-Fi + WebSocket)
    xTaskCreatePinnedToCore(
        network_manager_task,
        "net_manager",
        4096,
        NULL,
        4,
        NULL,
        0
    );

    // 12. Start Serial Command Task on Core 0 for Agent-driven configuration over USB
    xTaskCreatePinnedToCore(
        serial_cmd_task,
        "serial_cmd",
        4096,
        NULL,
        3,
        NULL,
        0
    );

    ESP_LOGI(TAG, "KinPod Phase 5 Online: Wi-Fi, WebSocket Hub & Multi-Agent Duplex Voice Active!");
}
