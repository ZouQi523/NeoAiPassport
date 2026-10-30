// Exercise the actual firmware entry point/callback/queue loop against scripted BSP inputs.
#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../main/main.c"

typedef struct {
    uint64_t at;
    int key; // -1: no event; otherwise deliver the specified edge plus ignored aggregation events.
    bool released;
    bomb_state_t state;
    uint32_t seconds;
    uint64_t age;
    bool deny_ui;
    int brightness;
    bsp_btn_ev_t event;
} step_t;

typedef struct { unsigned head, count, size; unsigned char items[24][32]; } fake_queue_t;
static const step_t *steps;
static unsigned step_count, step_index, renders;
static uint64_t clock_ms;
static bsp_btn_cb_t callback;
static void *callback_user;
static jmp_buf done;
static bool display_fail, font_fail, button_fail, queue_fail, sound_fail;
static unsigned created, deleted, ui_errors, random_calls;
static unsigned explosions;
static int light;
static const uint32_t random_words[] = {UINT32_MAX, 1, 2, 0};

int64_t esp_timer_get_time(void) { return (int64_t)clock_ms * 1000; }
uint32_t esp_random(void) {
    assert(random_calls < sizeof(random_words) / sizeof(random_words[0]));
    return random_words[random_calls++];
}
esp_err_t bsp_display_init(void) { return display_fail ? ESP_FAIL : ESP_OK; }
struct _lv_display_t *bsp_lvgl_init(void) { return (struct _lv_display_t *)1; }
bool bsp_lvgl_lock(int timeout) {
    (void)timeout;
    return step_index == 0 || !steps[step_index-1].deny_ui;
}
void bsp_lvgl_unlock(void) { }
void bsp_display_backlight(uint8_t value) { light = value; }
bool bomb_ui_init(void) { return !font_fail; }
void bomb_ui_error(const char *detail) { assert(!strcmp(detail, "BUTTON ERROR")); ++ui_errors; }
void bomb_ui_render(const bomb_game_t *game, uint64_t now, int battery) {
    assert(step_index && step_index <= step_count);
    const step_t *step = &steps[step_index-1];
    assert(game->state == step->state);
    assert(bomb_game_remaining(game, now) == step->seconds);
    assert(battery == -1); // An unavailable battery must not stop the game.
    assert(light == step->brightness);
    ++renders;
}
esp_err_t bomb_sound_init(void) { return sound_fail ? ESP_FAIL : ESP_OK; }
void bomb_sound_explode(uint64_t started_ms) { assert(started_ms == clock_ms); ++explosions; }
esp_err_t bsp_battery_init(void) { return ESP_FAIL; }
int bsp_battery_soc(void) { assert(false); return -1; }
esp_err_t bsp_button_init(bsp_btn_cb_t cb, void *user) {
    callback = cb; callback_user = user;
    return button_fail ? ESP_FAIL : ESP_OK;
}
int bsp_button_read_mv(void) {
    return steps[step_index-1].released ? BSP_BTN_MV_RELEASED_MIN : 0;
}
QueueHandle_t xQueueCreate(UBaseType_t length, UBaseType_t size) {
    assert(length == 24 && size <= 32); ++created;
    if (queue_fail) return NULL;
    fake_queue_t *queue = calloc(1, sizeof(*queue)); assert(queue);
    queue->size = size; return queue;
}
BaseType_t xQueueSend(QueueHandle_t handle, const void *item, TickType_t timeout) {
    fake_queue_t *queue = handle; assert(timeout == 0);
    if (queue->count == 24) return pdFALSE;
    memcpy(queue->items[(queue->head+queue->count)%24], item, queue->size);
    ++queue->count; return pdTRUE;
}
BaseType_t xQueueReceive(QueueHandle_t handle, void *item, TickType_t timeout) {
    fake_queue_t *queue = handle; assert(timeout == pdMS_TO_TICKS(20));
    if (step_index == step_count) { assert(queue->count == 0); longjmp(done, 1); }
    const step_t *step = &steps[step_index++];
    clock_ms = step->at - step->age;
    if (step->key >= 0) {
        callback((bsp_btn_t)step->key, step->event, callback_user);
        callback((bsp_btn_t)step->key, BSP_BTN_CLICK, callback_user);
        callback((bsp_btn_t)step->key, BSP_BTN_DOUBLE, callback_user);
    }
    clock_ms = step->at;
    if (!queue->count) return pdFALSE;
    memcpy(item, queue->items[queue->head], queue->size);
    queue->head = (queue->head+1)%24; --queue->count; return pdTRUE;
}
void vQueueDelete(QueueHandle_t queue) { ++deleted; free(queue); }

static void reset(void) {
    if (s_keys) vQueueDelete(s_keys);
    s_keys = NULL; s_accept_keys = false; s_dropped = 0;
    display_fail = font_fail = button_fail = queue_fail = sound_fail = false;
    explosions = 0;
    step_index = renders = created = deleted = ui_errors = random_calls = 0;
    clock_ms = 0; callback = NULL; light = 0;
}

int main(void) {
#define EDGE(t, k, e, st, sec) {t, k, true, st, sec, 0, false, 65, e}
    const step_t script[] = {
        {50, BSP_BTN_OK, false, BOMB_SETUP, 300, 0, false, 65, BSP_BTN_PRESS},
        EDGE(100, -1, BSP_BTN_PRESS, BOMB_SETUP, 300),
        {150, -1, false, BOMB_SETUP, 300, 0, false, 65, BSP_BTN_PRESS},
        EDGE(200, -1, BSP_BTN_PRESS, BOMB_SETUP, 300),
        EDGE(250, -1, BSP_BTN_PRESS, BOMB_SETUP, 300),
        EDGE(270, BSP_BTN_UP, BSP_BTN_PRESS, BOMB_SETUP, 300),
        EDGE(300, BSP_BTN_UP, BSP_BTN_RELEASE, BOMB_SETUP, 360),
        EDGE(310, BSP_BTN_DOWN, BSP_BTN_PRESS, BOMB_SETUP, 360),
        EDGE(350, BSP_BTN_DOWN, BSP_BTN_RELEASE, BOMB_SETUP, 300),
        EDGE(400, BSP_BTN_UP, BSP_BTN_PRESS, BOMB_SETUP, 300),
        EDGE(900, BSP_BTN_UP, BSP_BTN_LONG, BOMB_SETUP, 600),
        EDGE(920, BSP_BTN_UP, BSP_BTN_LONG, BOMB_SETUP, 600),
        EDGE(950, BSP_BTN_UP, BSP_BTN_RELEASE, BOMB_SETUP, 600),
        EDGE(1000, BSP_BTN_DOWN, BSP_BTN_PRESS, BOMB_SETUP, 600),
        EDGE(1500, BSP_BTN_DOWN, BSP_BTN_LONG, BOMB_SETUP, 300),
        EDGE(1550, BSP_BTN_DOWN, BSP_BTN_RELEASE, BOMB_SETUP, 300),
        EDGE(1600, BSP_BTN_OK, BSP_BTN_PRESS, BOMB_COUNTDOWN, 300),
        EDGE(1620, BSP_BTN_OK, BSP_BTN_RELEASE, BOMB_COUNTDOWN, 300),
        EDGE(1650, BSP_BTN_OK, BSP_BTN_PRESS, BOMB_WIRES, 300),
        EDGE(1670, BSP_BTN_OK, BSP_BTN_RELEASE, BOMB_WIRES, 300),
        EDGE(1700, BSP_BTN_UP, BSP_BTN_PRESS, BOMB_EXPLODED, 0),
        EDGE(1720, BSP_BTN_UP, BSP_BTN_RELEASE, BOMB_EXPLODED, 0),
        EDGE(2200, BSP_BTN_OK, BSP_BTN_PRESS, BOMB_EXPLODED, 0),
        EDGE(2220, BSP_BTN_OK, BSP_BTN_RELEASE, BOMB_EXPLODED, 0),
        EDGE(3499, -1, BSP_BTN_PRESS, BOMB_EXPLODED, 0),
        EDGE(4100, -1, BSP_BTN_PRESS, BOMB_SETUP, 300),
        EDGE(4150, BSP_BTN_OK, BSP_BTN_PRESS, BOMB_COUNTDOWN, 300),
        EDGE(4170, BSP_BTN_OK, BSP_BTN_RELEASE, BOMB_COUNTDOWN, 300),
        EDGE(4200, BSP_BTN_OK, BSP_BTN_PRESS, BOMB_WIRES, 300),
        EDGE(4220, BSP_BTN_OK, BSP_BTN_RELEASE, BOMB_WIRES, 300),
        EDGE(4250, BSP_BTN_OK, BSP_BTN_PRESS, BOMB_SUCCESS, 0),
        EDGE(4270, BSP_BTN_OK, BSP_BTN_RELEASE, BOMB_SUCCESS, 0),
        EDGE(6050, -1, BSP_BTN_PRESS, BOMB_SETUP, 300),
        EDGE(6100, BSP_BTN_OK, BSP_BTN_PRESS, BOMB_COUNTDOWN, 300),
        EDGE(6120, BSP_BTN_OK, BSP_BTN_RELEASE, BOMB_COUNTDOWN, 300),
        {306099, -1, true, BOMB_COUNTDOWN, 1, 0, true, 65, BSP_BTN_PRESS},
        EDGE(306100, BSP_BTN_OK, BSP_BTN_PRESS, BOMB_EXPLODED, 0),
        EDGE(306120, BSP_BTN_OK, BSP_BTN_RELEASE, BOMB_EXPLODED, 0),
        EDGE(308500, -1, BSP_BTN_PRESS, BOMB_SETUP, 300),
        {338501, -1, true, BOMB_SETUP, 300, 0, false, 15, BSP_BTN_PRESS},
        EDGE(338550, BSP_BTN_UP, BSP_BTN_PRESS, BOMB_SETUP, 300),
        EDGE(338600, BSP_BTN_UP, BSP_BTN_RELEASE, BOMB_SETUP, 360),
        {501200, BSP_BTN_DOWN, true, BOMB_SETUP, 360, 501, false, 15, BSP_BTN_PRESS},
        // A release without a fresh press cannot apply a late adjustment.
        EDGE(501220, BSP_BTN_DOWN, BSP_BTN_RELEASE, BOMB_SETUP, 360),
    };
    steps = script; step_count = sizeof(script) / sizeof(script[0]);
    reset();
    if (setjmp(done) == 0) app_main();
    assert(step_index == step_count && renders == step_count - 1 && random_calls == 4);
    assert(explosions == 2); // Wrong wire and expiry once each; success and repeated frames stay silent.
    reset(); sound_fail = true;
    if (setjmp(done) == 0) app_main();
    assert(step_index == step_count && renders == step_count - 1 && explosions == 2);
    reset(); display_fail = true; app_main(); assert(created == 0 && !s_accept_keys);
    reset(); font_fail = true; app_main(); assert(created == 0 && !s_accept_keys);
    reset(); button_fail = true; app_main(); assert(created == 1 && deleted == 1 && ui_errors == 1 && !s_accept_keys);
    reset(); queue_fail = true; app_main(); assert(ui_errors == 1 && callback == NULL && !s_accept_keys);
    reset();
    puts("Bomb app loop: real callback/queue wiring, short/long/release edges, ignored notifications, startup, RNG retry, win/loss/expiry/replay, dimming, stale input and init failures PASS");
}
