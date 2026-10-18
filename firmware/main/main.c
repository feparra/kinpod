#include <stdio.h>
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

static const char *TAG = "KinPod-Main";

static void scan_i2c_bus(void) {
    ESP_LOGI(TAG, "Scanning shared I2C0 bus (SCL=GPIO14, SDA=GPIO15)...");
    int count = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        uint8_t dummy = 0;
        if (kin_i2c_read_reg(addr, 0x00, &dummy) == ESP_OK) {
            ESP_LOGI(TAG, "  -> Found I2C device at address: 0x%02X", addr);
            count++;
        }
    }
    ESP_LOGI(TAG, "I2C scan complete: %d device(s) detected.", count);
}

static void power_monitor_task(void *pvParameters) {
    kin_power_telemetry_t telem = {0};
    while (1) {
        if (kin_power_get_telemetry(&telem) == ESP_OK) {
            ESP_LOGI(TAG, "Telemetry: VBat=%u mV | SOC=%u%% | Charging=%s",
                     telem.vbat_mv, telem.battery_pct, telem.is_charging ? "YES" : "NO");
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "=================================================");
    ESP_LOGI(TAG, "  KinPod (KinOS) v6.0 Firmware — Phase 3 Display ");
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

    // 4. I2C Peripheral Audit (Scan Touch & Codec addresses)
    scan_i2c_bus();

    // 5. Initialize AMOLED Display (CO5300 QSPI: SCLK=11, CS=12, D0-D3=4-7, RST=8)
    ret = kin_display_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Display initialization failed: %s", esp_err_to_name(ret));
    } else {
        // 6. Clear display & draw official KinPod Boot Splash on the AMOLED panel!
        kin_display_clear();
        kin_display_draw_splash();
    }

    // 7. Start Telemetry Monitor Task
    xTaskCreatePinnedToCore(
        power_monitor_task,
        "power_monitor",
        3072,
        NULL,
        5,
        NULL,
        1
    );

    ESP_LOGI(TAG, "KinPod Phase 3 Initialized: Boot Splash Rendered on Physical AMOLED Screen!");
}
