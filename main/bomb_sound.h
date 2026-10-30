#pragma once
#include "esp_err.h"
#include <stdint.h>

// Call once from application startup. Only the worker owns codec I/O afterward.
esp_err_t bomb_sound_init(void);
// Non-blocking; called once on entry to the exploded state, never from rendering.
void bomb_sound_explode(uint64_t started_ms);
