#include "bomb_sound.h"
#include "bomb_sound_asset.h"
#include "bomb_game.h"
#include "bsp_audio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <stddef.h>
#include <string.h>

static const char *TAG = "bomb_sound";
static QueueHandle_t s_commands;

static uint64_t sound_now_ms(void) {
    return (uint64_t)esp_timer_get_time() / 1000u;
}

static void play_explosion(uint64_t started_ms) {
    // Never let a delayed worker play an explosion after its result screen ends.
    if (sound_now_ms() - started_ms > 400u) return;
    esp_err_t error = bsp_audio_wake();
    if (error == ESP_OK) error = bsp_audio_set_format(16000, 16, 1);
    if (error == ESP_OK && sound_now_ms() - started_ms <= 400u) {
        size_t length;
        const uint8_t *pcm = bomb_sound_pcm(&length);
        int16_t chunk[512];
        for (size_t offset = 0; offset < length; offset += sizeof(chunk)) {
            if (sound_now_ms() - started_ms >= BOMB_EXPLOSION_MS) break;
            const size_t bytes = length - offset < sizeof(chunk) ? length - offset : sizeof(chunk);
            memcpy(chunk, pcm + offset, bytes);
            error = bsp_audio_write(chunk, bytes);
            if (error != ESP_OK) break;
        }
    }
    if (error != ESP_OK) ESP_LOGW(TAG, "Explosion playback failed: %d", error);
    // The asset ends with at least 128 ms silence, longer than the documented 90 ms DMA queue.
    // PCM production has stopped before suspending the codec. No worker references UI.
    if (bsp_audio_sleep() != ESP_OK) ESP_LOGW(TAG, "Codec suspend failed after explosion");
}

static void sound_worker(void *argument) {
    (void)argument;
    for (;;) {
        uint64_t started_ms;
        if (xQueueReceive(s_commands, &started_ms, portMAX_DELAY) == pdTRUE)
            play_explosion(started_ms);
    }
}

esp_err_t bomb_sound_init(void) {
    if (s_commands) return ESP_OK;
    esp_err_t error = bsp_audio_init();
    if (error != ESP_OK) return error;
    bsp_audio_set_volume(75);
    error = bsp_audio_sleep();
    if (error != ESP_OK) return error;
    s_commands = xQueueCreate(1, sizeof(uint64_t));
    if (!s_commands) return ESP_ERR_NO_MEM;
    if (xTaskCreate(sound_worker, "bomb_sound", 4096, NULL, 4, NULL) != pdPASS) {
        vQueueDelete(s_commands);
        s_commands = NULL;
        return ESP_ERR_NO_MEM;
    }
    ESP_LOGI(TAG, "Explosion sound ready: 16000Hz/16bit/mono, volume 75%%");
    return ESP_OK;
}

void bomb_sound_explode(uint64_t started_ms) {
    if (s_commands && xQueueSend(s_commands, &started_ms, 0) != pdTRUE)
        ESP_LOGW(TAG, "Explosion sound busy; event dropped");
}
