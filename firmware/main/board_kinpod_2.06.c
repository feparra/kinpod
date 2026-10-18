#include "board_kinpod_2.06.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "bsp/esp32_s3_touch_amoled_2_06.h"
#include "bsp/display.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "KinDisplay";

static esp_lcd_panel_handle_t s_panel_handle = NULL;
static esp_lcd_panel_io_handle_t s_io_handle = NULL;

// Raw splash asset embedded by CMake
extern const uint8_t boot_splash_start[] asm("_binary_boot_splash_raw_start");
extern const uint8_t boot_splash_end[]   asm("_binary_boot_splash_raw_end");

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
    }

    free(line_buf);
    return ESP_OK;
}

esp_err_t kin_display_draw_splash(void) {
    if (s_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Streaming Official Anti-Aliased KinPod Splash (Embedded Asset, 410x502)...");

    const int chunk_lines = 50;
    const size_t line_bytes = KIN_LCD_H_RES * sizeof(uint16_t);
    const size_t chunk_bytes = line_bytes * chunk_lines;

    // Allocate single DMA-capable RAM transfer buffer
    uint8_t *dma_buf = (uint8_t *)heap_caps_malloc(chunk_bytes, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (!dma_buf) {
        dma_buf = (uint8_t *)malloc(chunk_bytes);
    }
    if (!dma_buf) {
        ESP_LOGE(TAG, "Failed to allocate DMA buffer for splash transfer");
        return ESP_ERR_NO_MEM;
    }

    for (int y = 0; y < KIN_LCD_V_RES; y += chunk_lines) {
        int h = (y + chunk_lines <= KIN_LCD_V_RES) ? chunk_lines : (KIN_LCD_V_RES - y);
        size_t current_chunk_size = h * line_bytes;

        // Copy chunk from embedded flash image into DMA RAM
        memcpy(dma_buf, boot_splash_start + (y * line_bytes), current_chunk_size);

        // Send to AMOLED via QSPI
        esp_lcd_panel_draw_bitmap(s_panel_handle, 0, y, KIN_LCD_H_RES, y + h, dma_buf);
    }

    free(dma_buf);
    ESP_LOGI(TAG, "Official KinPod Boot Splash Streamed Successfully to AMOLED Display!");
    return ESP_OK;
}
