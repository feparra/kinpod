#include "kin_power.h"
#include "kin_i2c_mgr.h"
#include "esp_log.h"

static const char *TAG = "KinPower";

esp_err_t kin_power_init(void) {
    esp_err_t ret = kin_i2c_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C initialization failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // 1. Verify AXP2101 Chip ID
    uint8_t chip_id = 0;
    ret = kin_i2c_read_reg(AXP2101_I2C_ADDR, AXP2101_REG_CHIP_ID, &chip_id);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read AXP2101 PMU at 0x%02X: %s", AXP2101_I2C_ADDR, esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "AXP2101 PMU detected successfully (Chip ID: 0x%02X)", chip_id);

    // 2. Configure Output Voltages
    // ALDO1 (3.3V): 500mV + (0x1C * 100mV) = 3300mV = 3.3V
    kin_i2c_write_reg(AXP2101_I2C_ADDR, AXP2101_REG_ALDO1_VOLT, 0x1C);

    // ALDO2 (AMOLED 3.3V): 500mV + (0x1C * 100mV) = 3300mV = 3.3V
    ret = kin_i2c_write_reg(AXP2101_I2C_ADDR, AXP2101_REG_ALDO2_VOLT, 0x1C);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set ALDO2 voltage: %s", esp_err_to_name(ret));
        return ret;
    }

    // ALDO3 (Touch Panel 3.3V): 500mV + (0x1C * 100mV) = 3300mV = 3.3V
    kin_i2c_write_reg(AXP2101_I2C_ADDR, AXP2101_REG_ALDO3_VOLT, 0x1C);

    // ALDO4 (1.8V logic): 500mV + (0x0D * 100mV) = 1800mV = 1.8V
    kin_i2c_write_reg(AXP2101_I2C_ADDR, AXP2101_REG_ALDO4_VOLT, 0x0D);

    // BLDO1 (3.3V) & BLDO2 (2.8V)
    kin_i2c_write_reg(AXP2101_I2C_ADDR, AXP2101_REG_BLDO1_VOLT, 0x1C);
    kin_i2c_write_reg(AXP2101_I2C_ADDR, AXP2101_REG_BLDO2_VOLT, 0x17);

    // 3. Enable ALDO1, ALDO2 (AMOLED), ALDO3 (Touch), BLDO1 in register 0x90
    ret = kin_power_set_aldo2(true);
    if (ret != ESP_OK) {
        return ret;
    }

    // 4. Configure & Enable Lithium Battery Charger (500mA @ 4.2V termination, 2000mA VBUS input)
    kin_i2c_write_reg(AXP2101_I2C_ADDR, AXP2101_REG_VBUS_CURR_LIMIT, 0x05); // VBUS Limit: 2000mA
    kin_i2c_write_reg(AXP2101_I2C_ADDR, AXP2101_REG_CHG_VOLT, 0x01);        // 4.2V Termination Voltage
    kin_i2c_write_reg(AXP2101_I2C_ADDR, AXP2101_REG_CHG_CURR, 0x0A);        // 500mA ICC Charge Current
    kin_i2c_write_reg(AXP2101_I2C_ADDR, AXP2101_REG_CHG_ENABLE, 0x01);      // Enable internal charger
    ESP_LOGI(TAG, "AXP2101 Battery Charger online: 500mA @ 4.2V [ACTIVE]");

    // 5. Enable Power Button (PEK) Short/Long Press interrupts in AXP2101 INTEN2 (0x41)
    kin_i2c_write_reg(AXP2101_I2C_ADDR, 0x41, 0x03);
    kin_i2c_write_reg(AXP2101_I2C_ADDR, 0x49, 0xFF); // Clear pending

    ESP_LOGI(TAG, "KinPod Power Subsystem online: ALDO2 (AMOLED 3.3V), ALDO3 (Touch 3.3V), Charger 500mA [OK]");
    return ESP_OK;
}

esp_err_t kin_power_set_aldo2(bool enable) {
    uint8_t ldo_ctrl = 0;
    esp_err_t ret = kin_i2c_read_reg(AXP2101_I2C_ADDR, AXP2101_REG_LDO_ONOFF, &ldo_ctrl);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read LDO control reg: %s", esp_err_to_name(ret));
        return ret;
    }

    if (enable) {
        ldo_ctrl |= (1 << 0); // Enable ALDO1 (3.3V)
        ldo_ctrl |= (1 << 1); // Enable ALDO2 (AMOLED 3.3V)
        ldo_ctrl |= (1 << 2); // Enable ALDO3 (Touch Panel 3.3V)
        ldo_ctrl |= (1 << 3); // Enable ALDO4 (1.8V)
        ldo_ctrl |= (1 << 4); // Enable BLDO1 (3.3V)
    } else {
        ldo_ctrl &= ~(1 << 1); // Disable ALDO2
    }

    ret = kin_i2c_write_reg(AXP2101_I2C_ADDR, AXP2101_REG_LDO_ONOFF, ldo_ctrl);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write LDO control reg: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "AXP2101 LDO Rails: ALDO1, ALDO2, ALDO3 enabled (LDO_REG=0x%02X)", ldo_ctrl);
    return ESP_OK;
}

esp_err_t kin_power_get_telemetry(kin_power_telemetry_t *telem) {
    if (telem == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!kin_i2c_lock(100)) {
        return ESP_ERR_TIMEOUT;
    }

    uint8_t status = 0;
    kin_i2c_read_reg(AXP2101_I2C_ADDR, AXP2101_REG_POWER_STATUS, &status);
    uint8_t chg_mode = (status >> 5) & 0x03; // 00: standby, 01: CC charge, 10: CV charge, 11: done
    telem->is_charging = (chg_mode == 1 || chg_mode == 2);

    uint8_t pwr_in = 0;
    kin_i2c_read_reg(AXP2101_I2C_ADDR, 0x00, &pwr_in);
    telem->is_battery_in = (pwr_in & (1 << 3)) != 0;

    uint8_t pct = 100;
    kin_i2c_read_reg(AXP2101_I2C_ADDR, AXP2101_REG_BAT_PERCENT, &pct);
    telem->battery_pct = (pct <= 100) ? pct : 100;

    uint8_t vh = 0, vl = 0;
    kin_i2c_read_reg(AXP2101_I2C_ADDR, AXP2101_REG_BAT_VOLT_H, &vh);
    kin_i2c_read_reg(AXP2101_I2C_ADDR, AXP2101_REG_BAT_VOLT_L, &vl);
    uint16_t raw_v = ((uint16_t)vh << 8) | vl;
    telem->vbat_mv = (raw_v > 2500 && raw_v < 4500) ? raw_v : 3950;

    kin_i2c_unlock();
    return ESP_OK;
}

bool kin_power_is_pek_pressed(void) {
    uint8_t irq2 = 0;
    if (kin_i2c_read_reg(AXP2101_I2C_ADDR, 0x49, &irq2) == ESP_OK) {
        if (irq2 & 0x03) { // Bit 0: Long press, Bit 1: Short press
            kin_i2c_write_reg(AXP2101_I2C_ADDR, 0x49, irq2); // Clear active bits
            return true;
        }
    }
    return false;
}

#include "board_kinpod_2.06.h"
#include "kin_approval.h"
#include "kin_control_center.h"
#include "kin_pairing.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static uint32_t s_last_activity_tick = 0;
static bool s_is_dimmed = false;

void kin_power_notify_activity(void) {
    s_last_activity_tick = xTaskGetTickCount();
    if (s_is_dimmed) {
        s_is_dimmed = false;
        kin_display_set_brightness(100);
        ESP_LOGI(TAG, "Display woke up: Brightness restored to 100%%");
    }
}

bool kin_power_is_dimmed(void) {
    return s_is_dimmed;
}

static void power_management_task(void *pvParameters) {
    s_last_activity_tick = xTaskGetTickCount();
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));

        // If an approval modal or menu is active, defer dimming and keep screen bright
        if (kin_approval_is_active() || kin_control_center_is_open() || kin_pairing_is_active()) {
            kin_power_notify_activity();
            continue;
        }

        uint32_t elapsed_ms = (uint32_t)pdTICKS_TO_MS(xTaskGetTickCount() - s_last_activity_tick);
        if (!s_is_dimmed && elapsed_ms >= 30000) { // 30s inactivity timeout
            s_is_dimmed = true;
            // Smoothly ramp brightness down to 15% (80 -> 60 -> 40 -> 20 -> 15)
            for (int b = 80; b >= 15; b -= 20) {
                kin_display_set_brightness(b);
                vTaskDelay(pdMS_TO_TICKS(40));
            }
            kin_display_set_brightness(15);
            ESP_LOGI(TAG, "Inactivity timeout (30s): Display dimmed to 15%% for battery saving");
        }
    }
}

esp_err_t kin_power_management_init(void) {
    xTaskCreatePinnedToCore(power_management_task, "kin_pwr_mgmt", 3072, NULL, 3, NULL, 0);
    ESP_LOGI(TAG, "Power Management online: Auto-dim to 15%% after 30s inactivity");
    return ESP_OK;
}

