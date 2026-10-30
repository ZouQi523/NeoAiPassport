#include "bomb_sound_asset.h"
extern const uint8_t explosion_start[] __asm__("_binary_bomb_explosion_pcm_start");
extern const uint8_t explosion_end[] __asm__("_binary_bomb_explosion_pcm_end");

const uint8_t *bomb_sound_pcm(size_t *length) {
    *length = (size_t)(explosion_end - explosion_start);
    return explosion_start;
}
