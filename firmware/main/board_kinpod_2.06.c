#include "board_kinpod_2.06.h"
#include "kin_i2c_mgr.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "bsp/esp32_s3_touch_amoled_2_06.h"
#include "bsp/display.h"
#include "bsp/touch.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_touch.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "KinDisplay";

static esp_lcd_panel_handle_t s_panel_handle = NULL;
static esp_lcd_panel_io_handle_t s_io_handle = NULL;
static esp_lcd_touch_handle_t s_touch_handle = NULL;
static SemaphoreHandle_t s_trans_done_sem = NULL;

// Raw splash asset embedded by CMake
extern const uint8_t boot_splash_start[] asm("_binary_boot_splash_raw_start");
extern const uint8_t boot_splash_end[]   asm("_binary_boot_splash_raw_end");

static bool s_on_color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx) {
    BaseType_t need_yield = pdFALSE;
    if (s_trans_done_sem) {
        xSemaphoreGiveFromISR(s_trans_done_sem, &need_yield);
    }
    return need_yield == pdTRUE;
}

esp_err_t kin_display_init(void) {
    ESP_LOGI(TAG, "Initializing Waveshare 2.06\" AMOLED Panel via official BSP...");

    bsp_display_config_t disp_cfg = {
        .max_transfer_sz = KIN_LCD_H_RES * 50 * sizeof(uint16_t),
    };

    esp_err_t ret = bsp_display_new(&disp_cfg, &s_panel_handle, &s_io_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "bsp_display_new failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Create binary semaphore for DMA transfer synchronization
    if (s_trans_done_sem == NULL) {
        s_trans_done_sem = xSemaphoreCreateBinary();
    }

    // Register callback for DMA completion
    const esp_lcd_panel_io_callbacks_t cbs = {
        .on_color_trans_done = s_on_color_trans_done,
    };
    esp_lcd_panel_io_register_event_callbacks(s_io_handle, &cbs, NULL);

    // Set 100% Brightness
    bsp_display_brightness_set(100);
    ESP_LOGI(TAG, "Waveshare AMOLED Display Online: 410x502 @ 100%% brightness [OK]");

    return ESP_OK;
}

esp_err_t kin_display_set_brightness(uint8_t pct) {
    if (s_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return bsp_display_brightness_set(pct);
}

esp_err_t kin_display_clear(void) {
    if (s_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    const int chunk_height = 50;
    size_t chunk_size = KIN_LCD_H_RES * chunk_height * sizeof(uint16_t);
    uint16_t *line_buf = (uint16_t *)heap_caps_malloc(chunk_size, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (!line_buf) {
        line_buf = (uint16_t *)malloc(chunk_size);
    }
    if (!line_buf) {
        return ESP_ERR_NO_MEM;
    }
    memset(line_buf, 0x00, chunk_size);

    for (int y = 0; y < KIN_LCD_V_RES; y += chunk_height) {
        int h = (y + chunk_height <= KIN_LCD_V_RES) ? chunk_height : (KIN_LCD_V_RES - y);
        esp_lcd_panel_draw_bitmap(s_panel_handle, 0, y, KIN_LCD_H_RES, y + h, line_buf);
        if (s_trans_done_sem) {
            xSemaphoreTake(s_trans_done_sem, pdMS_TO_TICKS(100));
        }
    }

    free(line_buf);
    return ESP_OK;
}

esp_err_t kin_display_draw_region(int x_start, int y_start, int x_end, int y_end, const uint16_t *pixels) {
    if (s_panel_handle == NULL || pixels == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    esp_err_t ret = esp_lcd_panel_draw_bitmap(s_panel_handle, x_start, y_start, x_end, y_end, pixels);
    if (ret == ESP_OK && s_trans_done_sem) {
        xSemaphoreTake(s_trans_done_sem, pdMS_TO_TICKS(100));
    }
    return ret;
}

static uint8_t *s_sprite_dma_buf = NULL;
#define SPRITE_CHUNK_LINES 40

esp_err_t kin_display_draw_sprite(int x, int y, int w, int h, const uint8_t *raw_sprite) {
    if (s_panel_handle == NULL || raw_sprite == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    const size_t chunk_bytes = w * SPRITE_CHUNK_LINES * sizeof(uint16_t);

    if (!s_sprite_dma_buf) {
        s_sprite_dma_buf = (uint8_t *)heap_caps_malloc(chunk_bytes, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
        if (!s_sprite_dma_buf) {
            s_sprite_dma_buf = (uint8_t *)malloc(chunk_bytes);
        }
        if (!s_sprite_dma_buf) {
            ESP_LOGE(TAG, "Failed to allocate DMA buffer for sprite transfer");
            return ESP_ERR_NO_MEM;
        }
    }

    const uint16_t *src_pixels = (const uint16_t *)raw_sprite;

    for (int cur_y = 0; cur_y < h; cur_y += SPRITE_CHUNK_LINES) {
        int lines = (cur_y + SPRITE_CHUNK_LINES <= h) ? SPRITE_CHUNK_LINES : (h - cur_y);
        size_t transfer_bytes = w * lines * sizeof(uint16_t);

        // Copy contiguous sprite rows into DMA buffer
        memcpy(s_sprite_dma_buf, &src_pixels[cur_y * w], transfer_bytes);

        // Draw ONLY the exact sprite window [x .. x+w, y+cur_y .. y+cur_y+lines]
        esp_lcd_panel_draw_bitmap(s_panel_handle, x, y + cur_y, x + w, y + cur_y + lines, s_sprite_dma_buf);

        if (s_trans_done_sem) {
            xSemaphoreTake(s_trans_done_sem, pdMS_TO_TICKS(100));
        }
    }

    return ESP_OK;
}

esp_err_t kin_display_draw_raw_frame(const uint8_t *raw_frame) {
    if (s_panel_handle == NULL || raw_frame == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    const int chunk_lines = 50;
    const size_t line_bytes = KIN_LCD_H_RES * sizeof(uint16_t);
    const size_t chunk_bytes = line_bytes * chunk_lines;

    // Allocate single DMA-capable RAM transfer buffer
    uint8_t *dma_buf = (uint8_t *)heap_caps_malloc(chunk_bytes, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (!dma_buf) {
        dma_buf = (uint8_t *)malloc(chunk_bytes);
    }
    if (!dma_buf) {
        ESP_LOGE(TAG, "Failed to allocate DMA buffer for frame transfer");
        return ESP_ERR_NO_MEM;
    }

    for (int y = 0; y < KIN_LCD_V_RES; y += chunk_lines) {
        int h = (y + chunk_lines <= KIN_LCD_V_RES) ? chunk_lines : (KIN_LCD_V_RES - y);
        size_t current_chunk_size = h * line_bytes;

        // Copy chunk from embedded flash image into DMA RAM
        memcpy(dma_buf, raw_frame + (y * line_bytes), current_chunk_size);

        // Send to AMOLED via QSPI
        esp_lcd_panel_draw_bitmap(s_panel_handle, 0, y, KIN_LCD_H_RES, y + h, dma_buf);

        // Synchronize: wait until hardware DMA completes before copying next chunk or freeing
        if (s_trans_done_sem) {
            xSemaphoreTake(s_trans_done_sem, pdMS_TO_TICKS(100));
        }
    }

    free(dma_buf);
    return ESP_OK;
}

esp_err_t kin_display_draw_splash(void) {
    ESP_LOGI(TAG, "Streaming Official KinPod Boot Splash...");
    esp_err_t ret = kin_display_draw_raw_frame(boot_splash_start);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Boot Splash Streamed Successfully!");
    }
    return ret;
}

#include "esp_lcd_touch_ft5x06.h"

esp_err_t kin_touch_init(void) {
    if (s_touch_handle != NULL) {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initializing Touchscreen Controller...");

    i2c_master_bus_handle_t i2c_bus = bsp_i2c_get_handle();
    if (!i2c_bus) {
        ESP_LOGE(TAG, "I2C master bus handle is NULL");
        return ESP_ERR_INVALID_STATE;
    }

    // 1. Hardware Reset: Pulse GPIO 9 (BSP_LCD_TOUCH_RST) to bring touch chip out of reset
    gpio_config_t rst_io = {
        .pin_bit_mask = BIT64(GPIO_NUM_9),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&rst_io);
    gpio_set_level(GPIO_NUM_9, 0); // Assert reset
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(GPIO_NUM_9, 1); // Release reset
    vTaskDelay(pdMS_TO_TICKS(60)); // Allow touch MCU to initialize

    // 2. Scan I2C0 Bus to detect touch controller
    ESP_LOGI(TAG, "Scanning I2C0 bus for touch devices...");
    uint8_t touch_addr = 0x38;
    bool found = false;

    for (uint8_t addr = 0x08; addr < 0x78; addr++) {
        if (i2c_master_probe(i2c_bus, addr, 50) == ESP_OK) {
            ESP_LOGI(TAG, "  -> Found I2C device at 0x%02X", addr);
            if (addr == 0x38 || addr == 0x5A || addr == 0x14 || addr == 0x24) {
                touch_addr = addr;
                found = true;
            }
        }
    }

    if (found) {
        ESP_LOGI(TAG, "Active Touchscreen Controller confirmed at I2C address 0x%02X [OK]", touch_addr);
    } else {
        ESP_LOGW(TAG, "No known touch address acknowledged; attempting default 0x%02X", touch_addr);
    }

    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = KIN_LCD_H_RES,
        .y_max = KIN_LCD_V_RES,
        .rst_gpio_num = GPIO_NUM_9,
        .int_gpio_num = GPIO_NUM_38,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0,
        },
    };

    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_FT5x06_CONFIG();
    tp_io_config.dev_addr = touch_addr;
    tp_io_config.scl_speed_hz = 100000;

    esp_err_t ret = esp_lcd_new_panel_io_i2c(i2c_bus, &tp_io_config, &tp_io_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_lcd_new_panel_io_i2c failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_lcd_touch_new_i2c_ft5x06(tp_io_handle, &tp_cfg, &s_touch_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_lcd_touch_new_i2c_ft5x06 failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Touchscreen Online: Controller Active on I2C0 [OK]");
    return ESP_OK;
}

bool kin_touch_get_point(uint16_t *x, uint16_t *y) {
    if (s_touch_handle == NULL) {
        return false;
    }

    if (!kin_i2c_lock(20)) {
        return false;
    }

    esp_err_t ret = esp_lcd_touch_read_data(s_touch_handle);
    if (ret != ESP_OK) {
        kin_i2c_unlock();
        return false;
    }

    esp_lcd_touch_point_data_t pt;
    uint8_t count = 0;
    ret = esp_lcd_touch_get_data(s_touch_handle, &pt, &count, 1);
    kin_i2c_unlock();

    if (ret == ESP_OK && count > 0) {
        if (x) *x = pt.x;
        if (y) *y = pt.y;
        return true;
    }

    return false;
}
