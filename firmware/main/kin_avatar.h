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

#ifdef __cplusplus
}
#endif
