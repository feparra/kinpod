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

// Big-Endian RGB565 color macro for Waveshare AMOLED
#define COLOR_RGB565(r, g, b) (__builtin_bswap16((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3)))

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

    // Color definitions (Big-Endian swapped for AMOLED)
    const uint16_t COLOR_WHITE = COLOR_RGB565(255, 255, 255);
    const uint16_t COLOR_GRAY_SHADOW = COLOR_RGB565(90, 95, 110);
    const uint16_t COLOR_AMBER = COLOR_RGB565(255, 107, 0); // #FF6B00

    // Squircle Geometry centered horizontally: X in [115, 295], Y in [110, 290] (size 180x180)
    const int box_x0 = 115, box_x1 = 295;
    const int box_y0 = 110, box_y1 = 290;
    const int radius = 38;

    for (int y0 = 0; y0 < KIN_LCD_V_RES; y0 += chunk_h) {
        int h = (y0 + chunk_h <= KIN_LCD_V_RES) ? chunk_h : (KIN_LCD_V_RES - y0);
        memset(buf, 0x00, KIN_LCD_H_RES * h * sizeof(uint16_t)); // AMOLED True Black

        for (int row = 0; row < h; row++) {
            int y = y0 + row;

            if (y >= box_y0 && y <= box_y1) {
                for (int x = box_x0; x <= box_x1; x++) {
                    // Outer border thickness 4px
                    bool is_h_edge = (y <= box_y0 + 3 || y >= box_y1 - 3);
                    bool is_v_edge = (x <= box_x0 + 3 || x >= box_x1 - 3);

                    // Corner calculation
                    int dx = (x < box_x0 + radius) ? (box_x0 + radius - x) : (x > box_x1 - radius) ? (x - (box_x1 - radius)) : 0;
                    int dy = (y < box_y0 + radius) ? (box_y0 + radius - y) : (y > box_y1 - radius) ? (y - (box_y1 - radius)) : 0;
                    int dist_sq = dx * dx + dy * dy;

                    if (dx > 0 && dy > 0) {
                        if (dist_sq > radius * radius) {
                            continue; // Outside squircle
                        }
                        if (dist_sq >= (radius - 4) * (radius - 4)) {
                            buf[row * KIN_LCD_H_RES + x] = COLOR_WHITE;
                        }
                    } else if (is_h_edge || is_v_edge) {
                        buf[row * KIN_LCD_H_RES + x] = COLOR_WHITE;
                    }

                    // 1. Top Bar: x in [228, 272], y in [162, 196]
                    if (x >= 228 && x <= 272 && y >= 162 && y <= 196) {
                        buf[row * KIN_LCD_H_RES + x] = COLOR_WHITE;
                    }
                    if (x >= 228 && x <= 272 && y >= 197 && y <= 200) {
                        buf[row * KIN_LCD_H_RES + x] = COLOR_GRAY_SHADOW;
                    }

                    // 2. Center Bridge: x in [160, 250], y in [200, 234]
                    if (x >= 160 && x <= 250 && y >= 200 && y <= 234) {
                        buf[row * KIN_LCD_H_RES + x] = COLOR_WHITE;
                    }
                    if (x >= 160 && x <= 250 && y >= 235 && y <= 238) {
                        buf[row * KIN_LCD_H_RES + x] = COLOR_GRAY_SHADOW;
                    }

                    // 3. Bottom Bar: x in [138, 182], y in [238, 272]
                    if (x >= 138 && x <= 182 && y >= 238 && y <= 272) {
                        buf[row * KIN_LCD_H_RES + x] = COLOR_WHITE;
                    }
                }
            }
        }

        // Draw Typography: "K I N P O D" at y=350, scale=4
        const int text_y = 350;
        const int char_scale = 4;
        draw_glyph_scaled(buf, y0, h, 125, text_y, FONT_K, char_scale, COLOR_WHITE);
        draw_glyph_scaled(buf, y0, h, 153, text_y, FONT_I, char_scale, COLOR_WHITE);
        draw_glyph_scaled(buf, y0, h, 181, text_y, FONT_N, char_scale, COLOR_WHITE);
        draw_glyph_scaled(buf, y0, h, 209, text_y, FONT_P, char_scale, COLOR_AMBER);
        draw_glyph_scaled(buf, y0, h, 237, text_y, FONT_O, char_scale, COLOR_AMBER);
        draw_glyph_scaled(buf, y0, h, 265, text_y, FONT_D, char_scale, COLOR_AMBER);

        esp_lcd_panel_draw_bitmap(s_panel_handle, 0, y0, KIN_LCD_H_RES, y0 + h, buf);
    }

    free(buf);
    ESP_LOGI(TAG, "Official KinPod Boot Splash Rendered Successfully via Waveshare BSP!");
    return ESP_OK;
}
