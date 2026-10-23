#pragma once

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define KIN_I2C_PORT        I2C_NUM_0
#define KIN_I2C_SCL_GPIO    14
#define KIN_I2C_SDA_GPIO    15
#define KIN_I2C_FREQ_HZ     400000
#define KIN_I2C_TIMEOUT_MS  100

/**
 * @brief Initializes the shared I2C0 bus on GPIO14/15 with FreeRTOS mutex protection.
 */
esp_err_t kin_i2c_init(void);

/**
 * @brief Thread-safe write of a single byte to a device register.
 */
esp_err_t kin_i2c_write_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t val);

/**
 * @brief Thread-safe read of a single byte from a device register.
 */
esp_err_t kin_i2c_read_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *val);

/**
 * @brief Thread-safe read of multiple bytes starting at a device register.
 */
esp_err_t kin_i2c_read_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t *buf, size_t len);

/**
 * @brief Thread-safe write of multiple bytes to a device register.
 */
esp_err_t kin_i2c_write_bytes(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, size_t len);

/**
 * @brief Acquire shared I2C bus mutex lock.
 */
bool kin_i2c_lock(uint32_t timeout_ms);

/**
 * @brief Release shared I2C bus mutex lock.
 */
void kin_i2c_unlock(void);

#ifdef __cplusplus
}
#endif
