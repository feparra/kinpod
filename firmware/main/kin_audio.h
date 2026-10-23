#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define KIN_AUDIO_SAMPLE_RATE    16000
#define KIN_AUDIO_BITS_PER_SAMP  16
#define KIN_AUDIO_CHANNELS       1
#define KIN_PTT_GPIO_BOOT        GPIO_NUM_0  // Physical BOOT button (Active LOW)
#define KIN_HAPTIC_GPIO          GPIO_NUM_18 // Onboard Vibration Motor (Active HIGH)

/**
 * @brief Callback invoked during PTT recording with captured 16kHz PCM data.
 * @param pcm_data Pointer to raw 16-bit PCM buffer.
 * @param samples_count Number of samples in buffer.
 * @param is_final True when user released the PTT button.
 */
typedef void (*kin_audio_record_cb_t)(const int16_t *pcm_data, size_t samples_count, bool is_final);

/**
 * @brief Initializes ES7210 microphone, ES8311 speaker, NS4150 power amp, and I2S0 bus.
 */
esp_err_t kin_audio_init(void);

/**
 * @brief Sets speaker output volume (0 - 100).
 */
esp_err_t kin_audio_set_volume(uint8_t volume_pct);

/**
 * @brief Plays raw 16kHz 16-bit mono PCM audio through the speaker.
 */
esp_err_t kin_audio_play(const uint8_t *pcm_data, size_t len);

/**
 * @brief Plays a synthesized pure sine wave tone through the chest speaker.
 */
esp_err_t kin_audio_play_tone(uint16_t freq_hz, uint16_t duration_ms);

/**
 * @brief Synthesizes and plays a smooth, warm two-tone boot/ready chime.
 */
esp_err_t kin_audio_play_chime(void);

/**
 * @brief Reads audio samples directly from the ES7210 microphone.
 */
esp_err_t kin_audio_record(int16_t *buffer, size_t samples_to_read, size_t *samples_read);

/**
 * @brief Starts the background PTT monitor task.
 */
esp_err_t kin_audio_start_ptt_task(kin_audio_record_cb_t record_cb);

/**
 * @brief Returns true if the PTT button is currently held down.
 */
bool kin_audio_is_ptt_pressed(void);

/**
 * @brief Triggers a brief haptic vibration pulse via the onboard vibration motor (GPIO 18).
 */
void kin_audio_haptic_buzz(uint32_t duration_ms);

#ifdef __cplusplus
}
#endif
