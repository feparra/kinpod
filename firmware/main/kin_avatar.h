#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    KIN_AVATAR_ASTRO = 0,    // Gemini 2.5 Flash (#FF6B00)
    KIN_AVATAR_CRAB = 1,     // OpenClaw (#EF4444)
    KIN_AVATAR_HERMES = 2,   // Hermes Agent (#38BDF8)
    KIN_AVATAR_MUSE = 3,     // Muse Companion (#FBBF24)
    KIN_AVATAR_OCTOPUS = 4,  // Octo-Claude (#FF7A2F)
    KIN_AVATAR_DOTS = 5,     // Dots Ollama Local (#3B82F6)
    KIN_AVATAR_COUNT = 6
} kin_avatar_id_t;

typedef struct {
    kin_avatar_id_t id;
    const char *key;
    const char *name;
    const char *subtitle;
    uint32_t accent_hex;
    const uint8_t *raw_asset_start;
} kin_avatar_profile_t;

/**
 * @brief Initialize Kin Avatar subsystem and roster.
 */
esp_err_t kin_avatar_init(void);

/**
 * @brief Get profile of avatar by ID.
 */
const kin_avatar_profile_t *kin_avatar_get_profile(kin_avatar_id_t id);

/**
 * @brief Get currently active avatar ID.
 */
kin_avatar_id_t kin_avatar_get_active(void);

/**
 * @brief Set currently active avatar by ID and stream to display.
 */
esp_err_t kin_avatar_set_active(kin_avatar_id_t id);

/**
 * @brief Switch to next avatar (swipe left).
 */
esp_err_t kin_avatar_next(void);

/**
 * @brief Switch to previous avatar (swipe right).
 */
esp_err_t kin_avatar_prev(void);

/**
 * @brief Set avatar by string key (e.g. "crab", "astro", "hermes", etc.).
 */
esp_err_t kin_avatar_set_by_name(const char *name);

typedef enum {
    KIN_STATE_IDLE = 0,
    KIN_STATE_LISTENING = 1,
    KIN_STATE_THINKING = 2,
    KIN_STATE_SPEAKING = 3
} kin_state_t;

/**
 * @brief Sets character interaction state and displays corresponding UI feedback.
 * @param state KIN_STATE_IDLE, KIN_STATE_LISTENING, KIN_STATE_THINKING, or KIN_STATE_SPEAKING.
 * @param level Audio VU level (0-100) or animation tick.
 */
esp_err_t kin_avatar_set_state(kin_state_t state, int level);

/**
 * @brief Returns the current character state.
 */
kin_state_t kin_avatar_get_state(void);

/**
 * @brief Shows or hides the dynamic Push-To-Talk listening HUD overlay on the AMOLED panel (legacy wrapper).
 * @param active true to display recording pill and animated VU bars; false to restore avatar.
 * @param vu_pct Current mic audio VU meter level (0 to 100).
 */
esp_err_t kin_avatar_show_ptt_overlay(bool active, int vu_pct);

#ifdef __cplusplus
}
#endif

