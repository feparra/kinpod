#include "kin_avatar.h"
#include "kin_approval.h"
#include "kin_control_center.h"
#include "kin_pairing.h"
#include "kin_provisioning.h"
#include "board_kinpod_2.06.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "KinAvatar";

static inline bool is_display_locked_by_overlay(void) {
    return kin_approval_is_active() ||
           kin_control_center_is_open() ||
           kin_pairing_is_active() ||
           kin_provisioning_is_active();
}

// Raw avatar assets embedded by CMake
extern const uint8_t avatar_astro_start[]   asm("_binary_avatar_astro_raw_start");
extern const uint8_t avatar_crab_start[]    asm("_binary_avatar_crab_raw_start");
extern const uint8_t avatar_hermes_start[]  asm("_binary_avatar_hermes_raw_start");
extern const uint8_t avatar_muse_start[]    asm("_binary_avatar_muse_raw_start");
extern const uint8_t avatar_octopus_start[] asm("_binary_avatar_octopus_raw_start");
extern const uint8_t avatar_dots_start[]    asm("_binary_avatar_dots_raw_start");

// OpenClaw Crab Living Animation Frames embedded by CMake
extern const uint8_t crab_idle_start[]   asm("_binary_crab_idle_raw_start");
extern const uint8_t crab_blink_start[]  asm("_binary_crab_blink_raw_start");
extern const uint8_t crab_listen_start[] asm("_binary_crab_listen_raw_start");
extern const uint8_t crab_think_start[]  asm("_binary_crab_think_raw_start");
extern const uint8_t crab_speak_start[]  asm("_binary_crab_speak_raw_start");

#define KIN_SPRITE_X 85
#define KIN_SPRITE_Y 130
#define KIN_SPRITE_W 240
#define KIN_SPRITE_H 240
#define KIN_SPRITE_SIZE (KIN_SPRITE_W * KIN_SPRITE_H * sizeof(uint16_t)) // 115,200 bytes

// Dynamic PSRAM Animation Frames: 0: IDLE, 1: BLINK, 2: LISTEN, 3: THINK, 4: SPEAK
static uint8_t *s_active_sprites[5] = {NULL, NULL, NULL, NULL, NULL};
static bool s_spiffs_mounted = false;

static esp_err_t kin_avatar_draw_character_sprite(const uint8_t *raw_sprite) {
    if (is_display_locked_by_overlay()) {
        return ESP_OK; // Modal or Menu has display control, do not draw avatar sprite
    }
    if (raw_sprite) {
        return kin_display_draw_sprite(KIN_SPRITE_X, KIN_SPRITE_Y, KIN_SPRITE_W, KIN_SPRITE_H, raw_sprite);
    }
    return ESP_OK;
}

static const kin_avatar_profile_t s_roster[KIN_AVATAR_COUNT] = {
    {
        .id = KIN_AVATAR_ASTRO,
        .key = "astro",
        .name = "GEMINI",
        .subtitle = "MULTIMODAL LIVE",
        .accent_hex = 0xFF6B00,
        .raw_asset_start = avatar_astro_start
    },
    {
        .id = KIN_AVATAR_CRAB,
        .key = "crab",
        .name = "OPENCLAW",
        .subtitle = "AUTONOMOUS AGENT",
        .accent_hex = 0xEF4444,
        .raw_asset_start = avatar_crab_start
    },
    {
        .id = KIN_AVATAR_HERMES,
        .key = "hermes",
        .name = "HERMES",
        .subtitle = "FUNCTION CALLER",
        .accent_hex = 0x38BDF8,
        .raw_asset_start = avatar_hermes_start
    },
    {
        .id = KIN_AVATAR_MUSE,
        .key = "muse",
        .name = "MUSE",
        .subtitle = "CREATIVE ASSISTANT",
        .accent_hex = 0xFBBF24,
        .raw_asset_start = avatar_muse_start
    },
    {
        .id = KIN_AVATAR_OCTOPUS,
        .key = "octopus",
        .name = "OCTO-CLAUDE",
        .subtitle = "QUANTITATIVE AGENT",
        .accent_hex = 0xFF7A2F,
        .raw_asset_start = avatar_octopus_start
    },
    {
        .id = KIN_AVATAR_DOTS,
        .key = "dots",
        .name = "DOTS LOCAL",
        .subtitle = "OLLAMA CODER 14B",
        .accent_hex = 0x3B82F6,
        .raw_asset_start = avatar_dots_start
    }
};

static kin_avatar_id_t s_active_avatar = KIN_AVATAR_ASTRO;

#define PTT_OVERLAY_Y_START 414
#define PTT_OVERLAY_Y_END   484
#define PTT_OVERLAY_HEIGHT  (PTT_OVERLAY_Y_END - PTT_OVERLAY_Y_START) // 70 lines
#define PTT_OVERLAY_WIDTH   KIN_LCD_H_RES                             // 410 pixels
#define PTT_OVERLAY_PIXELS  (PTT_OVERLAY_WIDTH * PTT_OVERLAY_HEIGHT)  // 28,700 pixels = 57,400 bytes

static uint16_t *s_ptt_dma_buf = NULL;

static inline uint16_t hex_to_rgb565(uint32_t hex) {
    uint8_t r = (hex >> 16) & 0xFF;
    uint8_t g = (hex >> 8) & 0xFF;
    uint8_t b = hex & 0xFF;
    return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

static inline void draw_pixel(uint16_t *buf, int x, int y, uint16_t color) {
    if (x >= 0 && x < PTT_OVERLAY_WIDTH && y >= 0 && y < PTT_OVERLAY_HEIGHT) {
        buf[y * PTT_OVERLAY_WIDTH + x] = color;
    }
}

static void fill_rect(uint16_t *buf, int x, int y, int w, int h, uint16_t color) {
    for (int j = y; j < y + h; j++) {
        if (j < 0 || j >= PTT_OVERLAY_HEIGHT) continue;
        int row_offset = j * PTT_OVERLAY_WIDTH;
        for (int i = x; i < x + w; i++) {
            if (i < 0 || i >= PTT_OVERLAY_WIDTH) continue;
            buf[row_offset + i] = color;
        }
    }
}

static void fill_circle(uint16_t *buf, int cx, int cy, int r, uint16_t color) {
    for (int dy = -r; dy <= r; dy++) {
        for (int dx = -r; dx <= r; dx++) {
            if (dx * dx + dy * dy <= r * r) {
                draw_pixel(buf, cx + dx, cy + dy, color);
            }
        }
    }
}

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

static void draw_string(uint16_t *buf, int x, int y, const char *str, uint16_t color, int scale) {
    while (*str) {
        char c = *str++;
        if (c >= 'a' && c <= 'z') c -= 32;
        if (c >= ' ' && c <= 'Z') {
            const uint8_t *glyph = s_font5x7[c - ' '];
            for (int col = 0; col < 5; col++) {
                uint8_t line = glyph[col];
                for (int row = 0; row < 7; row++) {
                    if (line & (1 << row)) {
                        fill_rect(buf, x + col * scale, y + row * scale, scale, scale, color);
                    }
                }
            }
            x += (5 + 1) * scale;
        } else {
            x += 4 * scale;
        }
    }
}

static void avatar_ui_anim_task(void *pvParameters);

esp_err_t kin_avatar_init(void) {
    if (!s_ptt_dma_buf) {
        s_ptt_dma_buf = (uint16_t *)heap_caps_malloc(PTT_OVERLAY_PIXELS * sizeof(uint16_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
        if (!s_ptt_dma_buf) {
            s_ptt_dma_buf = (uint16_t *)malloc(PTT_OVERLAY_PIXELS * sizeof(uint16_t));
        }
    }

    // 1. Mount SPIFFS partition for Full Studio character animation assets
    esp_vfs_spiffs_conf_t spiffs_conf = {
        .base_path = "/spiffs",
        .partition_label = "assets",
        .max_files = 10,
        .format_if_mount_failed = false
    };
    esp_err_t spiffs_ret = esp_vfs_spiffs_register(&spiffs_conf);
    if (spiffs_ret != ESP_OK) {
        ESP_LOGW(TAG, "SPIFFS mount warning: %s (will use embedded fallback)", esp_err_to_name(spiffs_ret));
        s_spiffs_mounted = false;
    } else {
        s_spiffs_mounted = true;
        size_t total = 0, used = 0;
        esp_spiffs_info("assets", &total, &used);
        ESP_LOGI(TAG, "SPIFFS Partition [assets] Mounted: Total=%d KB, Used=%d KB", (int)(total / 1024), (int)(used / 1024));
    }

    // 2. Allocate the 5 high-speed sprite frame buffers in Octal PSRAM (576 KB total)
    for (int i = 0; i < 5; i++) {
        if (!s_active_sprites[i]) {
            s_active_sprites[i] = (uint8_t *)heap_caps_malloc(KIN_SPRITE_SIZE, MALLOC_CAP_SPIRAM);
            if (!s_active_sprites[i]) {
                s_active_sprites[i] = (uint8_t *)malloc(KIN_SPRITE_SIZE);
            }
        }
    }

    // Start background UI animation and timeout watchdog on Core 1
    xTaskCreatePinnedToCore(
        avatar_ui_anim_task,
        "avatar_anim",
        4096,
        NULL,
        4,
        NULL,
        1
    );

    ESP_LOGI(TAG, "KinPod Avatar Subsystem Initialized: 6 Kin Roster loaded, PSRAM buffers ready, watchdog active.");
    return ESP_OK;
}

const kin_avatar_profile_t *kin_avatar_get_profile(kin_avatar_id_t id) {
    if (id >= KIN_AVATAR_COUNT) {
        return &s_roster[0];
    }
    return &s_roster[id];
}

kin_avatar_id_t kin_avatar_get_active(void) {
    return s_active_avatar;
}

static void kin_avatar_load_sprites_for_active(void) {
    const kin_avatar_profile_t *p = &s_roster[s_active_avatar];
    const char *state_names[5] = {"idle", "blink", "listen", "think", "speak"};

    for (int i = 0; i < 5; i++) {
        bool loaded = false;
        if (s_spiffs_mounted && s_active_sprites[i]) {
            char path[64];
            snprintf(path, sizeof(path), "/spiffs/anim/%s_%s.raw", p->key, state_names[i]);
            FILE *f = fopen(path, "rb");
            if (f) {
                size_t read_bytes = fread(s_active_sprites[i], 1, KIN_SPRITE_SIZE, f);
                fclose(f);
                if (read_bytes == KIN_SPRITE_SIZE) {
                    loaded = true;
                }
            }
        }
        if (!loaded && s_active_avatar == KIN_AVATAR_CRAB && s_active_sprites[i]) {
            const uint8_t *crab_fallbacks[5] = {crab_idle_start, crab_blink_start, crab_listen_start, crab_think_start, crab_speak_start};
            memcpy(s_active_sprites[i], crab_fallbacks[i], KIN_SPRITE_SIZE);
            loaded = true;
        }
        if (!loaded && s_active_sprites[i]) {
            if (i > 0 && s_active_sprites[0]) {
                memcpy(s_active_sprites[i], s_active_sprites[0], KIN_SPRITE_SIZE);
            }
        }
    }
    ESP_LOGI(TAG, "Preloaded PSRAM animation frames for active Kin: %s (%s)", p->name, p->key);
}

esp_err_t kin_avatar_set_active(kin_avatar_id_t id) {
    if (id >= KIN_AVATAR_COUNT) {
        return ESP_ERR_INVALID_ARG;
    }

    s_active_avatar = id;
    const kin_avatar_profile_t *p = &s_roster[id];
    ESP_LOGI(TAG, "Switched active Kin to [%d: %s (%s)]", id, p->name, p->key);

    if (is_display_locked_by_overlay()) {
        kin_avatar_load_sprites_for_active();
        return ESP_OK;
    }

    // 1. Draw base 410x502 screen
    esp_err_t err = kin_display_draw_raw_frame(p->raw_asset_start);

    // 2. Preload active character frames from SPIFFS into PSRAM
    kin_avatar_load_sprites_for_active();

    // 3. Draw initial idle sprite (240x240)
    if (s_active_sprites[0]) {
        kin_avatar_draw_character_sprite(s_active_sprites[0]);
    }

    return err;
}

esp_err_t kin_avatar_next(void) {
    kin_avatar_id_t next_id = (s_active_avatar + 1) % KIN_AVATAR_COUNT;
    return kin_avatar_set_active(next_id);
}

esp_err_t kin_avatar_prev(void) {
    kin_avatar_id_t prev_id = (s_active_avatar == 0) ? (KIN_AVATAR_COUNT - 1) : (s_active_avatar - 1);
    return kin_avatar_set_active(prev_id);
}

esp_err_t kin_avatar_set_by_name(const char *name) {
    if (!name) return ESP_ERR_INVALID_ARG;

    for (int i = 0; i < KIN_AVATAR_COUNT; i++) {
        if (strcasecmp(name, s_roster[i].key) == 0 || strcasecmp(name, s_roster[i].name) == 0) {
            return kin_avatar_set_active((kin_avatar_id_t)i);
        }
    }

    ESP_LOGW(TAG, "Unknown avatar key: '%s'", name);
    return ESP_ERR_NOT_FOUND;
}

static kin_state_t s_current_state = KIN_STATE_IDLE;
static TickType_t s_state_enter_tick = 0;

kin_state_t kin_avatar_get_state(void) {
    return s_current_state;
}

static esp_err_t kin_avatar_render_hud(kin_state_t state, int level) {
    if (is_display_locked_by_overlay()) {
        return ESP_OK; // Modal or Menu has display control
    }
    if (!s_ptt_dma_buf) return ESP_ERR_INVALID_STATE;

    if (state == KIN_STATE_IDLE) {
        // Restore original bottom region directly from embedded flash asset
        const kin_avatar_profile_t *p = &s_roster[s_active_avatar];
        if (p->raw_asset_start) {
            size_t offset = PTT_OVERLAY_Y_START * PTT_OVERLAY_WIDTH * sizeof(uint16_t);
            size_t slice_size = PTT_OVERLAY_PIXELS * sizeof(uint16_t);
            memcpy(s_ptt_dma_buf, p->raw_asset_start + offset, slice_size);
            return kin_display_draw_region(0, PTT_OVERLAY_Y_START, KIN_LCD_H_RES, PTT_OVERLAY_Y_END, s_ptt_dma_buf);
        }
        return ESP_OK;
    }

    // Render dynamic HUD overlay
    const kin_avatar_profile_t *p = &s_roster[s_active_avatar];
    uint16_t accent_color = hex_to_rgb565(p->accent_hex);

    // 1. Clear background to AMOLED pitch black (0x0000)
    memset(s_ptt_dma_buf, 0x00, PTT_OVERLAY_PIXELS * sizeof(uint16_t));

    // 2. Draw Capsule Container (x=24 to x=386, y=6 to y=64)
    int cap_x = 24;
    int cap_y = 6;
    int cap_w = 362;
    int cap_h = 58;
    uint16_t bg_dark = 0x0842; // Deep slate obsidian

    fill_rect(s_ptt_dma_buf, cap_x + 4, cap_y + 4, cap_w - 8, cap_h - 8, bg_dark);

    // Draw 2px border in accent color
    fill_rect(s_ptt_dma_buf, cap_x + 6, cap_y, cap_w - 12, 2, accent_color);
    fill_rect(s_ptt_dma_buf, cap_x + 6, cap_y + cap_h - 2, cap_w - 12, 2, accent_color);
    fill_rect(s_ptt_dma_buf, cap_x, cap_y + 6, 2, cap_h - 12, accent_color);
    fill_rect(s_ptt_dma_buf, cap_x + cap_w - 2, cap_y + 6, 2, cap_h - 12, accent_color);

    if (state == KIN_STATE_LISTENING) {
        // Red Recording Indicator (Glowing circle)
        fill_circle(s_ptt_dma_buf, cap_x + 28, 35, 9, 0x6000); // Red halo
        fill_circle(s_ptt_dma_buf, cap_x + 28, 35, 6, 0xF800); // Bright red core

        draw_string(s_ptt_dma_buf, cap_x + 44, 28, "LISTENING", 0xFFFF, 2);

        // Draw Dynamic Audio VU Equalizer Bars
        const uint8_t weights[10] = {50, 80, 100, 70, 95, 100, 85, 90, 65, 45};
        int eq_start_x = cap_x + 195;
        int cy = 35;
        int vu = (level > 100) ? 100 : (level < 0 ? 0 : level);

        for (int i = 0; i < 10; i++) {
            int bx = eq_start_x + i * 14;
            int bar_h = 6 + (vu * weights[i] * 36) / 10000;
            if (bar_h > 42) bar_h = 42;
            int by = cy - (bar_h / 2);

            uint16_t bar_col = (bar_h > 30) ? 0xF800 : (bar_h > 18 ? 0xFDE0 : 0x3DEF);
            fill_rect(s_ptt_dma_buf, bx, by, 8, bar_h, bar_col);
        }
    } else if (state == KIN_STATE_THINKING) {
        // Amber / Yellow thinking indicator
        fill_circle(s_ptt_dma_buf, cap_x + 28, 35, 9, 0x6300); // Amber halo
        fill_circle(s_ptt_dma_buf, cap_x + 28, 35, 6, 0xFDE0); // Amber core

        draw_string(s_ptt_dma_buf, cap_x + 44, 28, "THINKING...", 0xFFFF, 2);

        // Animated pulse dots
        int dot_base_x = cap_x + 235;
        int step = (level % 4);
        for (int i = 0; i < 4; i++) {
            uint16_t col = (i == step) ? 0xFFFF : 0x4208;
            fill_circle(s_ptt_dma_buf, dot_base_x + i * 20, 35, 5, col);
        }
    } else if (state == KIN_STATE_SPEAKING) {
        // Cyan / Accent glowing speaking indicator
        fill_circle(s_ptt_dma_buf, cap_x + 28, 35, 9, 0x0210); // Cyan halo
        fill_circle(s_ptt_dma_buf, cap_x + 28, 35, 6, 0x07FF); // Cyan core

        draw_string(s_ptt_dma_buf, cap_x + 44, 28, "SPEAKING", 0xFFFF, 2);

        // Animated soundwave equalizer bars (speech reaction)
        const uint8_t spk_weights[10] = {60, 90, 75, 100, 85, 95, 80, 70, 85, 60};
        int eq_start_x = cap_x + 195;
        int cy = 35;
        int spk_lvl = (level > 0) ? level : 50;

        for (int i = 0; i < 10; i++) {
            int bx = eq_start_x + i * 14;
            int wave_factor = ((spk_weights[(i + (level / 5)) % 10]) * spk_lvl) / 100;
            int bar_h = 8 + (wave_factor * 34) / 100;
            if (bar_h > 42) bar_h = 42;
            int by = cy - (bar_h / 2);

            uint16_t bar_col = (i % 2 == 0) ? accent_color : 0x07FF;
            fill_rect(s_ptt_dma_buf, bx, by, 8, bar_h, bar_col);
        }
    }

    // Push to AMOLED screen via DMA
    return kin_display_draw_region(0, PTT_OVERLAY_Y_START, KIN_LCD_H_RES, PTT_OVERLAY_Y_END, s_ptt_dma_buf);
}

esp_err_t kin_avatar_set_state(kin_state_t state, int level) {
    kin_state_t prev_state = s_current_state;
    s_current_state = state;
    s_state_enter_tick = xTaskGetTickCount();

    if (is_display_locked_by_overlay()) {
        return ESP_OK; // Modal or Menu active: update internal state but suspend display drawing
    }

    // Trigger state-specific character animation frame for the active avatar
    if (state != prev_state) {
        int sprite_idx = 0;
        switch (state) {
            case KIN_STATE_IDLE:      sprite_idx = 0; break;
            case KIN_STATE_LISTENING: sprite_idx = 2; break;
            case KIN_STATE_THINKING:  sprite_idx = 3; break;
            case KIN_STATE_SPEAKING:  sprite_idx = 4; break;
        }
        if (s_active_sprites[sprite_idx]) {
            kin_avatar_draw_character_sprite(s_active_sprites[sprite_idx]);
        }
    }

    return kin_avatar_render_hud(state, level);
}

static void avatar_ui_anim_task(void *pvParameters) {
    int anim_step = 0;
    int idle_ticks = 0;
    bool speaking_frame_toggle = false;

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(150));
        if (is_display_locked_by_overlay()) {
            idle_ticks = 0;
            continue; // Modal or Menu is active: suspend all background animations
        }
        anim_step++;

        if (s_current_state == KIN_STATE_IDLE) {
            idle_ticks++;
            // Natural living blink every ~9.0s (60 ticks * 150ms = 9000ms) for an organic companion feel
            if (idle_ticks % 60 == 0) {
                if (s_active_sprites[1] && s_active_sprites[0]) {
                    ESP_LOGI(TAG, "Living Blink triggered for %s!", s_roster[s_active_avatar].name);
                    kin_avatar_draw_character_sprite(s_active_sprites[1]);
                    vTaskDelay(pdMS_TO_TICKS(140));
                    kin_avatar_draw_character_sprite(s_active_sprites[0]);
                }
            }
        } else if (s_current_state == KIN_STATE_THINKING) {
            idle_ticks = 0;
            TickType_t elapsed = xTaskGetTickCount() - s_state_enter_tick;
            if (elapsed > pdMS_TO_TICKS(8000)) { // 8s watchdog timeout
                ESP_LOGW(TAG, "Watchdog: Thinking state timeout (>8s) -> Restoring IDLE");
                kin_avatar_set_state(KIN_STATE_IDLE, 0);
            } else {
                kin_avatar_render_hud(KIN_STATE_THINKING, anim_step);
            }
        } else if (s_current_state == KIN_STATE_SPEAKING) {
            idle_ticks = 0;
            TickType_t elapsed = xTaskGetTickCount() - s_state_enter_tick;
            if (elapsed > pdMS_TO_TICKS(15000)) { // 15s watchdog timeout for speaking
                ESP_LOGW(TAG, "Watchdog: Speaking state timeout (>15s) -> Restoring IDLE");
                kin_avatar_set_state(KIN_STATE_IDLE, 0);
            } else {
                // Real mouth articulation and gesturing in sync with voice audio for all characters!
                speaking_frame_toggle = !speaking_frame_toggle;
                uint8_t *frame = speaking_frame_toggle ? s_active_sprites[4] : s_active_sprites[0];
                if (frame) {
                    kin_avatar_draw_character_sprite(frame);
                }
                kin_avatar_render_hud(KIN_STATE_SPEAKING, 40 + ((anim_step % 6) * 10));
            }
        } else if (s_current_state == KIN_STATE_LISTENING) {
            idle_ticks = 0;
            TickType_t elapsed = xTaskGetTickCount() - s_state_enter_tick;
            if (elapsed > pdMS_TO_TICKS(8000)) { // 8s watchdog timeout for listening
                ESP_LOGW(TAG, "Watchdog: Listening state timeout (>8s) -> Restoring IDLE");
                kin_avatar_set_state(KIN_STATE_IDLE, 0);
            }
        }
    }
}

esp_err_t kin_avatar_show_ptt_overlay(bool active, int vu_pct) {
    return kin_avatar_set_state(active ? KIN_STATE_LISTENING : KIN_STATE_IDLE, vu_pct);
}

