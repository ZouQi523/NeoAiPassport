idf_component_register(
    SRCS "main.c" "bomb_game.c" "bomb_ui.c" "bomb_sound.c" "bomb_sound_asset.c" "../assets/fonts/bomb_font_18.c"
    INCLUDE_DIRS "."
    REQUIRES bsp esp_hw_support esp_timer
    EMBED_FILES "../assets/music/bomb-explosion.pcm"
)
