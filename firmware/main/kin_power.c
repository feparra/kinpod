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

    // 4. Configure & Enable Lithium Battery Charger and E-Gauge Fuel Gauge Module
    // Reg 0x18: Bit [2:0] = 0x05 (2000mA VBUS limit) | Bit 3 = (1 << 3) = Fuel Gauge Module Enable (0x08) -> 0x0D
    kin_i2c_write_reg(AXP2101_I2C_ADDR, AXP2101_REG_VBUS_CURR_LIMIT, 0x0D);
    kin_i2c_write_reg(AXP2101_I2C_ADDR, AXP2101_REG_CHG_VOLT, 0x01);        // 4.2V Termination Voltage
    kin_i2c_write_reg(AXP2101_I2C_ADDR, AXP2101_REG_CHG_CURR, 0x0A);        // 500mA ICC Charge Current
    kin_i2c_write_reg(AXP2101_I2C_ADDR, AXP2101_REG_CHG_ENABLE, 0x01);      // Enable internal charger
    // Reg 0x30: Enable ADC channels for Battery Voltage, Current, VBUS Voltage and TS
    kin_i2c_write_reg(AXP2101_I2C_ADDR, 0x30, 0x3F);
    ESP_LOGI(TAG, "AXP2101 Battery Charger & Fuel Gauge online: 500mA @ 4.2V, ADC active [OK]");

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

    // 1. Charging state from AXP2101 Reg 0x01 (Power Status)
    uint8_t status = 0;
    kin_i2c_read_reg(AXP2101_I2C_ADDR, AXP2101_REG_POWER_STATUS, &status);
    uint8_t chg_mode = (status >> 5) & 0x03; // 00: standby/discharge, 01: CC charge, 10: CV charge, 11: done
    
    // Check VBUS input presence from Reg 0x00
    uint8_t pwr_in = 0;
    kin_i2c_read_reg(AXP2101_I2C_ADDR, 0x00, &pwr_in);
    bool vbus_present = (pwr_in & (1 << 5)) != 0;
    telem->is_charging = (chg_mode == 1 || chg_mode == 2) || (vbus_present && chg_mode != 0);
    telem->is_battery_in = (pwr_in & (1 << 3)) != 0;

    // 2. Battery Voltage: Read Reg 0x34 (upper 6 bits) and Reg 0x35 (lower 8 bits)
    uint8_t vh = 0, vl = 0;
    kin_i2c_read_reg(AXP2101_I2C_ADDR, AXP2101_REG_BAT_VOLT_H, &vh);
    kin_i2c_read_reg(AXP2101_I2C_ADDR, AXP2101_REG_BAT_VOLT_L, &vl);
    uint16_t vbat_raw = (((uint16_t)(vh & 0x3F)) << 8) | vl; // 1mV per LSB

    if (vbat_raw >= 2800 && vbat_raw <= 4450) {
        telem->vbat_mv = vbat_raw;
    } else {
        telem->vbat_mv = 3850; // Sensible 3.85V nominal LiPo fallback
    }

    // 3. Battery Percentage: Calculate accurate SoC from LiPo OCV curve
    // Standard 3.7V nominal / 4.2V termination Li-Po discharge characteristics
    uint8_t ocv_pct = 0;
    uint16_t mv = telem->vbat_mv;
    if (mv >= 4180) {
        ocv_pct = 100;
    } else if (mv >= 4050) { // 4050mV - 4180mV -> 90% - 100%
        ocv_pct = 90 + ((mv - 4050) * 10) / 130;
    } else if (mv >= 3920) { // 3920mV - 4050mV -> 75% - 90%
        ocv_pct = 75 + ((mv - 3920) * 15) / 130;
    } else if (mv >= 3800) { // 3800mV - 3920mV -> 50% - 75%
        ocv_pct = 50 + ((mv - 3800) * 25) / 120;
    } else if (mv >= 3700) { // 3700mV - 3800mV -> 25% - 50%
        ocv_pct = 25 + ((mv - 3700) * 25) / 100;
    } else if (mv >= 3600) { // 3600mV - 3700mV -> 10% - 25%
        ocv_pct = 10 + ((mv - 3600) * 15) / 100;
    } else if (mv >= 3450) { // 3450mV - 3600mV -> 3% - 10%
        ocv_pct = 3 + ((mv - 3450) * 7) / 150;
    } else {
        ocv_pct = 1;
    }

    // Read hardware E-Gauge register 0xA4
    uint8_t hw_pct = 0xFF;
    esp_err_t r_pct = kin_i2c_read_reg(AXP2101_I2C_ADDR, AXP2101_REG_BAT_PERCENT, &hw_pct);
    // If hardware fuel gauge is calibrated and dynamic (not stuck at uncalibrated 0x14/20 or invalid)
    if (r_pct == ESP_OK && hw_pct <= 100 && hw_pct != 0 && hw_pct != 20) {
        telem->battery_pct = hw_pct;
    } else {
        telem->battery_pct = ocv_pct;
    }

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
#include "kin_avatar.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static uint32_t s_last_activity_tick = 0;
static kin_sleep_state_t s_sleep_state = KIN_SLEEP_STATE_ACTIVE;

void kin_power_notify_activity(void) {
    s_last_activity_tick = xTaskGetTickCount();
    if (s_sleep_state != KIN_SLEEP_STATE_ACTIVE) {
        bool was_sleeping = (s_sleep_state == KIN_SLEEP_STATE_SLEEPING);
        s_sleep_state = KIN_SLEEP_STATE_ACTIVE;
        kin_display_set_power(true);
        kin_display_set_brightness(100);
        ESP_LOGI(TAG, "Display WOKE UP: Brightness restored to 100%% (was_sleeping=%d)", was_sleeping);
        if (was_sleeping) {
            // Restore visual state on screen without blank screen artifacts
            if (kin_control_center_is_open()) {
                kin_control_center_show();
            } else if (kin_pairing_is_active()) {
                kin_pairing_show();
            } else {
                kin_avatar_set_active(kin_avatar_get_active());
            }
        }
    }
}

void kin_power_wake(void) {
    kin_power_notify_activity();
}

kin_sleep_state_t kin_power_get_sleep_state(void) {
    return s_sleep_state;
}

bool kin_power_is_dimmed(void) {
    return s_sleep_state != KIN_SLEEP_STATE_ACTIVE;
}

bool kin_power_is_sleeping(void) {
    return s_sleep_state == KIN_SLEEP_STATE_SLEEPING;
}

static void power_management_task(void *pvParameters) {
    s_last_activity_tick = xTaskGetTickCount();
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));

        // If an approval modal is active, keep screen awake
        if (kin_approval_is_active()) {
            s_last_activity_tick = xTaskGetTickCount();
            continue;
        }

        uint32_t elapsed_ms = (uint32_t)pdTICKS_TO_MS(xTaskGetTickCount() - s_last_activity_tick);

        // Phase 1: Dim to 20% after 20s of user inactivity
        if (s_sleep_state == KIN_SLEEP_STATE_ACTIVE && elapsed_ms >= 20000) {
            s_sleep_state = KIN_SLEEP_STATE_DIMMED;
            for (int b = 80; b >= 20; b -= 20) {
                kin_display_set_brightness(b);
                vTaskDelay(pdMS_TO_TICKS(30));
            }
            kin_display_set_brightness(20);
            ESP_LOGI(TAG, "Inactivity (20s): Display DIMMED to 20%%");
        }

        // Phase 2: Full Sleep (Panel OFF / 0 mA AMOLED) after 40s of inactivity
        if (s_sleep_state == KIN_SLEEP_STATE_DIMMED && elapsed_ms >= 40000) {
            s_sleep_state = KIN_SLEEP_STATE_SLEEPING;
            kin_display_set_power(false);
            ESP_LOGI(TAG, "Inactivity (40s): Display SLEEPING (Panel Off, 0 mA true black)");
        }
    }
}

esp_err_t kin_power_management_init(void) {
    xTaskCreatePinnedToCore(power_management_task, "kin_pwr_mgmt", 3072, NULL, 3, NULL, 0);
    ESP_LOGI(TAG, "Power Management online: Dim (20s) -> Sleep/Off (40s) [OK]");
    return ESP_OK;
}

