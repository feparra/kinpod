#include "board_kinpod_2.06.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_co5300.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "KinDisplay";

static esp_lcd_panel_handle_t s_panel_handle = NULL;
static esp_lcd_panel_io_handle_t s_io_handle = NULL;

esp_err_t kin_display_init(void) {
    ESP_LOGI(TAG, "Initializing QSPI AMOLED Panel (CO5300, 410x502 px)...");

    // 1. Configure QSPI Bus (SCLK=11, D0=4, D1=5, D2=6, D3=7)
    spi_bus_config_t bus_cfg = CO5300_PANEL_BUS_QSPI_CONFIG(
        KIN_LCD_SCLK,
        KIN_LCD_D0,
        KIN_LCD_D1,
        KIN_LCD_D2,
        KIN_LCD_D3,
        KIN_LCD_H_RES * 40 * sizeof(uint16_t)
    );

    esp_err_t ret = spi_bus_initialize(KIN_LCD_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI bus initialization failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // 2. Configure Panel IO (CS=12)
    esp_lcd_panel_io_spi_config_t io_cfg = CO5300_PANEL_IO_QSPI_CONFIG(
        KIN_LCD_CS,
        NULL,
        NULL
    );
    io_cfg.pclk_hz = KIN_LCD_PCLK_HZ;

    ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)KIN_LCD_HOST, &io_cfg, &s_io_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Panel IO initialization failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // 3. Configure CO5300 Controller (RST=8)
    co5300_vendor_config_t vendor_cfg = {
        .flags = {
            .use_qspi_interface = 1,
        },
    };

    esp_lcd_panel_dev_config_t panel_dev_cfg = {
        .reset_gpio_num = KIN_LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
        .vendor_config = &vendor_cfg,
    };

    ret = esp_lcd_new_panel_co5300(s_io_handle, &panel_dev_cfg, &s_panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "CO5300 panel creation failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // 4. Reset & Initialize Panel
    ESP_LOGI(TAG, "Hardware Resetting CO5300 on GPIO%d...", KIN_LCD_RST);
    ret = esp_lcd_panel_reset(s_panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Panel reset failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Sending CO5300 initialization sequence...");
    ret = esp_lcd_panel_init(s_panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Panel init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // 5. Turn On Display & Set 100% Brightness
    ret = esp_lcd_panel_disp_on_off(s_panel_handle, true);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Panel display on failed: %s", esp_err_to_name(ret));
        return ret;
    }

    esp_lcd_panel_co5300_set_brightness(s_panel_handle, 100);
    ESP_LOGI(TAG, "CO5300 AMOLED Display Online: 410x502 @ 100%% brightness [OK]");

    return ESP_OK;
}

esp_err_t kin_display_set_brightness(uint8_t pct) {
    if (s_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    return esp_lcd_panel_co5300_set_brightness(s_panel_handle, pct);
}

esp_err_t kin_display_clear(void) {
    if (s_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    // Allocate 40 lines of black pixels
    const int chunk_height = 40;
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

// 5x7 Monospace font bitmap table for ASCII characters "KINPOD"
static const uint8_t FONT_K[7] = {0x46, 0x4C, 0x58, 0x70, 0x58, 0x4C, 0x46};
static const uint8_t FONT_I[7] = {0x7C, 0x10, 0x10, 0x10, 0x10, 0x10, 0x7C};
static const uint8_t FONT_N[7] = {0x42, 0x62, 0x52, 0x4A, 0x46, 0x42, 0x42};
static const uint8_t FONT_P[7] = {0x7C, 0x42, 0x42, 0x7C, 0x40, 0x40, 0x40};
static const uint8_t FONT_O[7] = {0x38, 0x44, 0x82, 0x82, 0x82, 0x44, 0x38};
static const uint8_t FONT_D[7] = {0x78, 0x44, 0x42, 0x42, 0x42, 0x44, 0x78};

static void draw_glyph_scaled(uint16_t *buf, int buf_y_start, int buf_h,
                              int gx, int gy, const uint8_t *glyph, int scale, uint16_t color) {
    for (int row = 0; row < 7; row++) {
        uint8_t bits = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (bits & (0x80 >> col)) {
                for (int dy = 0; dy < scale; dy++) {
                    int py = gy + row * scale + dy;
                    if (py >= buf_y_start && py < buf_y_start + buf_h) {
                        for (int dx = 0; dx < scale; dx++) {
                            int px = gx + col * scale + dx;
                            if (px >= 0 && px < KIN_LCD_H_RES) {
                                buf[(py - buf_y_start) * KIN_LCD_H_RES + px] = color;
                            }
                        }
                    }
                }
            }
        }
    }
}

esp_err_t kin_display_draw_splash(void) {
    if (s_panel_handle == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Drawing Official KinPod Boot Splash (AMOLED True-Black & 3-Level Node)...");

    const int chunk_h = 50;
    size_t chunk_size = KIN_LCD_H_RES * chunk_h * sizeof(uint16_t);
    uint16_t *buf = (uint16_t *)heap_caps_malloc(chunk_size, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (!buf) {
        buf = (uint16_t *)malloc(chunk_size);
    }
    if (!buf) {
        return ESP_ERR_NO_MEM;
    }

    // Official 3-level geometry coordinates (centered around x=205, y=210)
    // Box: x in [110, 300], y in [115, 305], radius 40
    const int box_x0 = 110, box_x1 = 300;
    const int box_y0 = 115, box_y1 = 305;
    const uint16_t COLOR_WHITE = 0xFFFF;
    const uint16_t COLOR_GRAY_SHADOW = 0x5AEB;
    const uint16_t COLOR_AMBER = 0xFD20;

    for (int y0 = 0; y0 < KIN_LCD_V_RES; y0 += chunk_h) {
        int h = (y0 + chunk_h <= KIN_LCD_V_RES) ? chunk_h : (KIN_LCD_V_RES - y0);
        memset(buf, 0x00, KIN_LCD_H_RES * h * sizeof(uint16_t)); // AMOLED True Black

        for (int row = 0; row < h; row++) {
            int y = y0 + row;

            // Draw Squircle Outline
            if (y >= box_y0 && y <= box_y1) {
                for (int x = box_x0; x <= box_x1; x++) {
                    // Check border edges
                    bool is_h_edge = (y == box_y0 || y == box_y0 + 1 || y == box_y1 || y == box_y1 - 1);
                    bool is_v_edge = (x == box_x0 || x == box_x0 + 1 || x == box_x1 || x == box_x1 - 1);

                    // Corner rounding
                    int dx = (x < box_x0 + 36) ? (box_x0 + 36 - x) : (x > box_x1 - 36) ? (x - (box_x1 - 36)) : 0;
                    int dy = (y < box_y0 + 36) ? (box_y0 + 36 - y) : (y > box_y1 - 36) ? (y - (box_y1 - 36)) : 0;
                    if (dx > 0 && dy > 0 && (dx * dx + dy * dy > 36 * 36)) {
                        continue;
                    }
                    if (is_h_edge || is_v_edge || (dx > 0 && dy > 0 && dx * dx + dy * dy >= 34 * 34)) {
                        buf[row * KIN_LCD_H_RES + x] = COLOR_WHITE;
                    }

                    // 1. Top Bar: x in [228, 272], y in [170, 206]
                    if (x >= 228 && x <= 272 && y >= 170 && y <= 206) {
                        buf[row * KIN_LCD_H_RES + x] = COLOR_WHITE;
                    }
                    // Top Bar shadow: y in [207, 210], x in [228, 272]
                    if (x >= 228 && x <= 272 && y >= 207 && y <= 210) {
                        buf[row * KIN_LCD_H_RES + x] = COLOR_GRAY_SHADOW;
                    }

                    // 2. Center Bridge: x in [162, 252], y in [210, 246]
                    if (x >= 162 && x <= 252 && y >= 210 && y <= 246) {
                        buf[row * KIN_LCD_H_RES + x] = COLOR_WHITE;
                    }
                    // Center Bridge shadow: y in [247, 250], x in [162, 252]
                    if (x >= 162 && x <= 252 && y >= 247 && y <= 250) {
                        buf[row * KIN_LCD_H_RES + x] = COLOR_GRAY_SHADOW;
                    }

                    // 3. Bottom Bar: x in [138, 182], y in [250, 286]
                    if (x >= 138 && x <= 182 && y >= 250 && y <= 286) {
                        buf[row * KIN_LCD_H_RES + x] = COLOR_WHITE;
                    }
                }
            }
        }

        // Draw Typography: "K I N P O D" at y=350 with scale 3
        const int text_y = 350;
        const int char_scale = 3;
        draw_glyph_scaled(buf, y0, h, 125, text_y, FONT_K, char_scale, COLOR_WHITE);
        draw_glyph_scaled(buf, y0, h, 155, text_y, FONT_I, char_scale, COLOR_WHITE);
        draw_glyph_scaled(buf, y0, h, 180, text_y, FONT_N, char_scale, COLOR_WHITE);
        draw_glyph_scaled(buf, y0, h, 210, text_y, FONT_P, char_scale, COLOR_AMBER);
        draw_glyph_scaled(buf, y0, h, 240, text_y, FONT_O, char_scale, COLOR_AMBER);
        draw_glyph_scaled(buf, y0, h, 270, text_y, FONT_D, char_scale, COLOR_AMBER);

        esp_lcd_panel_draw_bitmap(s_panel_handle, 0, y0, KIN_LCD_H_RES, y0 + h, buf);
    }

    free(buf);
    ESP_LOGI(TAG, "Official KinPod Boot Splash Rendered Successfully on AMOLED Display!");
    return ESP_OK;
}
