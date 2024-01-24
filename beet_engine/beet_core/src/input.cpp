#include <beet_core/input.h>

//===internal structs========
struct KeyInfo {
    bool keyDown{};
    float timePressed{};
    float timeReleased{};
};

struct MouseInfo {
    vec2i windowedPosition{};

    vec2i virtualPosition{};
    vec2i lastVirtualPosition{};
    vec2f moveDelta{};
    vec2f mouseSensitivity{};

    int32_t currentScrollPosition{};
    int32_t lastScrollPosition{};
    float scrollDelta{};
    float scrollSensitivity{};
};

static struct Input {
    KeyInfo keyboard[(size_t) KeyCode::Last]{};
    MouseInfo mouse{};
    KeyInfo mouseKeys[(size_t) MouseButton::Last]{};
    float cachedCurrentTime{};
} g_input = {};

//===internal functions======
void update_relative_mouse_position() {
    MouseInfo *mouseInfo = &g_input.mouse;
    mouseInfo->moveDelta.x = (float) mouseInfo->virtualPosition.x - (float) mouseInfo->lastVirtualPosition.x;
    mouseInfo->moveDelta.y = (float) mouseInfo->virtualPosition.y - (float) mouseInfo->lastVirtualPosition.y;
    mouseInfo->lastVirtualPosition = mouseInfo->virtualPosition;
}

void update_mouse_scroll() {
    MouseInfo *mouseInfo = &g_input.mouse;
    mouseInfo->scrollDelta = (float) mouseInfo->currentScrollPosition - (float) mouseInfo->lastScrollPosition;
    mouseInfo->lastScrollPosition = mouseInfo->currentScrollPosition;
}

//===api=====================
void input_update() {
    update_relative_mouse_position();
    update_mouse_scroll();
}

void input_set_time(const double time) {
    g_input.cachedCurrentTime = (float) time;
}

bool input_key_pressed(const KeyCode key) {
    const KeyInfo *keyInfo = &g_input.keyboard[(size_t) key];
    return keyInfo->timePressed == g_input.cachedCurrentTime;
}

bool input_key_released(const KeyCode key) {
    const KeyInfo *keyInfo = &g_input.keyboard[(size_t) key];
    return keyInfo->timeReleased == g_input.cachedCurrentTime;
}

bool input_key_down(const KeyCode key) {
    return g_input.keyboard[(size_t) key].keyDown;
}

float input_key_down_time(const KeyCode key) {
    const KeyInfo *keyInfo = &g_input.keyboard[(size_t) key];
    if (keyInfo->timePressed > keyInfo->timeReleased) {
        return g_input.cachedCurrentTime - keyInfo->timePressed;
    }
    return 0.0f;
}

bool input_mouse_pressed(const MouseButton button) {
    const KeyInfo *keyInfo = &g_input.mouseKeys[(size_t) button];
    return keyInfo->timePressed == g_input.cachedCurrentTime;
}

bool input_mouse_released(const MouseButton button) {
    const KeyInfo *keyInfo = &g_input.mouseKeys[(size_t) button];
    return keyInfo->timeReleased == g_input.cachedCurrentTime;
}

bool input_mouse_down(const MouseButton button) {
    return g_input.mouseKeys[(size_t) button].keyDown;
}

float input_mouse_down_time(const MouseButton button) {
    const KeyInfo *keyInfo = &g_input.mouseKeys[(size_t) button];
    if (keyInfo->timePressed > keyInfo->timeReleased) {
        return g_input.cachedCurrentTime - keyInfo->timePressed;
    }
    return 0.0f;
}

float input_mouse_scroll_delta_raw() {
    return (float) g_input.mouse.scrollDelta;
}

float input_mouse_scroll_delta() {
    return (float) g_input.mouse.scrollDelta * g_input.mouse.scrollSensitivity;
}

vec2f input_mouse_delta() {
    vec2f out = g_input.mouse.moveDelta;
    out.x *= g_input.mouse.mouseSensitivity.x;
    out.y *= g_input.mouse.mouseSensitivity.y;
    return out;
}

vec2f input_mouse_delta_raw() {
    return g_input.mouse.moveDelta;
}

vec2i input_mouse_position() {
    return g_input.mouse.windowedPosition;
}

void input_set_mouse_sensitivity(const vec2f sensitivity) {
    g_input.mouse.mouseSensitivity = sensitivity;
}

void input_set_scroll_sensitivity(const float sensitivity) {
    g_input.mouse.scrollSensitivity = sensitivity;
}

//===init & shutdown=========
void input_create() {
    g_input = {};

    input_set_mouse_sensitivity({0.3f, 0.3f});
    input_set_scroll_sensitivity(0.03f);
}

void input_cleanup() {
    g_input = {};
}

void input_key_down_callback(const int32_t keyCode) {
    KeyInfo *keyInfo = &g_input.keyboard[keyCode];
    if (!keyInfo->keyDown) {
        keyInfo->keyDown = true;
        keyInfo->timePressed = g_input.cachedCurrentTime;
    }
}

void input_key_up_callback(const int32_t keyCode) {
    KeyInfo *keyInfo = &g_input.keyboard[keyCode];
    keyInfo->keyDown = false;
    keyInfo->timeReleased = g_input.cachedCurrentTime;
}

void input_mouse_down_callback(const int32_t keyCode) {
    KeyInfo *keyInfo = &g_input.mouseKeys[keyCode];
    if (!keyInfo->keyDown) {
        keyInfo->keyDown = true;
        keyInfo->timePressed = g_input.cachedCurrentTime;
    }
}

void input_mouse_up_callback(const int32_t keyCode) {
    KeyInfo *keyInfo = &g_input.mouseKeys[keyCode];
    keyInfo->keyDown = false;
    keyInfo->timeReleased = g_input.cachedCurrentTime;
}

void input_mouse_move_callback(const int32_t x, const int32_t y) {
    MouseInfo *cursor = &g_input.mouse;
    cursor->virtualPosition.x = x;
    cursor->virtualPosition.y = y;
}

void input_mouse_windowed_position_callback(const int32_t x, const int32_t y) {
    MouseInfo *cursor = &g_input.mouse;
    cursor->windowedPosition.x = x;
    cursor->windowedPosition.y = y;
}

void input_mouse_scroll_callback(const int32_t y) {
    MouseInfo *cursor = &g_input.mouse;
    cursor->currentScrollPosition += y;
}