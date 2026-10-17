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
    // ALDO2 (AMOLED 3.3V): 500mV + (0x1C * 100mV) = 3300mV = 3.3V
    ret = kin_i2c_write_reg(AXP2101_I2C_ADDR, AXP2101_REG_ALDO2_VOLT, 0x1C);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set ALDO2 voltage: %s", esp_err_to_name(ret));
        return ret;
    }

    // ALDO1 (3.3V) & ALDO3 (3.3V)
    kin_i2c_write_reg(AXP2101_I2C_ADDR, AXP2101_REG_ALDO1_VOLT, 0x1C);
    kin_i2c_write_reg(AXP2101_I2C_ADDR, AXP2101_REG_ALDO3_VOLT, 0x1C);
    kin_i2c_write_reg(AXP2101_I2C_ADDR, AXP2101_REG_BLDO1_VOLT, 0x1C);

    // 3. Enable ALDO2 (Bit 1) and other required LDO rails in register 0x90
    ret = kin_power_set_aldo2(true);
    if (ret != ESP_OK) {
        return ret;
    }

    ESP_LOGI(TAG, "KinPod Power Subsystem online: ALDO2 @ 3.3V active for AMOLED display");
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
        ldo_ctrl |= (1 << 1); // Enable ALDO2
        ldo_ctrl |= (1 << 0); // Enable ALDO1
        ldo_ctrl |= (1 << 2); // Enable ALDO3
        ldo_ctrl |= (1 << 4); // Enable BLDO1
    } else {
        ldo_ctrl &= ~(1 << 1); // Disable ALDO2
    }

    ret = kin_i2c_write_reg(AXP2101_I2C_ADDR, AXP2101_REG_LDO_ONOFF, ldo_ctrl);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write LDO control reg: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "ALDO2 (AMOLED 3.3V) state set to: %s (LDO_REG=0x%02X)", enable ? "ON" : "OFF", ldo_ctrl);
    return ESP_OK;
}

esp_err_t kin_power_get_telemetry(kin_power_telemetry_t *telem) {
    if (telem == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t status = 0;
    kin_i2c_read_reg(AXP2101_I2C_ADDR, AXP2101_REG_POWER_STATUS, &status);
    telem->is_charging = (status & (1 << 6)) != 0;
    telem->is_battery_in = (status & (1 << 5)) != 0;

    uint8_t pct = 100;
    kin_i2c_read_reg(AXP2101_I2C_ADDR, AXP2101_REG_BAT_PERCENT, &pct);
    telem->battery_pct = (pct <= 100) ? pct : 100;

    uint8_t vh = 0, vl = 0;
    kin_i2c_read_reg(AXP2101_I2C_ADDR, AXP2101_REG_BAT_VOLT_H, &vh);
    kin_i2c_read_reg(AXP2101_I2C_ADDR, AXP2101_REG_BAT_VOLT_L, &vl);
    uint16_t raw_v = ((uint16_t)vh << 8) | vl;
    telem->vbat_mv = (raw_v > 2500 && raw_v < 4500) ? raw_v : 3950;

    return ESP_OK;
}
