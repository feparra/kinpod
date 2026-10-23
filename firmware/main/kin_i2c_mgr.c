#include "kin_i2c_mgr.h"
#include "esp_log.h"
#include "bsp/esp32_s3_touch_amoled_2_06.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <string.h>

static const char *TAG = "KinI2C";
static i2c_master_dev_handle_t s_dev_handles[128] = {0};
static SemaphoreHandle_t s_i2c_mutex = NULL;

static i2c_master_dev_handle_t get_dev_handle(uint8_t dev_addr) {
    if (dev_addr >= 128) return NULL;
    if (s_dev_handles[dev_addr] != NULL) {
        return s_dev_handles[dev_addr];
    }
    i2c_master_bus_handle_t bus = bsp_i2c_get_handle();
    if (!bus) {
        ESP_LOGE(TAG, "Failed to get I2C bus handle from BSP");
        return NULL;
    }
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = dev_addr,
        .scl_speed_hz = KIN_I2C_FREQ_HZ,
    };
    esp_err_t err = i2c_master_bus_add_device(bus, &dev_cfg, &s_dev_handles[dev_addr]);
    if (err != ESP_OK) {
        return NULL;
    }
    return s_dev_handles[dev_addr];
}

esp_err_t kin_i2c_init(void) {
    if (s_i2c_mutex == NULL) {
        s_i2c_mutex = xSemaphoreCreateRecursiveMutex();
    }
    esp_err_t ret = bsp_i2c_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "bsp_i2c_init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "I2C0 master bus active via Waveshare BSP (SCL=GPIO%d, SDA=GPIO%d at %d Hz)",
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
    if (buf == NULL || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    if (s_i2c_mutex && xSemaphoreTakeRecursive(s_i2c_mutex, pdMS_TO_TICKS(KIN_I2C_TIMEOUT_MS)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    i2c_master_dev_handle_t dev = get_dev_handle(dev_addr);
    if (!dev) {
        if (s_i2c_mutex) xSemaphoreGiveRecursive(s_i2c_mutex);
        return ESP_ERR_NOT_FOUND;
    }

    esp_err_t ret = i2c_master_transmit_receive(dev, &reg_addr, 1, buf, len, KIN_I2C_TIMEOUT_MS);
    if (s_i2c_mutex) xSemaphoreGiveRecursive(s_i2c_mutex);
    return ret;
}

esp_err_t kin_i2c_write_bytes(uint8_t dev_addr, uint8_t reg_addr, const uint8_t *data, size_t len) {
    if (data == NULL || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    if (s_i2c_mutex && xSemaphoreTakeRecursive(s_i2c_mutex, pdMS_TO_TICKS(KIN_I2C_TIMEOUT_MS)) != pdTRUE) {
        return ESP_ERR_TIMEOUT;
    }

    i2c_master_dev_handle_t dev = get_dev_handle(dev_addr);
    if (!dev) {
        if (s_i2c_mutex) xSemaphoreGiveRecursive(s_i2c_mutex);
        return ESP_ERR_NOT_FOUND;
    }

    uint8_t buffer[len + 1];
    buffer[0] = reg_addr;
    memcpy(&buffer[1], data, len);

    esp_err_t ret = i2c_master_transmit(dev, buffer, len + 1, KIN_I2C_TIMEOUT_MS);
    if (s_i2c_mutex) xSemaphoreGiveRecursive(s_i2c_mutex);
    return ret;
}

bool kin_i2c_lock(uint32_t timeout_ms) {
    if (!s_i2c_mutex) return true;
    return xSemaphoreTakeRecursive(s_i2c_mutex, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

void kin_i2c_unlock(void) {
    if (s_i2c_mutex) xSemaphoreGiveRecursive(s_i2c_mutex);
}
