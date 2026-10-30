#include "bomb_ui.h"
#include "bomb_text.h"
#include "lvgl.h"
#include <stdio.h>

LV_FONT_DECLARE(bomb_font_18);
#define INK 0xEDF2F7
#define MUTED 0x8794A8
#define AMBER 0xFFB547
#define RED 0xFF5B53
#define GREEN 0x5BE0A0
#define BACKGROUND 0x0B1019

static lv_obj_t *s_screen;
static lv_obj_t *s_clock[3];
static lv_obj_t *s_battery;
static lv_obj_t *s_progress;
static lv_obj_t *s_rings[3];
static lv_obj_t *s_shards[12];
static int s_state = -1;
static uint32_t s_remaining_ms = UINT32_MAX;
static int s_battery_value = -999;
static unsigned s_frame = UINT32_MAX;

static lv_obj_t *panel(lv_obj_t *parent, int x, int y, int w, int h, uint32_t color, int radius) {
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_style_bg_color(obj, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(obj, radius, 0);
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    return obj;
}

static lv_obj_t *text(lv_obj_t *parent, const char *value, int x, int y, int width,
                      const lv_font_t *font, uint32_t color, lv_text_align_t align) {
    lv_obj_t *label = lv_label_create(parent);
    lv_obj_remove_style_all(label);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_width(label, width);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_obj_set_style_text_align(label, align, 0);
    lv_label_set_text(label, value);
    return label;
}

static void chinese(const char *value, int y, uint32_t color) {
    text(s_screen, value, 16, y, 208, &bomb_font_18, color, LV_TEXT_ALIGN_CENTER);
}

static void header(bool show_title) {
    if (show_title) text(s_screen, "BOMB LAB", 20, 19, 140, &lv_font_montserrat_14, MUTED, LV_TEXT_ALIGN_LEFT);
    s_battery = text(s_screen, "--%", 166, 19, 54, &lv_font_montserrat_14, MUTED, LV_TEXT_ALIGN_RIGHT);
    s_battery_value = -999;
}

static void timer_panel(int y) {
    // Three equal cells and two equal separator gaps, centered on the 240 px panel.
    // Each cell also fits the three-digit maximum minute value at the same font size.
    enum { LEFT = 6, CELL = 68, GAP = 12, STRIDE = CELL + GAP, WIDTH = 3 * CELL + 2 * GAP };
    panel(s_screen, LEFT, y, WIDTH, 72, 0x151E2B, 14);
    for (unsigned i = 0; i < 3; ++i) {
        s_clock[i] = text(s_screen, "00", LEFT + (int)i * STRIDE, y + 12, CELL,
                          &lv_font_montserrat_40, i == 2 ? AMBER : INK, LV_TEXT_ALIGN_CENTER);
    }
    for (unsigned i = 0; i < 2; ++i)
        text(s_screen, ":", LEFT + CELL + (int)i * STRIDE, y + 12, GAP,
             &lv_font_montserrat_40, MUTED, LV_TEXT_ALIGN_CENTER);
    s_remaining_ms = UINT32_MAX;
}

static void build_page(bomb_state_t state) {
    lv_obj_clean(s_screen);
    lv_obj_set_style_bg_color(s_screen, lv_color_hex(BACKGROUND), 0);
    for (unsigned i = 0; i < 3; ++i) s_clock[i] = NULL;
    s_progress = NULL;
    s_frame = UINT32_MAX;
    header(state != BOMB_SETUP);
    switch (state) {
    case BOMB_SETUP:
        chinese(BOMB_TEXT_TITLE, 47, INK);
        timer_panel(79);
        panel(s_screen, 18, 175, 98, 48, 0x1C2939, 10);
        panel(s_screen, 124, 175, 98, 48, 0x1C2939, 10);
        text(s_screen, "+", 19, 178, 96, &lv_font_montserrat_40, INK, LV_TEXT_ALIGN_CENTER);
        text(s_screen, "-", 125, 178, 96, &lv_font_montserrat_40, INK, LV_TEXT_ALIGN_CENTER);
        panel(s_screen, 18, 243, 204, 52, 0x291B24, 12);
        text(s_screen, LV_SYMBOL_PLAY, 18, 251, 204, &lv_font_montserrat_32, RED, LV_TEXT_ALIGN_CENTER);
        break;
    case BOMB_COUNTDOWN:
        chinese(BOMB_TEXT_RUNNING, 61, AMBER);
        timer_panel(104);
        panel(s_screen, 18, 201, 204, 5, 0x273448, 2);
        s_progress = panel(s_screen, 18, 201, 204, 5, AMBER, 2);
        text(s_screen, "ARMED", 20, 227, 200, &lv_font_montserrat_20, AMBER, LV_TEXT_ALIGN_CENTER);
        panel(s_screen, 18, 265, 204, 34, 0x293244, 10);
        chinese(BOMB_TEXT_DISARM, 266, INK);
        break;
    case BOMB_WIRES: {
        chinese(BOMB_TEXT_CHOOSE, 54, INK);
        timer_panel(84);
        const char *names[] = {BOMB_TEXT_RED, BOMB_TEXT_YELLOW, BOMB_TEXT_BLUE};
        const uint32_t colors[] = {RED, 0xFFD65A, 0x58A6FF};
        for (unsigned i = 0; i < 3; ++i) {
            int y = 163 + (int)i * 47;
            panel(s_screen, 18, y, 204, 37, 0x172130, 9);
            panel(s_screen, 28, y + 15, 48, 7, colors[i], 3);
            panel(s_screen, 27, y + 11, 9, 15, colors[i], 3);
            panel(s_screen, 68, y + 11, 9, 15, colors[i], 3);
            text(s_screen, names[i], 87, y + 4, 126, &bomb_font_18, INK, LV_TEXT_ALIGN_CENTER);
        }
        break;
    }
    case BOMB_SUCCESS: {
        lv_obj_t *ring = panel(s_screen, 63, 75, 114, 114, 0x102D25, LV_RADIUS_CIRCLE);
        lv_obj_set_style_border_color(ring, lv_color_hex(GREEN), 0);
        lv_obj_set_style_border_width(ring, 3, 0);
        text(s_screen, "OK", 20, 102, 200, &lv_font_montserrat_48, GREEN, LV_TEXT_ALIGN_CENTER);
        chinese(BOMB_TEXT_SUCCESS, 208, GREEN);
        chinese(BOMB_TEXT_RETURN, 263, MUTED);
        break;
    }
    case BOMB_EXPLODED:
        for (unsigned i = 0; i < 3; ++i) {
            s_rings[i] = panel(s_screen, 100, 140, 40, 40, BACKGROUND, LV_RADIUS_CIRCLE);
            lv_obj_set_style_bg_opa(s_rings[i], LV_OPA_TRANSP, 0);
            lv_obj_set_style_border_color(s_rings[i], lv_color_hex(i == 1 ? AMBER : RED), 0);
            lv_obj_set_style_border_width(s_rings[i], 4, 0);
        }
        for (unsigned i = 0; i < 12; ++i) s_shards[i] = panel(s_screen, 110, 150, 7, 13, i % 2 ? AMBER : RED, 2);
        text(s_screen, "BOOM", 10, 124, 220, &lv_font_montserrat_48, INK, LV_TEXT_ALIGN_CENTER);
        panel(s_screen, 82, 190, 76, 28, BACKGROUND, 6);
        chinese(BOMB_TEXT_BOOM, 191, INK);
        chinese(BOMB_TEXT_RETURN, 267, MUTED);
        break;
    }
}

static bool font_coverage(void) {
    const char *all[] = {BOMB_TEXT_TITLE, BOMB_TEXT_SETUP, BOMB_TEXT_ADD, BOMB_TEXT_SUBTRACT,
        BOMB_TEXT_START, BOMB_TEXT_RUNNING, BOMB_TEXT_DISARM, BOMB_TEXT_CHOOSE, BOMB_TEXT_RED,
        BOMB_TEXT_YELLOW, BOMB_TEXT_BLUE, BOMB_TEXT_SUCCESS, BOMB_TEXT_SAFE, BOMB_TEXT_BOOM,
        BOMB_TEXT_RETURN, BOMB_TEXT_ERROR, BOMB_TEXT_REBOOT, BOMB_TEXT_HOLD};
    for (unsigned i = 0; i < sizeof(all) / sizeof(all[0]); ++i) {
        const unsigned char *p = (const unsigned char *)all[i];
        while (*p) {
            uint32_t cp = *p++;
            if (cp >= 0xe0) { cp = ((cp & 15u) << 12) | ((p[0] & 63u) << 6) | (p[1] & 63u); p += 2; }
            else if (cp >= 0xc0) { cp = ((cp & 31u) << 6) | (p[0] & 63u); ++p; }
            lv_font_glyph_dsc_t glyph = {0};
            if (!lv_font_get_glyph_dsc(&bomb_font_18, &glyph, cp, 0) || glyph.is_placeholder) return false;
        }
    }
    lv_font_glyph_dsc_t missing = {0};
    return !lv_font_get_glyph_dsc(&bomb_font_18, &missing, 0x9f98, 0) || missing.is_placeholder;
}

bool bomb_ui_init(void) {
    s_screen = lv_obj_create(NULL);
    lv_obj_remove_style_all(s_screen);
    lv_obj_set_style_bg_opa(s_screen, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(s_screen, lv_color_hex(BACKGROUND), 0);
    lv_obj_remove_flag(s_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_screen_load(s_screen);
    s_state = -1;
    if (!font_coverage()) { bomb_ui_error("FONT ERROR"); return false; }
    return true;
}

void bomb_ui_error(const char *detail) {
    lv_obj_clean(s_screen);
    text(s_screen, "DEVICE ERROR", 16, 112, 208, &lv_font_montserrat_20, RED, LV_TEXT_ALIGN_CENTER);
    text(s_screen, detail, 16, 160, 208, &lv_font_montserrat_14, INK, LV_TEXT_ALIGN_CENTER);
    text(s_screen, "Please restart", 16, 212, 208, &lv_font_montserrat_14, MUTED, LV_TEXT_ALIGN_CENTER);
    s_state = -1;
}

void bomb_ui_render(const bomb_game_t *game, uint64_t now_ms, int battery_percent) {
    if (s_state != (int)game->state) {
        s_state = (int)game->state;
        build_page(game->state);
    }
    if (s_battery_value != battery_percent) {
        s_battery_value = battery_percent;
        if (battery_percent < 0) lv_label_set_text(s_battery, "--%");
        else lv_label_set_text_fmt(s_battery, "%d%%", battery_percent);
    }
    const uint32_t remaining = bomb_game_remaining_ms(game, now_ms);
    if (s_clock[0] && remaining != s_remaining_ms) {
        const uint32_t values[] = {remaining / 60000u, (remaining / 1000u) % 60u, (remaining % 1000u) / 10u};
        const uint32_t previous[] = {s_remaining_ms / 60000u, (s_remaining_ms / 1000u) % 60u, (s_remaining_ms % 1000u) / 10u};
        for (unsigned i = 0; i < 3; ++i) {
            if (s_remaining_ms == UINT32_MAX || values[i] != previous[i])
                lv_label_set_text_fmt(s_clock[i], "%02lu", (unsigned long)values[i]);
            lv_obj_set_style_text_color(s_clock[i], lv_color_hex(
                game->state != BOMB_SETUP && remaining <= 30000 ? RED : i == 2 ? AMBER : INK), 0);
        }
        s_remaining_ms = remaining;
        if (s_progress) lv_obj_set_width(s_progress,
            (int)((uint64_t)204 * remaining / ((uint64_t)game->configured_seconds * 1000u)));
    }
    if (game->state == BOMB_EXPLODED) {
        const unsigned frame = (unsigned)((now_ms - game->result_started_ms) / 50);
        if (frame == s_frame) return;
        s_frame = frame;
        lv_obj_set_style_bg_color(s_screen, lv_color_hex(frame < 2 ? 0xFFB547 : BACKGROUND), 0);
        for (unsigned i = 0; i < 3; ++i) {
            int radius = 18 + (int)((frame * 7 + i * 27) % 126);
            lv_obj_set_size(s_rings[i], radius * 2, radius * 2);
            lv_obj_set_pos(s_rings[i], 120 - radius, 160 - radius);
        }
        static const int16_t dx[] = {1000,866,500,0,-500,-866,-1000,-866,-500,0,500,866};
        static const int16_t dy[] = {0,500,866,1000,866,500,0,-500,-866,-1000,-866,-500};
        int distance = 30 + (int)(frame * 4);
        for (unsigned i = 0; i < 12; ++i) {
            lv_obj_set_pos(s_shards[i], 117 + dx[i] * distance / 1000, 154 + dy[i] * distance / 1000);
        }
    }
}
