#include "bomb_game.h"

void bomb_game_init(bomb_game_t *game) {
    *game = (bomb_game_t){ .state = BOMB_SETUP, .configured_seconds = BOMB_DEFAULT_SECONDS };
}

void bomb_game_tick(bomb_game_t *game, uint64_t now_ms) {
    if ((game->state == BOMB_COUNTDOWN || game->state == BOMB_WIRES) &&
        now_ms >= game->deadline_ms) {
        game->state = BOMB_EXPLODED;
        game->result_started_ms = now_ms;
    } else if ((game->state == BOMB_SUCCESS || game->state == BOMB_EXPLODED) &&
               now_ms - game->result_started_ms >=
                   (game->state == BOMB_EXPLODED ? BOMB_EXPLOSION_MS : BOMB_RESULT_MS)) {
        game->state = BOMB_SETUP;
    }
}

void bomb_game_press(bomb_game_t *game, bomb_key_t key, uint64_t now_ms, uint8_t correct_wire) {
    const bomb_state_t before_tick = game->state;
    bomb_game_tick(game, now_ms);
    // A press arriving on a result transition cannot accidentally start a new round.
    if (game->state != before_tick) return;
    switch (game->state) {
    case BOMB_SETUP:
        if (key == BOMB_KEY_UP && game->configured_seconds < BOMB_MAX_SECONDS) {
            game->configured_seconds += BOMB_STEP_SECONDS;
        } else if (key == BOMB_KEY_DOWN && game->configured_seconds > BOMB_MIN_SECONDS) {
            game->configured_seconds -= BOMB_STEP_SECONDS;
        } else if (key == BOMB_KEY_OK && correct_wire < 3) {
            game->correct_wire = correct_wire;
            game->deadline_ms = now_ms + (uint64_t)game->configured_seconds * 1000;
            game->state = BOMB_COUNTDOWN;
        }
        break;
    case BOMB_COUNTDOWN:
        if (key == BOMB_KEY_OK) game->state = BOMB_WIRES;
        break;
    case BOMB_WIRES:
        if ((unsigned)key < 3) {
            game->state = (unsigned)key == game->correct_wire ? BOMB_SUCCESS : BOMB_EXPLODED;
            game->result_started_ms = now_ms;
        }
        break;
    case BOMB_SUCCESS:
    case BOMB_EXPLODED:
        break;
    }
}

void bomb_game_long_press(bomb_game_t *game, bomb_key_t key) {
    if (game->state != BOMB_SETUP) return;
    if (key == BOMB_KEY_UP) {
        const uint32_t available = BOMB_MAX_SECONDS - game->configured_seconds;
        game->configured_seconds += available < BOMB_LONG_STEP_SECONDS ? available : BOMB_LONG_STEP_SECONDS;
    } else if (key == BOMB_KEY_DOWN) {
        const uint32_t available = game->configured_seconds - BOMB_MIN_SECONDS;
        game->configured_seconds -= available < BOMB_LONG_STEP_SECONDS ? available : BOMB_LONG_STEP_SECONDS;
    }
}

uint32_t bomb_game_remaining_ms(const bomb_game_t *game, uint64_t now_ms) {
    if (game->state == BOMB_SETUP) return game->configured_seconds * 1000u;
    if (game->state == BOMB_COUNTDOWN || game->state == BOMB_WIRES) {
        if (now_ms >= game->deadline_ms) return 0;
        return (uint32_t)(game->deadline_ms - now_ms);
    }
    return 0;
}

uint32_t bomb_game_remaining(const bomb_game_t *game, uint64_t now_ms) {
    return (bomb_game_remaining_ms(game, now_ms) + 999u) / 1000u;
}

void bomb_input_cancel(bomb_input_t *input, bomb_key_t key) {
    if ((unsigned)key < 3) input->keys[key] = (bomb_held_key_t){0};
}

bomb_action_t bomb_input_event(bomb_input_t *input, bomb_state_t state,
                              bomb_key_t key, bomb_input_event_t event) {
    if ((unsigned)key >= 3) return BOMB_ACTION_NONE;
    bomb_held_key_t *held = &input->keys[key];
    if (event == BOMB_INPUT_PRESS) {
        if (held->held) return BOMB_ACTION_NONE;
        *held = (bomb_held_key_t){.held = true, .adjustment = state == BOMB_SETUP && key != BOMB_KEY_OK};
        return held->adjustment ? BOMB_ACTION_NONE : BOMB_ACTION_SHORT;
    }
    if (!held->held) return BOMB_ACTION_NONE;
    if (event == BOMB_INPUT_LONG) {
        if (!held->adjustment || held->long_handled) return BOMB_ACTION_NONE;
        held->long_handled = true;
        return state == BOMB_SETUP ? BOMB_ACTION_LONG : BOMB_ACTION_NONE;
    }
    if (event == BOMB_INPUT_RELEASE) {
        const bool short_adjust = held->adjustment && !held->long_handled && state == BOMB_SETUP;
        bomb_input_cancel(input, key);
        return short_adjust ? BOMB_ACTION_SHORT : BOMB_ACTION_NONE;
    }
    return BOMB_ACTION_NONE;
}

bool bomb_wire_from_random(uint32_t value, uint8_t *wire) {
    if (value == UINT32_MAX || wire == 0) return false;
    *wire = (uint8_t)(value % 3u);
    return true;
}

bool bomb_startup_guard_sample(bomb_startup_guard_t *guard, bool released) {
    if (guard->ready) return true;
    guard->released_samples = released ? guard->released_samples + 1 : 0;
    if (guard->released_samples >= 2) guard->ready = true;
    return guard->ready;
}
