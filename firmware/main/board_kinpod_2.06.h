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
 * @brief Renders the official true-black KinPod boot splash logo on the AMOLED panel.
 */
esp_err_t kin_display_draw_splash(void);

/**
 * @brief Adjusts display brightness (0-100%).
 */
esp_err_t kin_display_set_brightness(uint8_t pct);

/**
 * @brief Clears the entire AMOLED screen to pure black (0x0000).
 */
esp_err_t kin_display_clear(void);

#ifdef __cplusplus
}
#endif
