#ifndef BEETROOT_INPUT_H
#define BEETROOT_INPUT_H

#include <beet_core/input_types.h>
#include <beet_math/vec2.h>

//===API================================================================================================================
void input_update();

void input_set_time(double time);
bool input_key_pressed(KeyCode key);
bool input_key_released(KeyCode key);
bool input_key_down(KeyCode key);
float input_key_down_time(KeyCode key);

vec2f input_mouse_delta();
vec2f input_mouse_delta_raw();
vec2i input_mouse_position();

bool input_mouse_down(MouseButton button);
bool input_mouse_pressed(MouseButton button);
bool input_mouse_released(MouseButton key);
float input_mouse_down_time(MouseButton button);

float input_mouse_scroll_delta_raw();
float input_mouse_scroll_delta();

void input_key_down_callback(int32_t asciiKeyCode);
void input_key_up_callback(int32_t asciiKeyCode);

void input_mouse_down_callback(int32_t keyCode);
void input_mouse_up_callback(int32_t keyCode);

void input_mouse_move_callback(int32_t x, int32_t y);
void input_mouse_scroll_callback(int32_t y);
void input_mouse_windowed_position_callback(int32_t x, int32_t y);
//======================================================================================================================

//===INIT_&_SHUTDOWN====================================================================================================
void input_create();
void input_cleanup();
//======================================================================================================================

#endif //BEETROOT_INPUT_H
