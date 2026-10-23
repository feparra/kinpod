#include "kin_pairing.h"
#include "board_kinpod_2.06.h"
#include "kin_avatar.h"
#include "kin_audio.h"
#include "kin_ws.h"
#include "kin_provisioning.h"
#include "esp_heap_caps.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_log.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static const char *TAG = "KinPair";

// 410 x 502 RGB565 Frame in Octal PSRAM (411,640 bytes)
static uint16_t *s_pair_frame = NULL;
static bool s_is_active = false;
static bool s_is_fetching = false;

// Typed PIN buffer (4 digits + null terminator)
static char s_pin[8] = {0};
static int s_pin_len = 0;

// Big-Endian RGB565 helper: Swaps bytes so DMA streams High Byte first, Low Byte second
static inline uint16_t rgb565_be(uint8_t r, uint8_t g, uint8_t b) {
    uint16_t val = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    return (uint16_t)((val >> 8) | (val << 8));
}

// 5x7 Font for typography
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
        s_pair_frame[y * KIN_LCD_H_RES + x] = color;
    }
}

static void fill_rect(int x, int y, int w, int h, uint16_t color) {
    for (int j = y; j < y + h; j++) {
        for (int i = x; i < x + w; i++) {
            draw_pixel(i, j, color);
        }
    }
}

static void fill_rounded_rect(int x, int y, int w, int h, int r, uint16_t color) {
    fill_rect(x + r, y, w - 2 * r, h, color);
    fill_rect(x, y + r, r, h - 2 * r, color);
    fill_rect(x + w - r, y + r, r, h - 2 * r, color);
    for (int j = 0; j < r; j++) {
        for (int i = 0; i < r; i++) {
            if ((i - r + 1) * (i - r + 1) + (j - r + 1) * (j - r + 1) <= r * r) {
                draw_pixel(x + r - 1 - i, y + r - 1 - j, color);
                draw_pixel(x + w - r + i, y + r - 1 - j, color);
                draw_pixel(x + r - 1 - i, y + h - r + j, color);
                draw_pixel(x + w - r + i, y + h - r + j, color);
            }
        }
    }
}

static void draw_rounded_rect_border(int x, int y, int w, int h, int r, uint16_t color) {
    for (int i = x + r; i < x + w - r; i++) {
        draw_pixel(i, y, color);
        draw_pixel(i, y + h - 1, color);
    }
    for (int j = y + r; j < y + h - r; j++) {
        draw_pixel(x, j, color);
        draw_pixel(x + w - 1, j, color);
    }
    for (int j = 0; j <= r; j++) {
        for (int i = 0; i <= r; i++) {
            int d = (i - r) * (i - r) + (j - r) * (j - r);
            if (d >= (r - 1) * (r - 1) && d <= r * r) {
                draw_pixel(x + r - i, y + r - j, color);
                draw_pixel(x + w - 1 - r + i, y + r - j, color);
                draw_pixel(x + r - i, y + h - 1 - r + j, color);
                draw_pixel(x + w - 1 - r + i, y + h - 1 - r + j, color);
            }
        }
    }
}

static void fill_circle(int cx, int cy, int r, uint16_t color) {
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if (x * x + y * y <= r * r) {
                draw_pixel(cx + x, cy + y, color);
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

// -----------------------------------------------------------------------------
// AMOLED Rendering: Numeric Keypad (watchOS Slate Theme)
// -----------------------------------------------------------------------------
static void render_pairing_screen(const char *status_msg, uint16_t status_color) {
    if (!s_pair_frame) return;

    // Pitch black background (AMOLED power optimization)
    memset(s_pair_frame, 0x00, KIN_LCD_H_RES * KIN_LCD_V_RES * sizeof(uint16_t));

    const uint16_t col_card        = rgb565_be(24, 24, 26);     // #18181A Deep obsidian
    const uint16_t col_border      = rgb565_be(48, 48, 52);     // #303034 Subtle border
    const uint16_t col_btn_bg      = rgb565_be(36, 36, 40);     // #242428 Keypad button BG
    const uint16_t col_btn_border  = rgb565_be(56, 56, 62);     // #38383E Keypad border
    const uint16_t col_accent_blue = rgb565_be(10, 132, 255);   // #0A84FF Apple Blue
    const uint16_t col_white       = 0xFFFF;                    // Pure White
    const uint16_t col_gray        = rgb565_be(160, 160, 165);  // Gray
    const uint16_t col_dim         = rgb565_be(80, 80, 85);      // Dim gray

    // 1. Header Title & Subtitle
    fill_rounded_rect(185, 12, 40, 4, 2, col_dim); // Pill grabber
    draw_string_centered(26, "PAIR AGENT", col_white, 2);
    draw_string_centered(48, "ENTER 4-DIGIT PIN", col_accent_blue, 2);

    // 2. 4 PIN Display Boxes (y=76, h=56, w=48 each, spacing 14px)
    // Total width = 4 * 48 + 3 * 14 = 192 + 42 = 234 px. Start X = (410 - 234) / 2 = 88.
    const int box_w = 48;
    const int box_h = 56;
    const int box_r = 10;
    const int start_x = 88;
    const int box_y = 76;

    for (int i = 0; i < 4; i++) {
        int bx = start_x + i * (box_w + 14);
        bool is_current = (i == s_pin_len) && !s_is_fetching;
        uint16_t b_border = is_current ? col_accent_blue : col_border;

        fill_rounded_rect(bx, box_y, box_w, box_h, box_r, col_card);
        draw_rounded_rect_border(bx, box_y, box_w, box_h, box_r, b_border);

        if (i < s_pin_len) {
            // Draw entered digit
            char d_str[2] = { s_pin[i], '\0' };
            int dw = string_width(d_str, 3);
            int dx = bx + (box_w - dw) / 2;
            int dy = box_y + (box_h - 21) / 2;
            draw_string(dx, dy, d_str, col_white, 3);
        } else {
            // Draw subtle placeholder dot
            fill_circle(bx + box_w / 2, box_y + box_h / 2, 3, is_current ? col_accent_blue : col_dim);
        }
    }

    // 3. 3x4 Touch Numeric Keypad Grid (y=150 to 410)
    // 3 Columns: col_w = 96, spacing = 26px -> total width = 3 * 96 + 2 * 26 = 340. Start X = 35.
    const int btn_w = 96;
    const int btn_h = 54;
    const int btn_r = 14;
    const int col_spacing = 26;
    const int row_spacing = 14;
    const int kx_start = 35;
    const int ky_start = 148;

    const char *key_labels[4][3] = {
        {"1", "2", "3"},
        {"4", "5", "6"},
        {"7", "8", "9"},
        {"X", "0", "<"}
    };

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 3; col++) {
            int kx = kx_start + col * (btn_w + col_spacing);
            int ky = ky_start + row * (btn_h + row_spacing);

            fill_rounded_rect(kx, ky, btn_w, btn_h, btn_r, col_btn_bg);
            draw_rounded_rect_border(kx, ky, btn_w, btn_h, btn_r, col_btn_border);

            const char *lbl = key_labels[row][col];
            uint16_t text_col = col_white;
            if (row == 3 && col == 0) text_col = rgb565_be(255, 69, 58); // 'X' Red
            if (row == 3 && col == 2) text_col = col_accent_blue;        // '<' Blue

            int lw = string_width(lbl, 3);
            int lx = kx + (btn_w - lw) / 2;
            int ly = ky + (btn_h - 21) / 2;
            draw_string(lx, ly, lbl, text_col, 3);
        }
    }

    // 4. Status Bar at Bottom (y=436, h=48)
    fill_rounded_rect(20, 436, 370, 48, 12, col_card);
    draw_rounded_rect_border(20, 436, 370, 48, 12, col_border);

    if (status_msg) {
        draw_string_centered(452, status_msg, status_color, 2);
    } else {
        draw_string_centered(452, "TAP CODE GIVEN BY AGENT", col_gray, 2);
    }

    // DMA stream to AMOLED
    kin_display_draw_raw_frame((const uint8_t *)s_pair_frame);
}

// -----------------------------------------------------------------------------
// HTTP Relay Resolution Task
// -----------------------------------------------------------------------------
static void pairing_resolve_task(void *pvParameters) {
    char target_pin[8];
    strncpy(target_pin, s_pin, sizeof(target_pin));
    target_pin[sizeof(target_pin) - 1] = '\0';

    ESP_LOGI(TAG, "Resolving 4-digit PIN: %s against Cloud Relay...", target_pin);

    char request_url[256];
    snprintf(request_url, sizeof(request_url), "%s/api/pair/%s", KIN_DEFAULT_RELAY_URL, target_pin);

    esp_http_client_config_t config = {
        .url = request_url,
        .method = HTTP_METHOD_GET,
        .timeout_ms = 10000,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .user_agent = "KinPod/1.0",
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        s_is_fetching = false;
        render_pairing_screen("CLIENT INIT ERROR", rgb565_be(255, 69, 58));
        vTaskDelete(NULL);
        return;
    }

    esp_err_t err = esp_http_client_open(client, 0);
    bool success = false;
    char ws_url_found[256] = {0};
    int status_code = 0;

    if (err == ESP_OK) {
        esp_http_client_fetch_headers(client);
        status_code = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "Relay Response Code: %d", status_code);

        char resp_buf[512] = {0};
        int total_read = 0;
        while (total_read < sizeof(resp_buf) - 1) {
            int r = esp_http_client_read(client, resp_buf + total_read, sizeof(resp_buf) - 1 - total_read);
            if (r <= 0) break;
            total_read += r;
        }
        resp_buf[total_read] = '\0';
        ESP_LOGI(TAG, "Relay Response Body: %s", resp_buf);

        if (status_code == 200) {
            cJSON *root = cJSON_Parse(resp_buf);
            if (root) {
                cJSON *url_item = cJSON_GetObjectItem(root, "url");
                if (url_item && url_item->valuestring && strlen(url_item->valuestring) > 0) {
                    strncpy(ws_url_found, url_item->valuestring, sizeof(ws_url_found) - 1);
                    success = true;
                }
                cJSON_Delete(root);
            }
        }
    } else {
        int tls_err = 0;
        int tls_flags = 0;
        esp_http_client_get_and_clear_last_tls_error(client, &tls_err, &tls_flags);
        ESP_LOGE(TAG, "HTTP connection to relay failed: %s (%d) | TLS err=0x%x, flags=0x%x",
                 esp_err_to_name(err), err, tls_err, tls_flags);
    }

    esp_http_client_cleanup(client);

    if (success) {
        ESP_LOGI(TAG, ">>> SUCCESS! Paired with WebSocket: %s <<<", ws_url_found);
        render_pairing_screen("PAIRED! CONNECTING...", rgb565_be(48, 209, 88));
        kin_audio_play_chime();

        // 1. Save new WebSocket URL to NVS
        kin_provisioning_save_config(NULL, NULL, ws_url_found);

        // 2. Connect WebSocket
        kin_ws_init(ws_url_found);

        // 3. Delay to show success badge, then restore avatar
        vTaskDelay(pdMS_TO_TICKS(1400));
        kin_pairing_hide();
    } else {
        ESP_LOGW(TAG, "Pairing failed for PIN: %s (status=%d, err=%s)", target_pin, status_code, esp_err_to_name(err));
        kin_audio_haptic_buzz(120);

        if (err != ESP_OK) {
            render_pairing_screen("NETWORK / TLS ERROR", rgb565_be(255, 69, 58));
        } else if (status_code == 404) {
            render_pairing_screen("INVALID OR EXPIRED PIN", rgb565_be(255, 69, 58));
        } else {
            render_pairing_screen("PAIRING FAILED", rgb565_be(255, 69, 58));
        }

        vTaskDelay(pdMS_TO_TICKS(1800));

        s_pin_len = 0;
        memset(s_pin, 0, sizeof(s_pin));
        s_is_fetching = false;
        render_pairing_screen(NULL, 0);
    }

    vTaskDelete(NULL);
}

// -----------------------------------------------------------------------------
// Public Lifecycle Methods
// -----------------------------------------------------------------------------
esp_err_t kin_pairing_init(void) {
    if (!s_pair_frame) {
        s_pair_frame = (uint16_t *)heap_caps_malloc(KIN_LCD_H_RES * KIN_LCD_V_RES * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
        if (!s_pair_frame) {
            ESP_LOGE(TAG, "Failed to allocate Pairing screen in PSRAM");
            return ESP_ERR_NO_MEM;
        }
    }
    ESP_LOGI(TAG, "KinPod Pairing Subsystem initialized [PSRAM: 411 KB]");
    return ESP_OK;
}

bool kin_pairing_is_active(void) {
    return s_is_active;
}

esp_err_t kin_pairing_show(void) {
    if (!s_pair_frame) kin_pairing_init();
    s_is_active = true;
    s_is_fetching = false;
    s_pin_len = 0;
    memset(s_pin, 0, sizeof(s_pin));

    ESP_LOGI(TAG, "Opening 4-Digit Pairing Keypad screen");
    render_pairing_screen(NULL, 0);
    return ESP_OK;
}

esp_err_t kin_pairing_hide(void) {
    s_is_active = false;
    s_is_fetching = false;
    ESP_LOGI(TAG, "Closing Pairing screen -> Restoring active avatar");
    return kin_avatar_set_active(kin_avatar_get_active());
}

bool kin_pairing_handle_touch(uint16_t x, uint16_t y) {
    if (!s_is_active || s_is_fetching) return true;

    // Check hit inside 3x4 numeric keypad grid
    // kx_start = 35, ky_start = 148, btn_w = 96, btn_h = 54, col_spacing = 26, row_spacing = 14
    const int btn_w = 96;
    const int btn_h = 54;
    const int col_spacing = 26;
    const int row_spacing = 14;
    const int kx_start = 35;
    const int ky_start = 148;

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 3; col++) {
            int kx = kx_start + col * (btn_w + col_spacing);
            int ky = ky_start + row * (btn_h + row_spacing);

            if (x >= kx && x < kx + btn_w && y >= ky && y < ky + btn_h) {
                // Button tapped!
                if (row == 3 && col == 0) {
                    // 'X' Cancel
                    ESP_LOGI(TAG, "Pairing Cancelled by user");
                    kin_audio_haptic_buzz(25);
                    kin_pairing_hide();
                    return true;
                } else if (row == 3 && col == 2) {
                    // '<' Backspace
                    if (s_pin_len > 0) {
                        s_pin[--s_pin_len] = '\0';
                        kin_audio_haptic_buzz(20);
                        render_pairing_screen(NULL, 0);
                    }
                    return true;
                } else {
                    // Digit key (0 to 9)
                    char digit = '0';
                    if (row < 3) {
                        digit = '1' + (row * 3 + col);
                    } else if (row == 3 && col == 1) {
                        digit = '0';
                    }

                    if (s_pin_len < 4) {
                        s_pin[s_pin_len++] = digit;
                        s_pin[s_pin_len] = '\0';
                        kin_audio_haptic_buzz(25);
                        ESP_LOGI(TAG, "Entered Digit: %c (PIN now: %s)", digit, s_pin);

                        if (s_pin_len == 4) {
                            // 4 digits entered! Start cloud resolution immediately!
                            s_is_fetching = true;
                            render_pairing_screen("LINKING TO CLOUD...", rgb565_be(10, 132, 255));
                            xTaskCreatePinnedToCore(pairing_resolve_task, "kin_pair_tsk", 8192, NULL, 5, NULL, 1);
                        } else {
                            render_pairing_screen(NULL, 0);
                        }
                    }
                    return true;
                }
            }
        }
    }

    return true; // Consume touch while pairing is open
}
