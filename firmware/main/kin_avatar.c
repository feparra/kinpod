#include "kin_avatar.h"
#include "board_kinpod_2.06.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "KinAvatar";

// Raw avatar assets embedded by CMake
extern const uint8_t avatar_astro_start[]   asm("_binary_avatar_astro_raw_start");
extern const uint8_t avatar_crab_start[]    asm("_binary_avatar_crab_raw_start");
extern const uint8_t avatar_hermes_start[]  asm("_binary_avatar_hermes_raw_start");
extern const uint8_t avatar_muse_start[]    asm("_binary_avatar_muse_raw_start");
extern const uint8_t avatar_octopus_start[] asm("_binary_avatar_octopus_raw_start");
extern const uint8_t avatar_dots_start[]    asm("_binary_avatar_dots_raw_start");

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

esp_err_t kin_avatar_init(void) {
    ESP_LOGI(TAG, "KinPod Avatar Subsystem Initialized: 6 Kin Roster loaded.");
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

esp_err_t kin_avatar_set_active(kin_avatar_id_t id) {
    if (id >= KIN_AVATAR_COUNT) {
        return ESP_ERR_INVALID_ARG;
    }

    s_active_avatar = id;
    const kin_avatar_profile_t *p = &s_roster[id];
    ESP_LOGI(TAG, "Switched active Kin to [%d: %s (%s)]", id, p->name, p->key);

    return kin_display_draw_raw_frame(p->raw_asset_start);
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
