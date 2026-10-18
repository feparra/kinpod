#include "kin_audio.h"
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

    // 4. Configure PTT Button (GPIO 0 / BOOT button)
    gpio_config_t btn_cfg = {
        .pin_bit_mask = (1ULL << KIN_PTT_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&btn_cfg);
    ESP_LOGI(TAG, "PTT Button Configured on GPIO %d (BOOT button, Active LOW)", KIN_PTT_GPIO);

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
    uint32_t total_samples_captured = 0;

    ESP_LOGI(TAG, "PTT Monitor Task running: Press & hold BOOT (GPIO 0) to speak.");

    while (1) {
        // Active LOW: 0 = pressed, 1 = released
        bool is_pressed = (gpio_get_level(KIN_PTT_GPIO) == 0);
        s_ptt_pressed = is_pressed;

        if (is_pressed) {
            if (!was_pressed) {
                was_pressed = true;
                press_start_tick = xTaskGetTickCount();
                total_samples_captured = 0;
                ESP_LOGI(TAG, ">>> PTT PRESSED (GPIO 0) -> Capturing voice audio... <<<");
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

                // Print VU meter bar every ~160ms if active
                if ((total_samples_captured % (CHUNK_SAMPLES * 5)) < CHUNK_SAMPLES) {
                    char vu_bar[11] = "          ";
                    int bars = (rms * 10) / 4000;
                    if (bars > 10) bars = 10;
                    for (int b = 0; b < bars; b++) vu_bar[b] = '#';
                    ESP_LOGI(TAG, "MIC VU: [%s] RMS=%d | Peak=%d", vu_bar, rms, peak);
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

                if (s_record_cb) {
                    s_record_cb(NULL, 0, true);
                }
            }
            vTaskDelay(pdMS_TO_TICKS(20)); // 50 Hz poll rate when idle
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
