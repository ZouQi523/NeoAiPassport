#include "bomb_game.h"
#include "bomb_ui.h"
#include "lvgl.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint16_t draw[240 * 320];
static uint16_t frame[240 * 320];
static unsigned flushes;

static void clock_parts(const char *minutes, const char *seconds, const char *millis) {
    const char *parts[] = {minutes, seconds, millis};
    const int positions[] = {6, 86, 166};
    lv_obj_t *columns[3] = {0};
    for (unsigned part = 0; part < 3; ++part) {
        bool found = false;
        lv_obj_t *screen = lv_screen_active();
        for (unsigned i = 0; i < lv_obj_get_child_count(screen); ++i) {
            lv_obj_t *child = lv_obj_get_child(screen, i);
            if (!lv_obj_check_type(child, &lv_label_class) || lv_obj_get_x(child) != positions[part] ||
                strcmp(lv_label_get_text(child), parts[part])) continue;
            lv_point_t size;
            lv_text_get_size(&size, parts[part], lv_obj_get_style_text_font(child, 0),
                             0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
            assert(size.x <= lv_obj_get_width(child));
            assert(size.y == lv_obj_get_height(child)); // No wrapped or clipped timer digits.
            columns[part] = child;
            found = true;
        }
        assert(found);
    }
    const int width = lv_obj_get_width(columns[0]);
    int centers[3];
    for (unsigned i = 0; i < 3; ++i) {
        assert(lv_obj_get_width(columns[i]) == width);
        assert(lv_obj_get_style_text_align(columns[i], 0) == LV_TEXT_ALIGN_CENTER);
        centers[i] = lv_obj_get_x(columns[i]) * 2 + lv_obj_get_width(columns[i]);
    }
    assert(centers[1] - centers[0] == centers[2] - centers[1]);
    assert(centers[1] == 240); // Twice the middle column's center, exactly screen-centered.
    assert(lv_obj_get_x(columns[0]) == 240 - lv_obj_get_x(columns[2]) - width);
    unsigned separators = 0;
    lv_obj_t *screen = lv_screen_active();
    for (unsigned i = 0; i < lv_obj_get_child_count(screen); ++i) {
        lv_obj_t *child = lv_obj_get_child(screen, i);
        if (!lv_obj_check_type(child, &lv_label_class) || strcmp(lv_label_get_text(child), ":")) continue;
        assert(separators < 2);
        const int center = 2 * lv_obj_get_x(child) + lv_obj_get_width(child);
        assert(2 * center == centers[separators] + centers[separators + 1]);
        assert(lv_obj_get_style_text_align(child, 0) == LV_TEXT_ALIGN_CENTER);
        ++separators;
    }
    assert(separators == 2);
}

static void flush(lv_display_t *display, const lv_area_t *area, uint8_t *pixels) {
    int width = area->x2 - area->x1 + 1;
    for (int y = area->y1; y <= area->y2; ++y)
        memcpy(&frame[y * 240 + area->x1], pixels + (y - area->y1) * width * 2, (size_t)width * 2);
    ++flushes;
    lv_display_flush_ready(display);
}

static void snapshot(const char *directory, const char *name, const bomb_game_t *game, uint64_t now) {
    bomb_ui_render(game, now, 78);
    lv_obj_update_layout(lv_screen_active());
    lv_refr_now(NULL);
    assert(flushes > 0);
    char filename[1024];
    snprintf(filename, sizeof(filename), "%s/%s.ppm", directory, name);
    FILE *file = fopen(filename, "wb"); assert(file);
    fprintf(file, "P6\n240 320\n255\n");
    for (int y = 0; y < 320; ++y) {
        for (int x = 0; x < 240; ++x) {
            uint16_t value = frame[y * 240 + x];
            int cx = x < 30 ? 30 : x >= 210 ? 209 : x;
            int cy = y < 30 ? 30 : y >= 290 ? 289 : y;
            if ((x-cx)*(x-cx) + (y-cy)*(y-cy) > 900) value = 0;
            unsigned char rgb[] = {(unsigned char)(((value >> 11) & 31) * 255 / 31),
                (unsigned char)(((value >> 5) & 63) * 255 / 63), (unsigned char)((value & 31) * 255 / 31)};
            fwrite(rgb, 1, 3, file);
        }
    }
    fclose(file);
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    assert(argc == 2);
    lv_init();
    lv_display_t *display = lv_display_create(240, 320); assert(display);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(display, draw, NULL, sizeof(draw), LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(display, flush);
    assert(bomb_ui_init()); // Uses LVGL's real glyph lookup, including a missing-glyph control.
    bomb_game_t game;
    bomb_game_init(&game);
    snapshot(argv[1], "01-setup", &game, 0);
    clock_parts("05", "00", "00");
    game.configured_seconds = BOMB_MAX_SECONDS;
    snapshot(argv[1], "02-maximum", &game, 0);
    clock_parts("180", "00", "00");
    game.configured_seconds = BOMB_DEFAULT_SECONDS;
    bomb_game_press(&game, BOMB_KEY_OK, 0, 0);
    snapshot(argv[1], "03-countdown", &game, 28137);
    clock_parts("04", "31", "86");
    bomb_game_press(&game, BOMB_KEY_OK, 28140, 0);
    snapshot(argv[1], "04-wires", &game, 28157);
    clock_parts("04", "31", "84");
    // Borrow across a full minute must update every segment, including milliseconds.
    bomb_ui_render(&game, 1, 78);
    lv_obj_update_layout(lv_screen_active());
    clock_parts("04", "59", "99");
    bomb_ui_render(&game, 60000, 78);
    lv_obj_update_layout(lv_screen_active());
    clock_parts("04", "00", "00");
    bomb_ui_render(&game, 299999, 78);
    lv_obj_update_layout(lv_screen_active());
    clock_parts("00", "00", "00");
    bomb_ui_render(&game, 299990, 78);
    lv_obj_update_layout(lv_screen_active());
    clock_parts("00", "00", "01"); // Two fractional digits represent 10 ms each.
    bomb_game_press(&game, BOMB_KEY_UP, 28100, 0);
    snapshot(argv[1], "05-success", &game, 28100);
    bomb_game_init(&game);
    bomb_game_press(&game, BOMB_KEY_OK, 30000, 0);
    bomb_game_press(&game, BOMB_KEY_OK, 30100, 0);
    bomb_game_press(&game, BOMB_KEY_DOWN, 30200, 0);
    snapshot(argv[1], "06-explosion", &game, 30400);
    snapshot(argv[1], "07-explosion-late", &game, 30900);
    bomb_game_tick(&game, 32700);
    snapshot(argv[1], "08-replay", &game, 32700);
    // Exercise repeated page teardown/recreation in the same 32 KiB LVGL pool.
    for (unsigned i = 0; i < 100; ++i) {
        bomb_game_init(&game);
        bomb_ui_render(&game, 0, -1);
        bomb_game_press(&game, BOMB_KEY_OK, 0, 0);
        bomb_ui_render(&game, 0, -1);
        bomb_game_press(&game, BOMB_KEY_OK, 1, 0);
        bomb_ui_render(&game, 1, -1);
        bomb_game_press(&game, BOMB_KEY_DOWN, 2, 0);
        bomb_ui_render(&game, 2, -1);
    }
    puts("LVGL preview: glyph coverage, equal columns/center pitch/separators, hundredths/borrow/maximum width, all pages, 100 page cycles PASS");
    lv_mem_monitor_t memory;
    lv_mem_monitor(&memory);
    printf("LVGL pool: total=%lu, max_used=%lu, free=%lu, largest_free=%lu\n",
           (unsigned long)memory.total_size, (unsigned long)memory.max_used,
           (unsigned long)memory.free_size, (unsigned long)memory.free_biggest_size);
    lv_deinit();
    return 0;
}
