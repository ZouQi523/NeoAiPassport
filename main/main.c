// Countdown Bomb: one application loop owns state; BSP callbacks only enqueue events.
#include "bomb_game.h"
#include "bomb_ui.h"
#include "bomb_sound.h"
#include "bsp_battery.h"
#include "bsp_button.h"
#include "bsp_display.h"
#include "bsp_pins.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

static const char *TAG = "bomb";
static QueueHandle_t s_keys;
static volatile bool s_accept_keys;
static volatile uint32_t s_dropped;

typedef struct {
    bsp_btn_t key;
    bsp_btn_ev_t event;
    uint64_t at_ms;
} key_press_t;

static uint64_t now_ms(void) {
    return (uint64_t)esp_timer_get_time() / 1000u;
}

static void on_key(bsp_btn_t key, bsp_btn_ev_t event, void *user) {
    (void)user;
    if (!s_accept_keys || !s_keys ||
        (event != BSP_BTN_PRESS && event != BSP_BTN_LONG && event != BSP_BTN_RELEASE)) return;
    const key_press_t press = {.key = key, .event = event, .at_ms = now_ms()};
    if (xQueueSend(s_keys, &press, 0) != pdTRUE) ++s_dropped;
}

static uint8_t random_wire(void) {
    uint8_t wire;
    while (!bomb_wire_from_random(esp_random(), &wire)) { }
    return wire;
}

void app_main(void) {
    ESP_LOGI(TAG, "Countdown Bomb starting");
    if (bsp_display_init() != ESP_OK || !bsp_lvgl_init()) {
        ESP_LOGE(TAG, "Display unavailable; application stopped");
        return;
    }
    bsp_display_backlight(65);
    if (!bsp_lvgl_lock(1000)) return;
    const bool fonts_ok = bomb_ui_init();
    bsp_lvgl_unlock();
    if (!fonts_ok) {
        ESP_LOGE(TAG, "Chinese font coverage failed; controls disabled");
        return;
    }
    s_keys = xQueueCreate(24, sizeof(key_press_t));
    const bool keys_ok = s_keys && bsp_button_init(on_key, NULL) == ESP_OK;
    if (!keys_ok) {
        if (bsp_lvgl_lock(1000)) {
            bomb_ui_error("BUTTON ERROR");
            bsp_lvgl_unlock();
        }
        ESP_LOGE(TAG, "Button initialization failed");
        if (s_keys) { vQueueDelete(s_keys); s_keys = NULL; }
        return;
    }
    if (bomb_sound_init() != ESP_OK) ESP_LOGW(TAG, "Explosion sound unavailable; visual game remains enabled");
    const bool battery_ok = bsp_battery_init() == ESP_OK;
    bomb_game_t game;
    bomb_game_init(&game);
    bomb_input_t input = {0};
    uint64_t last_input = now_ms();
    uint64_t last_battery = 0;
    uint32_t last_dropped = 0;
    int battery = -1;
    int brightness = 65;
    bomb_startup_guard_t startup_guard = {0};
    ESP_LOGI(TAG, "Ready: UP/DOWN short +/-1 min, hold +/-5 min / OK start or disarm");
    for (;;) {
        const bomb_state_t previous_state = game.state;
        key_press_t press;
        const bool pressed = xQueueReceive(s_keys, &press, pdMS_TO_TICKS(20)) == pdTRUE;
        const uint64_t now = now_ms();
        // A key held at boot never starts a round; require two released samples.
        if (!s_accept_keys) {
            s_accept_keys = bomb_startup_guard_sample(&startup_guard,
                bsp_button_read_mv() >= BSP_BTN_MV_RELEASED_MIN);
        }
        if (pressed && (unsigned)press.key < 3) {
            bomb_key_t key = press.key == BSP_BTN_UP ? BOMB_KEY_UP :
                             press.key == BSP_BTN_DOWN ? BOMB_KEY_DOWN : BOMB_KEY_OK;
            if (now - press.at_ms > 500) {
                bomb_input_cancel(&input, key);
            } else {
                const bomb_input_event_t event = press.event == BSP_BTN_PRESS ? BOMB_INPUT_PRESS :
                    press.event == BSP_BTN_LONG ? BOMB_INPUT_LONG : BOMB_INPUT_RELEASE;
                const bomb_action_t action = bomb_input_event(&input, game.state, key, event);
                if (action == BOMB_ACTION_SHORT) {
                    const uint8_t wire = game.state == BOMB_SETUP && key == BOMB_KEY_OK ? random_wire() : 0;
                    bomb_game_press(&game, key, now, wire);
                } else if (action == BOMB_ACTION_LONG) {
                    bomb_game_long_press(&game, key);
                }
                last_input = now;
            }
        }
        bomb_game_tick(&game, now);
        if (game.state == BOMB_EXPLODED && previous_state != BOMB_EXPLODED)
            bomb_sound_explode(game.result_started_ms);
        if (last_battery == 0 || now - last_battery >= 10000) {
            battery = battery_ok ? bsp_battery_soc() : -1;
            last_battery = now;
            if (s_dropped != last_dropped) {
                last_dropped = s_dropped;
                input = (bomb_input_t){0};
                ESP_LOGW(TAG, "Input queue drops: %lu", (unsigned long)last_dropped);
            }
        }
        int target_brightness = game.state == BOMB_SETUP && now - last_input >= 30000 ? 15 : 65;
        if (target_brightness != brightness) {
            brightness = target_brightness;
            bsp_display_backlight((uint8_t)brightness);
        }
        if (bsp_lvgl_lock(100)) {
            bomb_ui_render(&game, now, battery);
            bsp_lvgl_unlock();
        }
    }
}
