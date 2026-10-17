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
#define AXP2101_REG_ALDO1_VOLT   0x91
#define AXP2101_REG_ALDO2_VOLT   0x92
#define AXP2101_REG_ALDO3_VOLT   0x93
#define AXP2101_REG_ALDO4_VOLT   0x94
#define AXP2101_REG_BLDO1_VOLT   0x95
#define AXP2101_REG_BLDO2_VOLT   0x96
#define AXP2101_REG_BAT_VOLT_H   0x34
#define AXP2101_REG_BAT_VOLT_L   0x35
#define AXP2101_REG_BAT_PERCENT  0xA4

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

#ifdef __cplusplus
}
#endif
