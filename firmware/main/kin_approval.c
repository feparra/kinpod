#include "kin_approval.h"
#include "board_kinpod_2.06.h"
#include "kin_avatar.h"
#include "kin_audio.h"
#include "kin_ws.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

static const char *TAG = "KinApproval";

// 410 x 502 RGB565 Frame in Octal PSRAM (411,640 bytes)
static uint16_t *s_approval_frame = NULL;
static bool s_is_active = false;
static TaskHandle_t s_timeout_task_handle = NULL;

static char s_action_id[64] = {0};
static char s_agent[32]     = {0};
static char s_action[64]    = {0};
static char s_details[192]  = {0};
static char s_risk[16]      = {0};

// Big-Endian RGB565 helper for DMA
static inline uint16_t rgb565_be(uint8_t r, uint8_t g, uint8_t b) {
    uint16_t val = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    return (uint16_t)((val >> 8) | (val << 8));
}

// 5x7 Typography Font Table
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
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // 'F'
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // 'G'
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // 'H'
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // 'I'
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // 'J'
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // 'K'
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // 'L'
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // 'M'
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
    {0x07, 0x08, 0x70, 0x08, 0x07}, // 'Y'
    {0x61, 0x51, 0x49, 0x45, 0x43}  // 'Z'
};

static inline void draw_pixel(int x, int y, uint16_t color) {
    if (x >= 0 && x < KIN_LCD_H_RES && y >= 0 && y < KIN_LCD_V_RES) {
        s_approval_frame[y * KIN_LCD_H_RES + x] = color;
    }
}

static void fill_rect(int x, int y, int w, int h, uint16_t color) {
    for (int j = y; j < y + h; j++) {
        if (j < 0 || j >= KIN_LCD_V_RES) continue;
        int row_offset = j * KIN_LCD_H_RES;
        for (int i = x; i < x + w; i++) {
            if (i < 0 || i >= KIN_LCD_H_RES) continue;
            s_approval_frame[row_offset + i] = color;
        }
    }
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

static void fill_rounded_rect(int x, int y, int w, int h, int r, uint16_t color) {
    if (r <= 0) {
        fill_rect(x, y, w, h, color);
        return;
    }
    fill_rect(x + r, y, w - 2 * r, h, color);
    fill_rect(x, y + r, r, h - 2 * r, color);
    fill_rect(x + w - r, y + r, r, h - 2 * r, color);
    fill_circle(x + r, y + r, r, color);
    fill_circle(x + w - r - 1, y + r, r, color);
    fill_circle(x + r, y + h - r - 1, r, color);
    fill_circle(x + w - r - 1, y + h - r - 1, r, color);
}

static void draw_rounded_rect_border(int x, int y, int w, int h, int r, uint16_t color) {
    for (int i = x + r; i < x + w - r; i++) {
        draw_pixel(i, y, color);
        draw_pixel(i, y + 1, color);
        draw_pixel(i, y + h - 1, color);
        draw_pixel(i, y + h - 2, color);
    }
    for (int j = y + r; j < y + h - r; j++) {
        draw_pixel(x, j, color);
        draw_pixel(x + 1, j, color);
        draw_pixel(x + w - 1, j, color);
        draw_pixel(x + w - 2, j, color);
    }
}

static void draw_char(int x, int y, char c, uint16_t color, int scale) {
    char uc = toupper((unsigned char)c);
    if (uc < 32 || uc > 'Z') uc = ' ';
    const uint8_t *glyph = s_font5x7[uc - 32];
    for (int col = 0; col < 5; col++) {
        uint8_t line = glyph[col];
        for (int row = 0; row < 7; row++) {
            if (line & (1 << row)) {
                fill_rect(x + col * scale, y + row * scale, scale, scale, color);
            }
        }
    }
}

static int string_width(const char *str, int scale) {
    if (!str) return 0;
    int len = strlen(str);
    return len > 0 ? (len * 6 - 1) * scale : 0;
}

static void draw_string(int x, int y, const char *str, uint16_t color, int scale) {
    if (!str) return;
    while (*str) {
        draw_char(x, y, *str, color, scale);
        x += 6 * scale;
        str++;
    }
}

static void draw_string_centered(int y, const char *str, uint16_t color, int scale) {
    int w = string_width(str, scale);
    int x = (KIN_LCD_H_RES - w) / 2;
    draw_string(x, y, str, color, scale);
}

static void render_approval_modal(void) {
    if (!s_approval_frame) return;

    // 1. True Black AMOLED Background
    memset(s_approval_frame, 0x00, KIN_LCD_H_RES * KIN_LCD_V_RES * sizeof(uint16_t));

    const uint16_t col_card        = rgb565_be(24, 24, 28);     // #18181C Deep obsidian slate
    const uint16_t col_border      = rgb565_be(52, 52, 60);     // #34343C Subtle slate border
    const uint16_t col_white       = 0xFFFF;                    // #FFFFFF Pure white
    const uint16_t col_gray        = rgb565_be(160, 160, 168);  // #A0A0A8 Light text gray
    const uint16_t col_dim         = rgb565_be(80, 80, 88);      // #505058 Dim gray
    const uint16_t col_red         = rgb565_be(239, 68, 68);    // #EF4444 Crimson
    const uint16_t col_amber       = rgb565_be(245, 158, 11);   // #F59E0B Amber
    const uint16_t col_green       = rgb565_be(16, 185, 129);   // #10B981 Emerald
    const uint16_t col_blue        = rgb565_be(56, 189, 248);   // #38BDF8 Apple Cyan

    // Risk color selection
    uint16_t risk_color = col_amber;
    if (strcasecmp(s_risk, "CRITICAL") == 0 || strcasecmp(s_risk, "HIGH") == 0) {
        risk_color = col_red;
    } else if (strcasecmp(s_risk, "LOW") == 0) {
        risk_color = col_blue;
    }

    // Top Pill Grabber
    fill_rounded_rect(185, 10, 40, 4, 2, col_dim);

    // 2. Risk Badge (y=26, h=34, w=280)
    int badge_w = 280;
    int badge_h = 34;
    int badge_x = (KIN_LCD_H_RES - badge_w) / 2;
    int badge_y = 24;

    fill_rounded_rect(badge_x, badge_y, badge_w, badge_h, 8, rgb565_be(32, 20, 24));
    draw_rounded_rect_border(badge_x, badge_y, badge_w, badge_h, 8, risk_color);

    char risk_badge_text[32];
    snprintf(risk_badge_text, sizeof(risk_badge_text), "! %s APPROVAL", s_risk);
    for (int i = 0; risk_badge_text[i]; i++) risk_badge_text[i] = toupper((unsigned char)risk_badge_text[i]);
    int rw = string_width(risk_badge_text, 2);
    draw_string(badge_x + (badge_w - rw) / 2, badge_y + 9, risk_badge_text, risk_color, 2);

    // 3. Agent Tagline
    char agent_tag[48];
    snprintf(agent_tag, sizeof(agent_tag), "REQUEST FROM: %s", s_agent);
    for (int i = 0; agent_tag[i]; i++) agent_tag[i] = toupper((unsigned char)agent_tag[i]);
    draw_string_centered(68, agent_tag, col_blue, 2);

    // 4. Main Action & Details Obsidian Card (x=24, y=96, w=362, h=236)
    int card_x = 24;
    int card_y = 96;
    int card_w = 362;
    int card_h = 236;
    fill_rounded_rect(card_x, card_y, card_w, card_h, 16, col_card);
    draw_rounded_rect_border(card_x, card_y, card_w, card_h, 16, col_border);

    // Action Name
    char action_upper[64];
    snprintf(action_upper, sizeof(action_upper), "%s", s_action);
    for (int i = 0; action_upper[i]; i++) action_upper[i] = toupper((unsigned char)action_upper[i]);
    draw_string_centered(card_y + 16, action_upper, col_white, 2);

    // Divider Line
    fill_rect(card_x + 16, card_y + 44, card_w - 32, 2, col_border);

    // Details Word-Wrap (up to 5 lines, max 26 chars per line, breaking on word boundaries)
    const char *p = s_details;
    int line_y = card_y + 58;
    int max_chars_per_line = 26;
    int line_count = 0;

    while (*p && line_count < 5) {
        while (*p == ' ') p++;
        if (!*p) break;

        int len = 0;
        int last_space = -1;
        while (p[len] && len < max_chars_per_line) {
            if (p[len] == ' ') last_space = len;
            len++;
        }

        int take = len;
        if (p[len] && p[len] != ' ' && last_space > 0) {
            take = last_space;
        }

        char line_buf[32] = {0};
        memcpy(line_buf, p, take);
        line_buf[take] = '\0';
        p += take;

        // Draw left-aligned inside card
        draw_string(card_x + 20, line_y, line_buf, col_gray, 2);
        line_y += 24;
        line_count++;
    }

    // Subtitle Note inside card (Scale 2 for high visibility on chest AMOLED)
    draw_string_centered(card_y + card_h - 28, "REQUIRES HUMAN CONFIRMATION", col_gray, 2);

    // 5. Tactile Decision Buttons
    // REJECT BUTTON (Left: x=24 to 194, w=170, h=78)
    int btn_y = 352;
    int btn_w = 170;
    int btn_h = 78;

    fill_rounded_rect(24, btn_y, btn_w, btn_h, 18, rgb565_be(48, 16, 20));
    draw_rounded_rect_border(24, btn_y, btn_w, btn_h, 18, col_red);
    int rej_w = string_width("X REJECT", 2);
    draw_string(24 + (btn_w - rej_w) / 2, btn_y + 30, "X REJECT", col_red, 2);

    // APPROVE BUTTON (Right: x=216 to 386, w=170, h=78)
    fill_rounded_rect(216, btn_y, btn_w, btn_h, 18, rgb565_be(16, 48, 28));
    draw_rounded_rect_border(216, btn_y, btn_w, btn_h, 18, col_green);
    int app_w = string_width("OK APPROVE", 2);
    draw_string(216 + (btn_w - app_w) / 2, btn_y + 30, "OK APPROVE", col_green, 2);

    // 6. Footer Timeout Hint
    draw_string_centered(452, "TAP CHEST TO DECIDE (TIMEOUT 60S)", col_dim, 1);

    // Push complete 410x502 frame via DMA
    kin_display_draw_raw_frame((const uint8_t *)s_approval_frame);
}

static void approval_timeout_task(void *pvParameters) {
    ESP_LOGI(TAG, "Approval countdown started (60s)...");
    for (int i = 0; i < 60; i++) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        if (!s_is_active) {
            vTaskDelete(NULL);
            return;
        }
    }

    if (s_is_active) {
        ESP_LOGW(TAG, "Approval timed out! Auto-rejecting action '%s'", s_action_id);
        kin_audio_play_tone(330, 100);
        kin_ws_send_approval_response(s_action_id, false);
        kin_approval_hide();
    }
    vTaskDelete(NULL);
}

esp_err_t kin_approval_init(void) {
    if (!s_approval_frame) {
        s_approval_frame = (uint16_t *)heap_caps_malloc(KIN_LCD_H_RES * KIN_LCD_V_RES * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
        if (!s_approval_frame) {
            ESP_LOGE(TAG, "Failed to allocate 410x502 approval frame in PSRAM!");
            return ESP_ERR_NO_MEM;
        }
    }
    ESP_LOGI(TAG, "Human-In-The-Loop Approval Subsystem initialized [OK]");
    return ESP_OK;
}

bool kin_approval_is_active(void) {
    return s_is_active;
}

esp_err_t kin_approval_show(const char *action_id, const char *agent, const char *action, const char *details, const char *risk) {
    if (!s_approval_frame) {
        kin_approval_init();
    }

    snprintf(s_action_id, sizeof(s_action_id), "%s", action_id ? action_id : "");
    snprintf(s_agent,     sizeof(s_agent),     "%s", agent ? agent : "AGENT");
    snprintf(s_action,    sizeof(s_action),    "%s", action ? action : "ACTION");
    snprintf(s_details,   sizeof(s_details),   "%s", details ? details : "No details provided.");
    snprintf(s_risk,      sizeof(s_risk),      "%s", risk ? risk : "HIGH");

    s_is_active = true;
    ESP_LOGI(TAG, ">>> POPPING APPROVAL MODAL: Action='%s' (ID: %s, Risk: %s) <<<", s_action, s_action_id, s_risk);

    // Render obsidian modal on AMOLED
    render_approval_modal();

    // Urgent haptic pulse: double pulse (40ms on, 60ms off, 40ms on)
    kin_audio_haptic_buzz(40);
    vTaskDelay(pdMS_TO_TICKS(60));
    kin_audio_haptic_buzz(40);

    // Urgent high-tech alert chime (G5 784 Hz -> C6 1046 Hz)
    kin_audio_play_tone(784, 80);
    vTaskDelay(pdMS_TO_TICKS(30));
    kin_audio_play_tone(1046, 120);

    // Launch background 60s timeout task
    if (s_timeout_task_handle) {
        TaskHandle_t th = s_timeout_task_handle;
        s_timeout_task_handle = NULL;
        vTaskDelete(th);
    }
    xTaskCreatePinnedToCore(approval_timeout_task, "appr_timeout", 3072, NULL, 4, &s_timeout_task_handle, 1);

    return ESP_OK;
}

esp_err_t kin_approval_hide(void) {
    if (!s_is_active) return ESP_OK;
    s_is_active = false;

    if (s_timeout_task_handle) {
        TaskHandle_t th = s_timeout_task_handle;
        s_timeout_task_handle = NULL;
        if (xTaskGetCurrentTaskHandle() != th) {
            vTaskDelete(th);
        }
    }

    ESP_LOGI(TAG, "Approval modal dismissed -> Restoring active Kin avatar");
    return kin_avatar_set_active(kin_avatar_get_active());
}

void kin_approval_handle_touch(uint16_t x, uint16_t y) {
    if (!s_is_active) return;

    ESP_LOGI(TAG, "Approval Touch Event at (%u, %u)", x, y);

    // Any touch in the lower decision zone (y >= 300)
    if (y >= 300) {
        if (x <= 205) {
            // REJECT BUTTON (Left side)
            ESP_LOGI(TAG, ">>> USER TAPPED [REJECT] for Action '%s' at (%u, %u) <<<", s_action_id, x, y);

            // Tactile buzz & low rejection tone
            kin_audio_haptic_buzz(35);
            kin_audio_play_tone(330, 90);

            // Visual feedback flash
            fill_rounded_rect(24, 352, 170, 78, 18, rgb565_be(180, 20, 20));
            int rej_fb_w = string_width("REJECTED!", 2);
            draw_string(24 + (170 - rej_fb_w) / 2, 352 + 30, "REJECTED!", 0xFFFF, 2);
            kin_display_draw_raw_frame((const uint8_t *)s_approval_frame);
            vTaskDelay(pdMS_TO_TICKS(250));

            // Dispatch WebSocket response
            kin_ws_send_approval_response(s_action_id, false);

            // Dismiss modal
            kin_approval_hide();
        } else {
            // APPROVE BUTTON (Right side)
            ESP_LOGI(TAG, ">>> USER TAPPED [APPROVE] for Action '%s' at (%u, %u) <<<", s_action_id, x, y);

            // Tactile buzz & celebratory chime (A5 880 Hz -> D6 1174 Hz)
            kin_audio_haptic_buzz(60);
            kin_audio_play_tone(880, 80);
            vTaskDelay(pdMS_TO_TICKS(25));
            kin_audio_play_tone(1174, 120);

            // Visual feedback flash
            fill_rounded_rect(216, 352, 170, 78, 18, rgb565_be(20, 160, 60));
            int app_fb_w = string_width("APPROVED!", 2);
            draw_string(216 + (170 - app_fb_w) / 2, 352 + 30, "APPROVED!", 0xFFFF, 2);
            kin_display_draw_raw_frame((const uint8_t *)s_approval_frame);
            vTaskDelay(pdMS_TO_TICKS(250));

            // Dispatch WebSocket response
            kin_ws_send_approval_response(s_action_id, true);

            // Dismiss modal
            kin_approval_hide();
        }
    } else {
        ESP_LOGW(TAG, "Approval Touch outside decision buttons at (%u, %u) - ignored", x, y);
    }
}
