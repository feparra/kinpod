#include "kin_wifi.h"
#include "kin_provisioning.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <string.h>

static const char *TAG = "KinWiFi";

static EventGroupHandle_t s_wifi_event_group = NULL;
#define WIFI_CONNECTED_BIT BIT0

static bool s_is_connected = false;
static int s_retry_num = 0;
#define MAX_RETRY_COUNT 10

#include "esp_sntp.h"

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Wi-Fi Station started, connecting to AP...");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        s_is_connected = false;
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        if (!kin_provisioning_is_active()) {
            s_retry_num++;
            ESP_LOGW(TAG, "Wi-Fi disconnected. Reconnecting (attempt %d)...", s_retry_num);
            vTaskDelay(pdMS_TO_TICKS(1500));
            esp_wifi_connect();
        } else {
            ESP_LOGI(TAG, "Station disconnected while Provisioning SoftAP is active (suppressing reconnect)");
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Wi-Fi Connected! KinPod IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        s_is_connected = true;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);

        if (!esp_sntp_enabled()) {
            ESP_LOGI(TAG, "Initializing SNTP time synchronization...");
            esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
            esp_sntp_setservername(0, "pool.ntp.org");
            esp_sntp_setservername(1, "time.google.com");
            esp_sntp_init();
        }
    }
}

static bool s_netif_initialized = false;

esp_err_t kin_wifi_init(const char *ssid, const char *pass) {
    ESP_LOGI(TAG, "Initializing KinPod Wi-Fi Subsystem...");

    if (!s_netif_initialized) {
        s_wifi_event_group = xEventGroupCreate();

        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_create_default_wifi_sta();
        esp_netif_create_default_wifi_ap();

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        esp_event_handler_instance_t instance_any_id;
        esp_event_handler_instance_t instance_got_ip;
        ESP_ERROR_CHECK(esp_event_handler_instance_register(
            WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(
            IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip));

        s_netif_initialized = true;
    }

    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };

    const char *target_ssid = ssid ? ssid : KIN_DEFAULT_WIFI_SSID;
    const char *target_pass = pass ? pass : KIN_DEFAULT_WIFI_PASS;

    strncpy((char *)wifi_config.sta.ssid, target_ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, target_pass, sizeof(wifi_config.sta.password) - 1);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi configured for SSID: '%s'", target_ssid);
    return ESP_OK;
}

bool kin_wifi_is_connected(void) {
    return s_is_connected;
}

int8_t kin_wifi_get_rssi(void) {
    if (!s_is_connected) return -100;
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        return ap_info.rssi;
    }
    return -100;
}

esp_err_t kin_wifi_get_connected_ssid(char *ssid_out, size_t max_len) {
    if (!s_is_connected || !ssid_out || max_len == 0) return ESP_ERR_INVALID_STATE;
    wifi_ap_record_t ap_info;
    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
        strncpy(ssid_out, (const char *)ap_info.ssid, max_len - 1);
        ssid_out[max_len - 1] = '\0';
        return ESP_OK;
    }
    return ESP_FAIL;
}
