// Run the real worker against a fake codec; compare every write with the retained PCM.
#include <assert.h>
#include <stdbool.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../main/bomb_sound.c"
#include "bomb_game.h"

static uint8_t pcm[61440];
static unsigned init_calls, wakes, formats, sleeps, writes, creates, deletes;
static size_t written;
static uint64_t clock_ms, command;
static unsigned queue_count, fail_write;
static bool fail_init, fail_sleep, fail_wake, fail_format, fail_queue, fail_task;
static TaskFunction_t worker;
static jmp_buf done;

const uint8_t *bomb_sound_pcm(size_t *length) { *length = sizeof(pcm); return pcm; }
int64_t esp_timer_get_time(void) { return (int64_t)clock_ms * 1000; }
esp_err_t bsp_audio_init(void) { ++init_calls; return fail_init ? ESP_FAIL : ESP_OK; }
void bsp_audio_set_volume(uint8_t volume) { assert(volume == 75); }
esp_err_t bsp_audio_wake(void) { ++wakes; clock_ms += 30; return fail_wake ? ESP_FAIL : ESP_OK; }
esp_err_t bsp_audio_set_format(uint32_t hz, uint8_t bits, uint8_t channels) {
    assert(hz == 16000 && bits == 16 && channels == 1); ++formats;
    return fail_format ? ESP_FAIL : ESP_OK;
}
esp_err_t bsp_audio_sleep(void) { ++sleeps; return fail_sleep ? ESP_FAIL : ESP_OK; }
esp_err_t bsp_audio_write(const void *buffer, size_t bytes) {
    assert(bytes > 0 && bytes <= 1024 && bytes % 2 == 0);
    assert(written + bytes <= sizeof(pcm));
    assert(!memcmp(buffer, pcm + written, bytes));
    ++writes;
    if (fail_write == writes) return ESP_FAIL;
    written += bytes; clock_ms += bytes / 32; // Real writes block; the caller is the worker.
    return ESP_OK;
}
QueueHandle_t xQueueCreate(UBaseType_t length, UBaseType_t item_size) {
    assert(length == 1 && item_size == sizeof(uint64_t));
    ++creates; return fail_queue ? NULL : (QueueHandle_t)1;
}
void vQueueDelete(QueueHandle_t queue) { assert(queue); ++deletes; }
BaseType_t xQueueSend(QueueHandle_t queue, const void *item, TickType_t timeout) {
    assert(queue && timeout == 0);
    if (queue_count) return pdFALSE;
    memcpy(&command, item, sizeof(command)); queue_count = 1; return pdTRUE;
}
BaseType_t xQueueReceive(QueueHandle_t queue, void *item, TickType_t timeout) {
    assert(queue && timeout == portMAX_DELAY);
    if (!queue_count) longjmp(done, 1);
    memcpy(item, &command, sizeof(command)); queue_count = 0; return pdTRUE;
}
BaseType_t xTaskCreate(TaskFunction_t function, const char *name, uint32_t stack,
                       void *argument, UBaseType_t priority, TaskHandle_t *handle) {
    assert(!strcmp(name, "bomb_sound") && stack == 4096 && priority == 4 && !argument && !handle);
    worker = function; return fail_task ? pdFALSE : pdPASS;
}
static void reset(void) {
    s_commands = NULL;
    init_calls = wakes = formats = sleeps = writes = creates = deletes = 0;
    written = clock_ms = command = queue_count = fail_write = 0;
    fail_init = fail_sleep = fail_wake = fail_format = fail_queue = fail_task = false;
    worker = NULL;
}
static void run_worker(void) {
    assert(worker);
    if (setjmp(done) == 0) worker(NULL);
}

int main(int argc, char **argv) {
    assert(argc == 2);
    FILE *file = fopen(argv[1], "rb"); assert(file);
    assert(fread(pcm, 1, sizeof(pcm), file) == sizeof(pcm));
    assert(fgetc(file) == EOF); fclose(file);
    reset();
    bomb_sound_explode(0); assert(queue_count == 0);
    assert(bomb_sound_init() == ESP_OK);
    assert(bomb_sound_init() == ESP_OK && init_calls == 1 && creates == 1);
    bomb_sound_explode(0); bomb_sound_explode(1);
    assert(writes == 0 && wakes == 0 && queue_count == 1); // Non-blocking trigger, busy event dropped.
    run_worker();
    assert(written == sizeof(pcm) && writes == 60 && sleeps == 2);
    assert(wakes == 1 && formats == 1 && clock_ms < BOMB_EXPLOSION_MS);
    bomb_sound_explode(clock_ms); written = writes = 0;
    run_worker(); assert(written == sizeof(pcm) && wakes == 2 && sleeps == 3);

    reset(); assert(bomb_sound_init() == ESP_OK);
    bomb_sound_explode(0); clock_ms = 401;
    run_worker(); assert(!wakes && !writes && sleeps == 1); // Stale events never play late.
    reset(); assert(bomb_sound_init() == ESP_OK);
    bomb_sound_explode(0); clock_ms = 390;
    run_worker(); assert(wakes == 1 && !writes && sleeps == 2); // Slow wake also misses its window.

    reset(); assert(bomb_sound_init() == ESP_OK); fail_write = 3;
    bomb_sound_explode(0); run_worker();
    assert(writes == 3 && written == 2048 && sleeps == 2);
    fail_write = 0; written = writes = 0;
    bomb_sound_explode(clock_ms); run_worker();
    assert(written == sizeof(pcm) && sleeps == 3); // A later round retries after an I/O error.
    reset(); assert(bomb_sound_init() == ESP_OK); fail_wake = true;
    bomb_sound_explode(0); run_worker(); assert(!writes && !formats && sleeps == 2);
    reset(); assert(bomb_sound_init() == ESP_OK); fail_format = true;
    bomb_sound_explode(0); run_worker(); assert(!writes && formats == 1 && sleeps == 2);
    reset(); assert(bomb_sound_init() == ESP_OK); fail_sleep = true;
    bomb_sound_explode(0); run_worker(); assert(written == sizeof(pcm) && sleeps == 2);

    reset(); fail_init = true; assert(bomb_sound_init() != ESP_OK && !creates);
    reset(); fail_sleep = true; assert(bomb_sound_init() != ESP_OK && !creates);
    reset(); fail_queue = true; assert(bomb_sound_init() == ESP_ERR_NO_MEM && !s_commands);
    reset(); fail_task = true; assert(bomb_sound_init() == ESP_ERR_NO_MEM && deletes == 1 && !s_commands);
    puts("Bomb sound: non-blocking one-shot queue, full PCM stream, replay, stale rejection, codec/write/allocation failures PASS");
}
