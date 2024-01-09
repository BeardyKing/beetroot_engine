#ifndef BEETROOT_WINDOW_H
#define BEETROOT_WINDOW_H

#include <beet_math/vec2.h>
#include <beet_core/input_types.h>

constexpr uint32_t MAX_WINDOW_TITLE_SIZE = 64;

//===api=====================
bool window_is_open();
void window_update();
void window_set_cursor(CursorState state);
void window_set_cursor_lock_position(vec2i lockPos);
bool window_is_cursor_over_window();
void* window_get_handle();

//===init & shutdown=========
void window_create(const char windowTitle[MAX_WINDOW_TITLE_SIZE], const vec2i &windowSize, const vec2i &windowPosition = {UINT32_MAX, UINT32_MAX});
void window_cleanup();

#endif //BEETROOT_WINDOW_H
