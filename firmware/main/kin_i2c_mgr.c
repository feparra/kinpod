#include "kin_i2c_mgr.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static const char *TAG = "KinI2C";
static SemaphoreHandle_t s_i2c_mutex = NULL;
static bool s_i2c_initialized = false;

esp_err_t kin_i2c_init(void) {
    if (s_i2c_initialized) {
        return ESP_OK;
    }

    if (s_i2c_mutex == NULL) {
        s_i2c_mutex = xSemaphoreCreateMutex();
        if (s_i2c_mutex == NULL) {
            ESP_LOGE(TAG, "Failed to create I2C mutex");
            return ESP_ERR_NO_MEM;
        }
    }

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = KIN_I2C_SDA_GPIO,
        .scl_io_num = KIN_I2C_SCL_GPIO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = KIN_I2C_FREQ_HZ,
        .clk_flags = 0,
    };

    esp_err_t ret = i2c_param_config(KIN_I2C_PORT, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C param config failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = i2c_driver_install(KIN_I2C_PORT, conf.mode, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(ret));
        return ret;
    }

    s_i2c_initialized = true;
    ESP_LOGI(TAG, "I2C0 initialized successfully (SCL=GPIO%d, SDA=GPIO%d at %d Hz)",
             KIN_I2C_SCL_GPIO, KIN_I2C_SDA_GPIO, KIN_I2C_FREQ_HZ);
    return ESP_OK;
}

esp_err_t kin_i2c_write_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t val) {
    return kin_i2c_write_bytes(dev_addr, reg_addr, &val, 1);
}

esp_err_t kin_i2c_read_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *val) {
    return kin_i2c_read_bytes(dev_addr, reg_addr, val, 1);
}

esp_err_t kin_i2c_read_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t *buf, size_t len) {
    if (!s_i2c_initialized || buf == NULL || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    if (xSemaphoreTake(s_i2c_mutex, pdMS_TO_TICKS(KIN_I2C_TIMEOUT_MS)) != pdTRUE) {
        ESP_LOGE(TAG, "I2C mutex timeout on read dev 0x%02X reg 0x%02X", dev_addr, reg_addr);
        return ESP_ERR_TIMEOUT;
    }

    esp_err_t ret = i2c_master_write_read_device(
        KIN_I2C_PORT,
        dev_addr,
        &reg_addr,
        1,
        buf,
        len,
        pdMS_TO_TICKS(KIN_I2C_TIMEOUT_MS)
    );

    xSemaphoreGive(s_i2c_mutex);
    return ret;
}

esp_err_t kin_i2c_write_bytes(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, size_t len) {
    if (!s_i2c_initialized || data == NULL || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    if (xSemaphoreTake(s_i2c_mutex, pdMS_TO_TICKS(KIN_I2C_TIMEOUT_MS)) != pdTRUE) {
        ESP_LOGE(TAG, "I2C mutex timeout on write dev 0x%02X reg 0x%02X", dev_addr, reg_addr);
        return ESP_ERR_TIMEOUT;
    }

    uint8_t buffer[len + 1];
    buffer[0] = reg_addr;
    for (size_t i = 0; i < len; i++) {
        buffer[i + 1] = data[i];
    }

    esp_err_t ret = i2c_master_write_to_device(
        KIN_I2C_PORT,
        dev_addr,
        buffer,
        len + 1,
        pdMS_TO_TICKS(KIN_I2C_TIMEOUT_MS)
    );

    xSemaphoreGive(s_i2c_mutex);
    return ret;
}
