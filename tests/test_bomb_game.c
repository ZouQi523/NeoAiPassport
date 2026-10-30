#include "bomb_game.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    bomb_startup_guard_t guard = {0};
    for (unsigned i = 0; i < 1000; ++i) assert(!bomb_startup_guard_sample(&guard, false));
    assert(!bomb_startup_guard_sample(&guard, true));
    assert(!bomb_startup_guard_sample(&guard, false)); // A transient gap is not release.
    assert(!bomb_startup_guard_sample(&guard, true));
    assert(bomb_startup_guard_sample(&guard, true));
    assert(bomb_startup_guard_sample(&guard, false)); // Later presses remain usable.
    bomb_game_t g;
    bomb_game_init(&g);
    assert(g.state == BOMB_SETUP && g.configured_seconds == 300);
    bomb_game_press(&g, BOMB_KEY_DOWN, 0, 0);
    assert(g.configured_seconds == 240);
    bomb_game_press(&g, BOMB_KEY_UP, 0, 0);
    assert(g.configured_seconds == 300);
    bomb_game_press(&g, BOMB_KEY_DOWN, 0, 0);
    assert(g.configured_seconds == 240);
    for (int i = 0; i < 200; ++i) bomb_game_press(&g, BOMB_KEY_UP, 0, 0);
    assert(g.configured_seconds == BOMB_MAX_SECONDS);
    for (int i = 0; i < 200; ++i) bomb_game_press(&g, BOMB_KEY_DOWN, 0, 0);
    assert(g.configured_seconds == BOMB_MIN_SECONDS);

    assert(g.configured_seconds == 60);
    bomb_game_long_press(&g, BOMB_KEY_DOWN);
    assert(g.configured_seconds == 60);
    bomb_game_long_press(&g, BOMB_KEY_UP);
    assert(g.configured_seconds == 360);
    bomb_game_long_press(&g, BOMB_KEY_DOWN);
    assert(g.configured_seconds == 60);
    g.configured_seconds = BOMB_MAX_SECONDS - 60;
    bomb_game_long_press(&g, BOMB_KEY_UP);
    assert(g.configured_seconds == BOMB_MAX_SECONDS);
    g.configured_seconds = 120;
    bomb_game_long_press(&g, BOMB_KEY_DOWN);
    assert(g.configured_seconds == BOMB_MIN_SECONDS);
    bomb_game_init(&g);
    bomb_game_press(&g, BOMB_KEY_OK, 1234, 3); // Invalid randomness must not start.
    assert(g.state == BOMB_SETUP);
    bomb_game_press(&g, BOMB_KEY_OK, 1234, 1);
    assert(g.deadline_ms == 301234 && g.state == BOMB_COUNTDOWN);
    assert(bomb_game_remaining(&g, 1234) == 300);
    assert(bomb_game_remaining(&g, 1235) == 300);
    assert(bomb_game_remaining(&g, 2234) == 299);
    assert(bomb_game_remaining_ms(&g, 1234) == 300000);
    assert(bomb_game_remaining_ms(&g, 1235) == 299999);
    assert(bomb_game_remaining_ms(&g, 2233) == 299001);
    assert(bomb_game_remaining_ms(&g, 301233) == 1);
    assert(bomb_game_remaining_ms(&g, 301234) == 0);
    bomb_game_long_press(&g, BOMB_KEY_DOWN);
    assert(g.configured_seconds == 300);
    bomb_game_tick(&g, 123456); // A delayed scheduler must not slow the countdown.
    assert(bomb_game_remaining(&g, 123456) == 178);
    bomb_game_press(&g, BOMB_KEY_UP, 123456, 0);
    assert(g.configured_seconds == 300 && g.deadline_ms == 301234);
    bomb_game_press(&g, BOMB_KEY_OK, 123456, 0);
    assert(g.state == BOMB_WIRES && g.correct_wire == 1); // Entry is not a cut.
    bomb_game_tick(&g, 300235);
    assert(bomb_game_remaining(&g, 300235) == 1);
    bomb_game_press(&g, BOMB_KEY_DOWN, 301234, 1); // Deadline wins even with correct wire.
    assert(g.state == BOMB_EXPLODED);
    bomb_game_press(&g, BOMB_KEY_OK, 301235, 0);
    assert(g.state == BOMB_EXPLODED);
    bomb_game_press(&g, BOMB_KEY_OK, 301234 + BOMB_EXPLOSION_MS, 0);
    assert(g.state == BOMB_SETUP); // Transition press is consumed, never starts again.

    for (unsigned correct = 0; correct < 3; ++correct) {
        unsigned wins = 0;
        for (unsigned chosen = 0; chosen < 3; ++chosen) {
            bomb_game_init(&g);
            bomb_game_press(&g, BOMB_KEY_OK, 0, (uint8_t)correct);
            bomb_game_press(&g, BOMB_KEY_OK, 100, 2);
            assert(g.state == BOMB_WIRES && g.correct_wire == correct);
            bomb_game_press(&g, (bomb_key_t)chosen, 200, 2);
            if (g.state == BOMB_SUCCESS) wins++;
            else assert(g.state == BOMB_EXPLODED);
            const unsigned result_ms = g.state == BOMB_EXPLODED ? BOMB_EXPLOSION_MS : BOMB_RESULT_MS;
            bomb_game_tick(&g, 200 + result_ms - 1);
            assert(g.state != BOMB_SETUP);
            bomb_game_tick(&g, 200 + result_ms);
            assert(g.state == BOMB_SETUP);
        }
        assert(wins == 1); // Exactly one of the three keys wins for every round.
    }
    bomb_game_init(&g);
    bomb_game_press(&g, BOMB_KEY_UP, 0, 0);
    bomb_game_press(&g, BOMB_KEY_OK, 5000, 0);
    bomb_game_tick(&g, 605000); // Expiry also works outside the wire page.
    assert(g.state == BOMB_EXPLODED);
    bomb_game_tick(&g, 607400);
    assert(g.state == BOMB_SETUP && g.configured_seconds == 360);
    bomb_input_t input = {0};
    for (unsigned key = 0; key < 2; ++key) {
        assert(bomb_input_event(&input, BOMB_SETUP, key, BOMB_INPUT_RELEASE) == BOMB_ACTION_NONE);
        assert(bomb_input_event(&input, BOMB_SETUP, key, BOMB_INPUT_PRESS) == BOMB_ACTION_NONE);
        assert(bomb_input_event(&input, BOMB_SETUP, key, BOMB_INPUT_PRESS) == BOMB_ACTION_NONE);
        assert(bomb_input_event(&input, BOMB_SETUP, key, BOMB_INPUT_RELEASE) == BOMB_ACTION_SHORT);
        assert(bomb_input_event(&input, BOMB_SETUP, key, BOMB_INPUT_LONG) == BOMB_ACTION_NONE);
        assert(bomb_input_event(&input, BOMB_SETUP, key, BOMB_INPUT_PRESS) == BOMB_ACTION_NONE);
        assert(bomb_input_event(&input, BOMB_SETUP, key, BOMB_INPUT_LONG) == BOMB_ACTION_LONG);
        assert(bomb_input_event(&input, BOMB_SETUP, key, BOMB_INPUT_LONG) == BOMB_ACTION_NONE);
        assert(bomb_input_event(&input, BOMB_SETUP, key, BOMB_INPUT_RELEASE) == BOMB_ACTION_NONE);
        // A delayed release after a page change cannot adjust the next page.
        assert(bomb_input_event(&input, BOMB_SETUP, key, BOMB_INPUT_PRESS) == BOMB_ACTION_NONE);
        assert(bomb_input_event(&input, BOMB_COUNTDOWN, key, BOMB_INPUT_RELEASE) == BOMB_ACTION_NONE);
        assert(bomb_input_event(&input, BOMB_SETUP, key, BOMB_INPUT_PRESS) == BOMB_ACTION_NONE);
        bomb_input_cancel(&input, key);
        assert(bomb_input_event(&input, BOMB_SETUP, key, BOMB_INPUT_RELEASE) == BOMB_ACTION_NONE);
    }
    assert(bomb_input_event(&input, BOMB_SETUP, BOMB_KEY_OK, BOMB_INPUT_PRESS) == BOMB_ACTION_SHORT);
    assert(bomb_input_event(&input, BOMB_COUNTDOWN, BOMB_KEY_OK, BOMB_INPUT_LONG) == BOMB_ACTION_NONE);
    assert(bomb_input_event(&input, BOMB_COUNTDOWN, BOMB_KEY_OK, BOMB_INPUT_RELEASE) == BOMB_ACTION_NONE);
    for (unsigned key = 0; key < 3; ++key) {
        assert(bomb_input_event(&input, BOMB_WIRES, key, BOMB_INPUT_PRESS) == BOMB_ACTION_SHORT);
        assert(bomb_input_event(&input, BOMB_WIRES, key, BOMB_INPUT_LONG) == BOMB_ACTION_NONE);
        assert(bomb_input_event(&input, BOMB_WIRES, key, BOMB_INPUT_RELEASE) == BOMB_ACTION_NONE);
    }
    unsigned counts[3] = {0}; uint8_t wire;
    assert(!bomb_wire_from_random(UINT32_MAX, &wire));
    assert(!bomb_wire_from_random(1, NULL));
    for (uint32_t n = 0; n < 3000; ++n) {
        assert(bomb_wire_from_random(n, &wire)); counts[wire]++;
    }
    assert(counts[0] == 1000 && counts[1] == 1000 && counts[2] == 1000);
    puts("Bomb game: limits, deadline timing, page entry, all 9 wire choices, expiry, replay, RNG mapping PASS");
}
