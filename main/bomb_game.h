#pragma once

#include <stdbool.h>
#include <stdint.h>

#define BOMB_STEP_SECONDS 60u
#define BOMB_LONG_STEP_SECONDS 300u
#define BOMB_DEFAULT_SECONDS 300u
#define BOMB_MIN_SECONDS 60u
#define BOMB_MAX_SECONDS (180u * 60u)
#define BOMB_RESULT_MS 1800u
#define BOMB_EXPLOSION_MS 2400u

typedef enum {
    BOMB_SETUP, BOMB_COUNTDOWN, BOMB_WIRES, BOMB_SUCCESS, BOMB_EXPLODED
} bomb_state_t;

typedef enum { BOMB_KEY_UP, BOMB_KEY_DOWN, BOMB_KEY_OK } bomb_key_t;

typedef struct {
    bomb_state_t state;
    uint32_t configured_seconds;
    uint64_t deadline_ms;
    uint64_t result_started_ms;
    uint8_t correct_wire;
} bomb_game_t;

void bomb_game_init(bomb_game_t *game);
void bomb_game_tick(bomb_game_t *game, uint64_t now_ms);
// correct_wire is consumed only when starting a new round; it is never changed on the wire page.
void bomb_game_press(bomb_game_t *game, bomb_key_t key, uint64_t now_ms, uint8_t correct_wire);
void bomb_game_long_press(bomb_game_t *game, bomb_key_t key);
uint32_t bomb_game_remaining(const bomb_game_t *game, uint64_t now_ms);
uint32_t bomb_game_remaining_ms(const bomb_game_t *game, uint64_t now_ms);

typedef enum { BOMB_INPUT_PRESS, BOMB_INPUT_LONG, BOMB_INPUT_RELEASE } bomb_input_event_t;
typedef enum { BOMB_ACTION_NONE, BOMB_ACTION_SHORT, BOMB_ACTION_LONG } bomb_action_t;
typedef struct { bool held, adjustment, long_handled; } bomb_held_key_t;
typedef struct { bomb_held_key_t keys[3]; } bomb_input_t;
// Setup adjustment waits for RELEASE or LONG, never applies both. Other actions use PRESS.
bomb_action_t bomb_input_event(bomb_input_t *input, bomb_state_t state,
                              bomb_key_t key, bomb_input_event_t event);
void bomb_input_cancel(bomb_input_t *input, bomb_key_t key);
// Reject one value so all three outcomes have exactly the same number of 32-bit inputs.
bool bomb_wire_from_random(uint32_t value, uint8_t *wire);

typedef struct { unsigned released_samples; bool ready; } bomb_startup_guard_t;
// An unavailable/held ADC sample is false. Two released samples are required at boot.
bool bomb_startup_guard_sample(bomb_startup_guard_t *guard, bool released);
