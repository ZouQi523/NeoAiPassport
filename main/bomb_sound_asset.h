#pragma once
#include <stddef.h>
#include <stdint.h>

// Read-only 16 kHz, signed 16-bit little-endian mono samples retained in Flash.
const uint8_t *bomb_sound_pcm(size_t *length);
