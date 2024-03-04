#include <beet_shared/platform_defines.h>

#if PLATFORM_LINUX

#include <beet_core/window.h>
#include <beet_core/input_types.h>

//===internal structs========
static struct WindowInfo {
    const wchar_t *applicationName;
    const wchar_t *titleName;

    int32_t width;
    int32_t height;
    int32_t posX;
    int32_t posY;
    bool shouldWindowClose;

    CursorState currentCursorState;
    vec2i lockedCursorPosition;
    vec2i virtualCursorPosition;
    bool cursorOverWindow;
} g_windowInfo = {};

//===internal functions======

void update_cursor_state() {}

void window_poll() {}

//===api=====================
void window_update() {
    window_poll();
    update_cursor_state();
}

bool window_is_open() {
    return !g_windowInfo.shouldWindowClose;
}

bool window_is_cursor_over_window() {
    return g_windowInfo.cursorOverWindow;
}

void window_set_cursor_lock_position(const vec2i lockPos) {
    g_windowInfo.lockedCursorPosition = vec2i{
            g_windowInfo.posX + lockPos.x,
            g_windowInfo.posY + lockPos.y
    };
}

void window_set_cursor(CursorState state) {

}

void *window_get_handle() {
    return nullptr;
}

//===init & shutdown=========
void
window_create(const char windowTitle[MAX_WINDOW_TITLE_SIZE], const vec2i &windowSize, const vec2i &windowPosition) {}

void window_cleanup() {
    g_windowInfo = {};
}

#endif