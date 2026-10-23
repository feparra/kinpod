#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AXP2101_I2C_ADDR    0x34

// AXP2101 Key Registers
#define AXP2101_REG_CHIP_ID     0x03
#define AXP2101_REG_POWER_STATUS 0x01
#define AXP2101_REG_LDO_ONOFF    0x90
#define AXP2101_REG_LDO_ONOFF1   0x91
#define AXP2101_REG_ALDO1_VOLT   0x92
#define AXP2101_REG_ALDO2_VOLT   0x93
#define AXP2101_REG_ALDO3_VOLT   0x94
#define AXP2101_REG_ALDO4_VOLT   0x95
#define AXP2101_REG_BLDO1_VOLT   0x96
#define AXP2101_REG_BLDO2_VOLT   0x97
#define AXP2101_REG_BAT_VOLT_H   0x34
#define AXP2101_REG_BAT_VOLT_L   0x35
#define AXP2101_REG_BAT_PERCENT  0xA4
#define AXP2101_REG_VBUS_CURR_LIMIT 0x18
#define AXP2101_REG_CHG_VOLT        0x61
#define AXP2101_REG_CHG_CURR        0x62
#define AXP2101_REG_CHG_ENABLE      0x69

/**
 * @brief Telemetry data read from AXP2101 PMU.
 */
typedef struct {
    uint16_t vbat_mv;       /*!< Battery voltage in millivolts */
    uint8_t  battery_pct;   /*!< Battery state of charge (0-100%) */
    bool     is_charging;   /*!< True if USB charging is active */
    bool     is_battery_in; /*!< True if battery is connected */
} kin_power_telemetry_t;

/**
 * @brief Initializes AXP2101 PMU, verifies chip ID, and activates ALDO2 @ 3.3V.
 */
esp_err_t kin_power_init(void);

/**
 * @brief Enables or disables the ALDO2 power rail (AMOLED display power).
 * @param enable true to turn on 3.3V, false to power down.
 */
esp_err_t kin_power_set_aldo2(bool enable);

/**
 * @brief Reads real-time battery voltage, percentage, and charging state.
 */
esp_err_t kin_power_get_telemetry(kin_power_telemetry_t *telem);

/**
 * @brief Checks if the physical PWR button (AXP2101 PEK) was pressed.
 */
bool kin_power_is_pek_pressed(void);

/**
 * @brief Notifies the power management system of user activity (touch, button, speech).
 * Resets the inactivity timer and restores 100% brightness if dimmed.
 */
void kin_power_notify_activity(void);

/**
 * @brief Initializes the power management auto-dimming task.
 * Dims display to 15% brightness after 30 seconds of inactivity.
 */
esp_err_t kin_power_management_init(void);

/**
 * @brief Returns true if the display is currently dimmed.
 */
bool kin_power_is_dimmed(void);

#ifdef __cplusplus
}
#endif
