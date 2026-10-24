#include "kin_control_center.h"
#include "board_kinpod_2.06.h"
#include "kin_avatar.h"
#include "kin_audio.h"
#include "kin_power.h"
#include "kin_wifi.h"
#include "kin_ws.h"
#include "kin_pairing.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static const char *TAG = "KinCC";

// Buffer for full 410 x 502 RGB565 frame (411,640 bytes in Octal PSRAM)
static uint16_t *s_cc_frame = NULL;
static bool s_is_open = false;
static kin_cc_wifi_setup_cb_t s_wifi_cb = NULL;

// Big-Endian RGB565 helper: Swaps bytes so DMA streams High Byte first, Low Byte second
static inline uint16_t rgb565_be(uint8_t r, uint8_t g, uint8_t b) {
    uint16_t val = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    return (uint16_t)((val >> 8) | (val << 8));
}

static inline uint16_t hex_to_rgb565_be(uint32_t hex) {
    uint8_t r = (hex >> 16) & 0xFF;
    uint8_t g = (hex >> 8) & 0xFF;
    uint8_t b = hex & 0xFF;
    return rgb565_be(r, g, b);
}

// 5x7 Font for crisp Apple watchOS typography
static const uint8_t s_font5x7[59][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // ' ' (32)
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // '!'
    {0x00, 0x07, 0x00, 0x07, 0x00}, // '"'
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // '#'
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // '$'
    {0x23, 0x13, 0x08, 0x64, 0x62}, // '%'
    {0x36, 0x49, 0x55, 0x22, 0x50}, // '&'
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '''
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // '('
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // ')'
    {0x08, 0x2A, 0x1C, 0x2A, 0x08}, // '*'
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // '+'
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ','
    {0x08, 0x08, 0x08, 0x08, 0x08}, // '-'
    {0x00, 0x60, 0x60, 0x00, 0x00}, // '.'
    {0x20, 0x10, 0x08, 0x04, 0x02}, // '/'
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // '0'
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // '1'
    {0x42, 0x61, 0x51, 0x49, 0x46}, // '2'
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // '3'
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // '4'
    {0x27, 0x45, 0x45, 0x45, 0x39}, // '5'
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // '6'
    {0x01, 0x71, 0x09, 0x05, 0x03}, // '7'
    {0x36, 0x49, 0x49, 0x49, 0x36}, // '8'
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // '9'
    {0x00, 0x36, 0x36, 0x00, 0x00}, // ':'
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ';'
    {0x00, 0x08, 0x14, 0x22, 0x41}, // '<'
    {0x14, 0x14, 0x14, 0x14, 0x14}, // '='
    {0x41, 0x22, 0x14, 0x08, 0x00}, // '>'
    {0x02, 0x01, 0x51, 0x09, 0x06}, // '?'
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // '@'
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // 'A'
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // 'B'
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // 'C'
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // 'D'
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // 'E'
    {0x7F, 0x09, 0x09, 0x01, 0x01}, // 'F'
    {0x3E, 0x41, 0x41, 0x51, 0x32}, // 'G'
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // 'H'
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // 'I'
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // 'J'
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // 'K'
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // 'L'
    {0x7F, 0x02, 0x04, 0x02, 0x7F}, // 'M'
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // 'N'
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 'O'
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // 'P'
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // 'Q'
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // 'R'
    {0x46, 0x49, 0x49, 0x49, 0x31}, // 'S'
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // 'T'
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // 'U'
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // 'V'
    {0x7F, 0x20, 0x18, 0x20, 0x7F}, // 'W'
    {0x63, 0x14, 0x08, 0x14, 0x63}, // 'X'
    {0x03, 0x04, 0x78, 0x04, 0x03}, // 'Y'
    {0x61, 0x51, 0x49, 0x45, 0x43}, // 'Z'
};

static inline void draw_pixel(int x, int y, uint16_t color) {
    if (x >= 0 && x < KIN_LCD_H_RES && y >= 0 && y < KIN_LCD_V_RES) {
        s_cc_frame[y * KIN_LCD_H_RES + x] = color;
    }
}

static void fill_rect(int x, int y, int w, int h, uint16_t color) {
    for (int j = y; j < y + h; j++) {
        if (j < 0 || j >= KIN_LCD_V_RES) continue;
        int row_offset = j * KIN_LCD_H_RES;
        for (int i = x; i < x + w; i++) {
            if (i < 0 || i >= KIN_LCD_H_RES) continue;
            s_cc_frame[row_offset + i] = color;
        }
    }
}

static void fill_rounded_rect(int x, int y, int w, int h, int r, uint16_t color) {
    fill_rect(x + r, y, w - 2 * r, h, color);
    fill_rect(x, y + r, r, h - 2 * r, color);
    fill_rect(x + w - r, y + r, r, h - 2 * r, color);

    for (int dy = 0; dy < r; dy++) {
        for (int dx = 0; dx < r; dx++) {
            if ((r - dx) * (r - dx) + (r - dy) * (r - dy) <= r * r) {
                draw_pixel(x + dx, y + dy, color);
                draw_pixel(x + w - 1 - dx, y + dy, color);
                draw_pixel(x + dx, y + h - 1 - dy, color);
                draw_pixel(x + w - 1 - dx, y + h - 1 - dy, color);
            }
        }
    }
}

static void draw_rounded_rect_border(int x, int y, int w, int h, int r, uint16_t border_col) {
    fill_rect(x + r, y, w - 2 * r, 2, border_col);
    fill_rect(x + r, y + h - 2, w - 2 * r, 2, border_col);
    fill_rect(x, y + r, 2, h - 2 * r, border_col);
    fill_rect(x + w - 2, y + r, 2, h - 2 * r, border_col);
}

static void fill_circle(int cx, int cy, int r, uint16_t color) {
    for (int dy = -r; dy <= r; dy++) {
        for (int dx = -r; dx <= r; dx++) {
            if (dx * dx + dy * dy <= r * r) {
                draw_pixel(cx + dx, cy + dy, color);
            }
        }
    }
}

static void draw_string(int x, int y, const char *str, uint16_t color, int scale) {
    while (*str) {
        char c = *str++;
        if (c >= 'a' && c <= 'z') c -= 32;
        if (c >= ' ' && c <= 'Z') {
            const uint8_t *glyph = s_font5x7[c - ' '];
            for (int col = 0; col < 5; col++) {
                uint8_t line = glyph[col];
                for (int row = 0; row < 7; row++) {
                    if (line & (1 << row)) {
                        fill_rect(x + col * scale, y + row * scale, scale, scale, color);
                    }
                }
            }
            x += (5 + 1) * scale;
        } else {
            x += 4 * scale;
        }
    }
}

static int string_width(const char *str, int scale) {
    int len = 0;
    while (*str++) len++;
    if (len == 0) return 0;
    return len * 6 * scale - scale;
}

static void draw_string_centered(int y, const char *str, uint16_t color, int scale) {
    int w = string_width(str, scale);
    int x = (KIN_LCD_H_RES - w) / 2;
    if (x < 10) x = 10;
    draw_string(x, y, str, color, scale);
}

esp_err_t kin_control_center_init(kin_cc_wifi_setup_cb_t wifi_setup_cb) {
    s_wifi_cb = wifi_setup_cb;

    if (!s_cc_frame) {
        s_cc_frame = (uint16_t *)heap_caps_malloc(KIN_LCD_H_RES * KIN_LCD_V_RES * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
        if (!s_cc_frame) {
            ESP_LOGE(TAG, "Failed to allocate Control Center frame in PSRAM");
            return ESP_ERR_NO_MEM;
        }
    }

    ESP_LOGI(TAG, "KinPod Apple-style Control Center initialized [PSRAM: 411 KB]");
    return ESP_OK;
}

bool kin_control_center_is_open(void) {
    return s_is_open;
}

esp_err_t kin_control_center_show(void) {
    if (!s_cc_frame) return ESP_ERR_INVALID_STATE;
    s_is_open = true;

    // Live telemetry query
    kin_power_telemetry_t telem = {0};
    uint16_t vbat = 3950;
    uint8_t soc = 85;
    bool charging = false;
    if (kin_power_get_telemetry(&telem) == ESP_OK) {
        vbat = telem.vbat_mv;
        soc = telem.battery_pct;
        charging = telem.is_charging;
    }

    const kin_avatar_profile_t *active_kin = kin_avatar_get_profile(kin_avatar_get_active());
    bool wifi_on = kin_wifi_is_connected();
    int8_t rssi = kin_wifi_get_rssi();
    bool ws_on = kin_ws_is_connected();

    // 1. Clear entire AMOLED screen to pure pitch black (0x0000)
    memset(s_cc_frame, 0x00, KIN_LCD_H_RES * KIN_LCD_V_RES * sizeof(uint16_t));

    // True Apple watchOS Dark Obsidian Color Palette (Big-Endian byte-swapped for AMOLED DMA)
    const uint16_t col_card        = rgb565_be(24, 24, 26);     // #18181A Deep obsidian slate
    const uint16_t col_border      = rgb565_be(48, 48, 52);     // #303034 Subtle glass border
    const uint16_t col_accent_blue = rgb565_be(10, 132, 255);   // #0A84FF Apple System Blue (Charging)
    const uint16_t col_green       = rgb565_be(48, 209, 88);    // #30D158 Apple System Green (>20%)
    const uint16_t col_yellow      = rgb565_be(255, 204, 0);    // #FFCC00 Apple System Yellow (11-20%)
    const uint16_t col_red         = rgb565_be(255, 69, 58);    // #FF453A Apple System Red (<=10%)
    const uint16_t col_white       = 0xFFFF;                    // #FFFFFF Pure Crisp White
    const uint16_t col_gray        = rgb565_be(170, 170, 175);  // #AAAAAF High-contrast readable gray
    const uint16_t col_dim         = rgb565_be(75, 75, 80);     // #4B4B50 Grabber & inactive badge
    const uint16_t col_btn_bg      = rgb565_be(38, 38, 42);     // #26262A Dark button background

    // 2. Top Pill Grabber (watchOS style pull-down/pull-up indicator)
    fill_rounded_rect(185, 12, 40, 4, 2, col_dim);

    // 3. Header Bar: Center title safely away from rounded glass corners
    draw_string_centered(26, "CONTROL CENTER", col_white, 2);

    // Dynamic battery color thresholds: Blue (Charging) | Green (>20%) | Yellow (11-20%) | Red (<=10%)
    uint16_t col_battery;
    if (charging) {
        col_battery = col_accent_blue;
    } else if (soc <= 10) {
        col_battery = col_red;
    } else if (soc <= 20) {
        col_battery = col_yellow;
    } else {
        col_battery = col_green;
    }

    // Draw numerical battery percentage text next to pill (e.g., "85%" or "+94%")
    char soc_str[10];
    if (charging) {
        snprintf(soc_str, sizeof(soc_str), "+%u%%", soc);
    } else {
        snprintf(soc_str, sizeof(soc_str), "%u%%", soc);
    }
    draw_string(292, 28, soc_str, charging ? col_accent_blue : (soc <= 20 ? col_battery : col_gray), 1);

    // Battery pill at top-right (x=336, y=24, w=48, h=18)
    fill_rounded_rect(336, 24, 48, 18, 4, col_card);
    draw_rounded_rect_border(336, 24, 48, 18, 4, (soc <= 10 && !charging) ? col_red : col_border);
    // Positive terminal nipple on right side: x=384, y=28, w=3, h=10
    fill_rounded_rect(384, 28, 3, 10, 1, col_border);

    // Inner battery fill (max 40px)
    int fill_w = (soc * 40) / 100;
    if (fill_w > 40) fill_w = 40;
    if (fill_w < 3 && soc > 0) fill_w = 3;
    fill_rounded_rect(340, 27, fill_w, 12, 2, col_battery);

    // If charging, draw small white lightning bolt icon over battery pill
    if (charging) {
        fill_rect(358, 27, 2, 4, col_white);
        fill_rect(356, 30, 5, 2, col_white);
        fill_rect(357, 32, 2, 4, col_white);
    }

    // -------------------------------------------------------------
    // CARD 1: WI-FI & HOTSPOT (y=56, h=136, x=20, w=370)
    // -------------------------------------------------------------
    fill_rounded_rect(20, 56, 370, 136, 14, col_card);
    draw_rounded_rect_border(20, 56, 370, 136, 14, col_border);

    // Glowing Wi-Fi badge
    fill_circle(48, 86, 14, wifi_on ? col_accent_blue : col_dim);
    fill_circle(48, 86, 9, wifi_on ? col_white : col_gray);

    draw_string(74, 70, "WI-FI NETWORK", col_accent_blue, 2);
    if (wifi_on) {
        char current_ssid[64] = {0};
        if (kin_wifi_get_connected_ssid(current_ssid, sizeof(current_ssid)) == ESP_OK && strlen(current_ssid) > 0) {
            draw_string(74, 92, current_ssid, col_white, 2);
        } else {
            draw_string(74, 92, "CONNECTED", col_white, 2);
        }
        char rssi_str[40];
        snprintf(rssi_str, sizeof(rssi_str), "ONLINE : %d DBM", rssi);
        draw_string(74, 114, rssi_str, col_gray, 2);
    } else {
        draw_string(74, 92, "DISCONNECTED", col_gray, 2);
        draw_string(74, 114, "NO LINKED NETWORK", col_dim, 2);
    }

    // Interactive Button: "CONFIGURE WI-FI >" (Touch Target: y=138 to 182)
    fill_rounded_rect(34, 140, 342, 38, 10, col_accent_blue);
    draw_string_centered(152, "CONFIGURE WI-FI >", col_white, 2);

    // -------------------------------------------------------------
    // CARD 2: ACTIVE AGENT & KINBRIDGE HUB (y=204, h=108, x=20, w=370)
    // -------------------------------------------------------------
    fill_rounded_rect(20, 204, 370, 108, 14, col_card);
    draw_rounded_rect_border(20, 204, 370, 108, 14, col_border);

    // Active Kin Character Accent Bar on left edge
    uint16_t kin_accent = hex_to_rgb565_be(active_kin->accent_hex);
    fill_rounded_rect(28, 214, 8, 88, 4, kin_accent);

    draw_string(48, 218, "ACTIVE AGENT", col_gray, 2);
    draw_string(48, 240, active_kin->name, col_white, 2);

    // Interactive Button: "LINK AGENT (4-DIGIT PIN) >" (Touch Target: y=266 to 306)
    fill_rounded_rect(44, 268, 332, 36, 8, col_btn_bg);
    draw_rounded_rect_border(44, 268, 332, 36, 8, col_border);
    fill_circle(58, 286, 5, ws_on ? col_green : col_accent_blue);
    draw_string(70, 278, ws_on ? "LINKED (TAP TO RE-PAIR)" : "LINK AGENT (PIN) >", ws_on ? col_green : col_accent_blue, 2);

    // -------------------------------------------------------------
    // CARD 3: SYSTEM TELEMETRY & HARDWARE (y=324, h=96, x=20, w=370)
    // -------------------------------------------------------------
    fill_rounded_rect(20, 324, 370, 96, 14, col_card);
    draw_rounded_rect_border(20, 324, 370, 96, 14, col_border);

    draw_string(36, 336, "SYSTEM TELEMETRY", col_gray, 2);

    char bat_str[40];
    snprintf(bat_str, sizeof(bat_str), "BATTERY: %u%% (%u MV)", soc, vbat);
    draw_string(36, 358, bat_str, col_white, 2);

    draw_string(36, 380, "ESP32-S3 / 8MB PSRAM", col_gray, 2);

    // -------------------------------------------------------------
    // BOTTOM CLOSE BUTTON (y=432, h=52, x=20, w=370)
    // -------------------------------------------------------------
    fill_rounded_rect(20, 432, 370, 52, 14, col_btn_bg);
    draw_rounded_rect_border(20, 432, 370, 52, 14, col_border);
    draw_string_centered(448, "^ SWIPE OR TAP TO CLOSE ^", col_white, 2);

    // Stream full 410x502 frame to AMOLED panel
    ESP_LOGI(TAG, "Control Center rendered -> Streaming to AMOLED via DMA");
    return kin_display_draw_raw_frame((const uint8_t *)s_cc_frame);
}

esp_err_t kin_control_center_hide(void) {
    s_is_open = false;
    ESP_LOGI(TAG, "Closing Control Center -> Restoring active avatar");
    return kin_avatar_set_active(kin_avatar_get_active());
}

bool kin_control_center_handle_touch(uint16_t x, uint16_t y) {
    if (!s_is_open) return false;

    ESP_LOGI(TAG, "Control Center Touch at (%u, %u)", x, y);

    // 1. Tap on "CONFIGURAR WI-FI >" Button (x=30 to 380, y=136 to 184)
    if (x >= 30 && x <= 380 && y >= 136 && y <= 184) {
        ESP_LOGI(TAG, ">>> TAP: CONFIGURAR WI-FI TRIGGERED <<<");
        kin_audio_haptic_buzz(40);
        if (s_wifi_cb) {
            s_wifi_cb();
        }
        return true;
    }

    // 2. Tap on "LINK AGENT (PIN) >" Button (x=36 to 384, y=262 to 310)
    if (x >= 36 && x <= 384 && y >= 262 && y <= 310) {
        ESP_LOGI(TAG, ">>> TAP: LINK AGENT (4-DIGIT PIN) TRIGGERED <<<");
        kin_audio_haptic_buzz(35);
        s_is_open = false;
        kin_pairing_show();
        return true;
    }

    // 3. Tap on Bottom Close Button (x=16 to 394, y=424 to 490)
    if (x >= 16 && x <= 394 && y >= 424 && y <= 490) {
        ESP_LOGI(TAG, ">>> TAP: CLOSE CONTROL CENTER <<<");
        kin_audio_haptic_buzz(25);
        kin_control_center_hide();
        return true;
    }

    return true; // Touches while CC is open are consumed
}
