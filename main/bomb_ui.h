#pragma once
#include "bomb_game.h"
#include <stdbool.h>
#include <stdint.h>

// All calls belong to the LVGL thread or must hold the BSP LVGL lock.
bool bomb_ui_init(void);
void bomb_ui_render(const bomb_game_t *game, uint64_t now_ms, int battery_percent);
void bomb_ui_error(const char *detail);
