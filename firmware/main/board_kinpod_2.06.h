#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "esp_lcd_panel_ops.h"

#ifdef __cplusplus
extern "C" {
#endif

// Physical QSPI Pinout for Waveshare ESP32-S3-Touch-AMOLED-2.06
#define KIN_LCD_HOST        SPI2_HOST
#define KIN_LCD_PCLK_HZ     (40 * 1000 * 1000)
#define KIN_LCD_SCLK        11
#define KIN_LCD_D0          4
#define KIN_LCD_D1          5
#define KIN_LCD_D2          6
#define KIN_LCD_D3          7
#define KIN_LCD_CS          12
#define KIN_LCD_RST         8
#define KIN_LCD_TE          13

// AMOLED Native Dimensions
#define KIN_LCD_H_RES       410
#define KIN_LCD_V_RES       502

/**
 * @brief Initializes the QSPI bus and the CO5300 AMOLED panel controller.
 */
esp_err_t kin_display_init(void);

/**
 * @brief Streams an arbitrary 410x502 RGB565 frame to the AMOLED display via DMA.
 */
esp_err_t kin_display_draw_raw_frame(const uint8_t *raw_frame);

/**
 * @brief Draws a rectangular sub-region to the AMOLED display via DMA.
 */
esp_err_t kin_display_draw_region(int x_start, int y_start, int x_end, int y_end, const uint16_t *pixels);

/**
 * @brief Streams a w x h RGB565 sprite directly to a sub-window of the AMOLED panel via DMA.
 */
esp_err_t kin_display_draw_sprite(int x, int y, int w, int h, const uint8_t *raw_sprite);

/**
 * @brief Renders the official true-black KinPod boot splash logo on the AMOLED panel.
 */
esp_err_t kin_display_draw_splash(void);

/**
 * @brief Adjusts display brightness (0-100%).
 */
esp_err_t kin_display_set_brightness(uint8_t pct);

/**
 * @brief Powers display panel on or off (Sleep mode: true = on, false = deep sleep 0 mA).
 */
esp_err_t kin_display_set_power(bool on);

/**
 * @brief Returns true if AMOLED display panel is currently powered on.
 */
bool kin_display_is_on(void);

/**
 * @brief Clears the entire AMOLED screen to pure black (0x0000).
 */
esp_err_t kin_display_clear(void);

/**
 * @brief Initializes the capacitive touch controller (CST9220 / FT5x06) via BSP.
 */
esp_err_t kin_touch_init(void);

/**
 * @brief Reads current touch coordinate if screen is being touched.
 * @return true if touched, false otherwise.
 */
bool kin_touch_get_point(uint16_t *x, uint16_t *y);

#ifdef __cplusplus
}
#endif
