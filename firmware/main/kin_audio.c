#include "kin_audio.h"
#include "kin_avatar.h"
#include "kin_power.h"
#include "kin_ws.h"
#include "bsp/esp32_s3_touch_amoled_2_06.h"
#include "esp_codec_dev.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>
#include <string.h>

static const char *TAG = "KinAudio";

static esp_codec_dev_handle_t s_spk_handle = NULL;
static esp_codec_dev_handle_t s_mic_handle = NULL;
static bool s_ptt_pressed = false;
static kin_audio_record_cb_t s_record_cb = NULL;

void kin_audio_haptic_buzz(uint32_t duration_ms) {
    gpio_set_level(KIN_HAPTIC_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    gpio_set_level(KIN_HAPTIC_GPIO, 0);
}

#define CHUNK_SAMPLES 512

esp_err_t kin_audio_init(void) {
    ESP_LOGI(TAG, "Initializing KinPod Audio Subsystem (I2S0 16kHz Duplex)...");

    // 1. Configure I2S duplex bus
    esp_err_t ret = bsp_audio_init(NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "bsp_audio_init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // 2. Initialize ES8311 Speaker Codec (I2C 0x18)
    s_spk_handle = bsp_audio_codec_speaker_init();
    if (!s_spk_handle) {
        ESP_LOGE(TAG, "bsp_audio_codec_speaker_init failed");
        return ESP_FAIL;
    }

    esp_codec_dev_sample_info_t fs_spk = {
        .bits_per_sample = KIN_AUDIO_BITS_PER_SAMP,
        .channel = KIN_AUDIO_CHANNELS,
        .channel_mask = 0,
        .sample_rate = KIN_AUDIO_SAMPLE_RATE,
        .mclk_multiple = 256,
    };
    ret = esp_codec_dev_open(s_spk_handle, &fs_spk);
    if (ret != ESP_CODEC_DEV_OK) {
        ESP_LOGE(TAG, "Failed to open ES8311 speaker codec: %d", ret);
        return ESP_FAIL;
    }
    esp_codec_dev_set_out_vol(s_spk_handle, 85);
    ESP_LOGI(TAG, "ES8311 Speaker Codec Online: Volume set to 85%%");

    // 3. Initialize ES7210 Microphone Codec (I2C 0x40)
    s_mic_handle = bsp_audio_codec_microphone_init();
    if (!s_mic_handle) {
        ESP_LOGE(TAG, "bsp_audio_codec_microphone_init failed");
        return ESP_FAIL;
    }

    esp_codec_dev_sample_info_t fs_mic = {
        .bits_per_sample = KIN_AUDIO_BITS_PER_SAMP,
        .channel = KIN_AUDIO_CHANNELS,
        .channel_mask = 0,
        .sample_rate = KIN_AUDIO_SAMPLE_RATE,
        .mclk_multiple = 256,
    };
    ret = esp_codec_dev_open(s_mic_handle, &fs_mic);
    if (ret != ESP_CODEC_DEV_OK) {
        ESP_LOGE(TAG, "Failed to open ES7210 microphone codec: %d", ret);
        return ESP_FAIL;
    }
    esp_codec_dev_set_in_gain(s_mic_handle, 28.0); // 28 dB gain for voice capture
    ESP_LOGI(TAG, "ES7210 Microphone Codec Online: Gain set to 28dB");

    // 4. Configure PTT BOOT Button (GPIO 0, Active LOW)
    gpio_config_t btn_cfg = {
        .pin_bit_mask = (1ULL << KIN_PTT_GPIO_BOOT),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&btn_cfg);

    // 5. Configure Haptic Vibration Motor (GPIO 18, Active HIGH)
    gpio_config_t motor_cfg = {
        .pin_bit_mask = (1ULL << KIN_HAPTIC_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&motor_cfg);
    gpio_set_level(KIN_HAPTIC_GPIO, 0); // Motor OFF initially

    ESP_LOGI(TAG, "PTT Subsystem: BOOT Button on GPIO %d (Active LOW), Haptic Motor on GPIO %d [OK]",
             KIN_PTT_GPIO_BOOT, KIN_HAPTIC_GPIO);

    return ESP_OK;
}

esp_err_t kin_audio_set_volume(uint8_t volume_pct) {
    if (!s_spk_handle) return ESP_ERR_INVALID_STATE;
    if (volume_pct > 100) volume_pct = 100;
    return esp_codec_dev_set_out_vol(s_spk_handle, volume_pct) == ESP_CODEC_DEV_OK ? ESP_OK : ESP_FAIL;
}

esp_err_t kin_audio_play(const uint8_t *pcm_data, size_t len) {
    if (!s_spk_handle || !pcm_data || len == 0) return ESP_ERR_INVALID_ARG;
    int ret = esp_codec_dev_write(s_spk_handle, (void *)pcm_data, len);
    return ret == ESP_CODEC_DEV_OK ? ESP_OK : ESP_FAIL;
}

esp_err_t kin_audio_play_tone(uint16_t freq_hz, uint16_t duration_ms) {
    if (!s_spk_handle || freq_hz == 0 || duration_ms == 0) return ESP_ERR_INVALID_ARG;
    size_t samples = (duration_ms * KIN_AUDIO_SAMPLE_RATE) / 1000;
    int16_t *buf = (int16_t *)malloc(samples * sizeof(int16_t));
    if (!buf) return ESP_ERR_NO_MEM;
    const float pi = 3.1415926535f;
    for (size_t i = 0; i < samples; i++) {
        float t = (float)i / KIN_AUDIO_SAMPLE_RATE;
        float env = 0.5f * (1.0f - cosf(2.0f * pi * i / samples)); // Hann envelope
        buf[i] = (int16_t)(sinf(2.0f * pi * freq_hz * t) * env * 14000.0f);
    }
    esp_err_t ret = kin_audio_play((const uint8_t *)buf, samples * sizeof(int16_t));
    free(buf);
    return ret;
}

esp_err_t kin_audio_record(int16_t *buffer, size_t samples_to_read, size_t *samples_read) {
    if (!s_mic_handle || !buffer || samples_to_read == 0) return ESP_ERR_INVALID_ARG;
    size_t bytes_to_read = samples_to_read * sizeof(int16_t);
    int ret = esp_codec_dev_read(s_mic_handle, (void *)buffer, bytes_to_read);
    if (ret == ESP_CODEC_DEV_OK) {
        if (samples_read) *samples_read = samples_to_read;
        return ESP_OK;
    }
    return ESP_FAIL;
}

esp_err_t kin_audio_play_chime(void) {
    if (!s_spk_handle) return ESP_ERR_INVALID_STATE;

    ESP_LOGI(TAG, "Playing KinPod Boot Chime (D5 587Hz -> A5 880Hz)...");

    // Two tones: Tone 1 = 587 Hz (120ms), Tone 2 = 880 Hz (140ms)
    const size_t tone1_samples = (120 * KIN_AUDIO_SAMPLE_RATE) / 1000;
    const size_t tone2_samples = (140 * KIN_AUDIO_SAMPLE_RATE) / 1000;
    const size_t total_samples = tone1_samples + tone2_samples;

    int16_t *chime_buf = (int16_t *)malloc(total_samples * sizeof(int16_t));
    if (!chime_buf) return ESP_ERR_NO_MEM;

    const float pi = 3.1415926535f;

    // Tone 1: 587 Hz
    for (size_t i = 0; i < tone1_samples; i++) {
        float t = (float)i / KIN_AUDIO_SAMPLE_RATE;
        float env = 0.5f * (1.0f - cosf(2.0f * pi * i / tone1_samples)); // Hann window
        chime_buf[i] = (int16_t)(sinf(2.0f * pi * 587.33f * t) * env * 14000.0f);
    }

    // Tone 2: 880 Hz
    for (size_t i = 0; i < tone2_samples; i++) {
        float t = (float)i / KIN_AUDIO_SAMPLE_RATE;
        float env = 0.5f * (1.0f - cosf(2.0f * pi * i / tone2_samples)); // Hann window
        chime_buf[tone1_samples + i] = (int16_t)(sinf(2.0f * pi * 880.0f * t) * env * 16000.0f);
    }

    esp_err_t ret = kin_audio_play((const uint8_t *)chime_buf, total_samples * sizeof(int16_t));
    free(chime_buf);
    return ret;
}

bool kin_audio_is_ptt_pressed(void) {
    return s_ptt_pressed;
}

static void ptt_monitor_task(void *pvParameters) {
    int16_t pcm_chunk[CHUNK_SAMPLES];
    bool was_pressed = false;
    TickType_t press_start_tick = 0;
    TickType_t last_ui_update_tick = 0;
    uint32_t total_samples_captured = 0;
    int debounce_count = 0;

    ESP_LOGI(TAG, "PTT Monitor Task running: Press & hold physical BOOT (GPIO 0) to speak.");

    while (1) {
        // Physical BOOT button: Active LOW (0 = pressed, 1 = released)
        int raw_level = gpio_get_level(KIN_PTT_GPIO_BOOT);
        if (raw_level == 0) {
            if (debounce_count < 3) debounce_count++;
        } else {
            if (debounce_count > 0) debounce_count--;
        }

        bool is_pressed = (debounce_count >= 2);
        if (is_pressed && was_pressed) {
            uint32_t current_held_ms = (uint32_t)pdTICKS_TO_MS(xTaskGetTickCount() - press_start_tick);
            if (current_held_ms > 10000) {
                ESP_LOGW(TAG, "PTT hold timeout (>10s) -> auto-releasing");
                is_pressed = false;
                debounce_count = 0;
            }
        }
        s_ptt_pressed = is_pressed;

        if (is_pressed) {
            if (!was_pressed) {
                was_pressed = true;
                press_start_tick = xTaskGetTickCount();
                last_ui_update_tick = press_start_tick;
                total_samples_captured = 0;
                ESP_LOGI(TAG, ">>> PTT PRESSED (BOOT GPIO 0) -> LISTENING... <<<");

                // 1. Tactile haptic pulse (35ms motor buzz)
                gpio_set_level(KIN_HAPTIC_GPIO, 1);

                // 2. Play immediate start audio chirp (1046 Hz High C6)
                kin_audio_play_tone(1046, 45);
                gpio_set_level(KIN_HAPTIC_GPIO, 0);

                // 3. Display PTT listening overlay on AMOLED
                kin_avatar_show_ptt_overlay(true, 0);

                // 4. Send ptt_start event to KinBridge WebSocket
                kin_ws_send_ptt_start(kin_avatar_get_profile(kin_avatar_get_active())->key);
            }

            // Read audio chunk from ES7210 microphone
            size_t read_count = 0;
            if (kin_audio_record(pcm_chunk, CHUNK_SAMPLES, &read_count) == ESP_OK && read_count > 0) {
                total_samples_captured += read_count;

                // Calculate RMS energy (Voice Activity Detection VU meter)
                int64_t sum_sq = 0;
                int16_t peak = 0;
                for (size_t i = 0; i < read_count; i++) {
                    int16_t sample = pcm_chunk[i];
                    sum_sq += (int32_t)sample * sample;
                    int16_t abs_s = abs(sample);
                    if (abs_s > peak) peak = abs_s;
                }
                int rms = (int)sqrtf((float)sum_sq / read_count);
                int rms_pct = (rms * 100) / 2000;
                if (rms_pct > 100) rms_pct = 100;

                // Update on-screen VU waveform every ~60ms (15 FPS visualizer)
                TickType_t now = xTaskGetTickCount();
                if ((now - last_ui_update_tick) >= pdMS_TO_TICKS(60)) {
                    last_ui_update_tick = now;
                    kin_avatar_show_ptt_overlay(true, rms_pct);
                }

                if (s_record_cb) {
                    s_record_cb(pcm_chunk, read_count, false);
                }
            }
        } else {
            if (was_pressed) {
                was_pressed = false;
                uint32_t duration_ms = (uint32_t)pdTICKS_TO_MS(xTaskGetTickCount() - press_start_tick);
                ESP_LOGI(TAG, "<<< PTT RELEASED: %lu ms (%lu samples, %.1f KB) captured >>>",
                         duration_ms, total_samples_captured, (float)(total_samples_captured * 2) / 1024.0f);

                // 1. Tactile haptic release pulse (20ms)
                gpio_set_level(KIN_HAPTIC_GPIO, 1);

                // 2. Play immediate release audio chirp (659 Hz E5)
                kin_audio_play_tone(659, 40);
                gpio_set_level(KIN_HAPTIC_GPIO, 0);

                // 3. Update HUD state: If valid voice duration (>300ms), transition to THINKING
                if (duration_ms >= 300) {
                    kin_avatar_set_state(KIN_STATE_THINKING, 0);
                } else {
                    kin_avatar_set_state(KIN_STATE_IDLE, 0);
                }

                if (s_record_cb) {
                    s_record_cb(NULL, 0, true);
                }
            }
            vTaskDelay(pdMS_TO_TICKS(15)); // 66 Hz poll rate when idle
        }
    }
}

esp_err_t kin_audio_start_ptt_task(kin_audio_record_cb_t record_cb) {
    s_record_cb = record_cb;
    BaseType_t ret = xTaskCreatePinnedToCore(
        ptt_monitor_task,
        "ptt_monitor",
        4096,
        NULL,
        5,
        NULL,
        1
    );
    return ret == pdPASS ? ESP_OK : ESP_FAIL;
}

