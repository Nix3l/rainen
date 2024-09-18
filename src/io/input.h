#ifndef INPUT_H
#define INPUT_H

#include "base.h"

typedef struct {
    bool keys[GLFW_KEY_LAST];
    bool keys_pressed[GLFW_KEY_LAST];
    bool buttons[GLFW_MOUSE_BUTTON_LAST];
    bool buttons_pressed[GLFW_MOUSE_BUTTON_LAST];
    v2f mouse_pos;
    v2f last_mouse_pos;
} input_state_s;

typedef struct {
    v2f pos; // position in the window
    v2f absolute_move; // movement in pixels
    v2f move; // normalised movement (dividing by width/height of window)
    v2f raw_move; // whether the mouse is moving or not in a direction WITHOUT the amount of movement
} mouse_state_s;

void init_input();
void update_input();

// NOTE(nix3l): these could technically be macros which might be a tiny bit faster
// although compiler optimisation might do all the effort for me so eh who cares

// return if a key is held
bool is_key_down(int key);
// return if a key is pressed. only returns true for one frame even if the user is holding
bool is_key_pressed(int key);
// return if a mouse button is held
bool is_button_down(int button);
// return if a mouse button is pressed. only returns true for one frame even if the user is holding
bool is_button_pressed(int button);

// position in the window
v2f get_mouse_pos();
// movement in pixels
v2f get_mouse_absolute_move();
// normalised movement (dividing by width/height of window)
v2f get_mouse_move();
// whether the mouse is moving or not in a direction WITHOUT the amount of movement
v2f get_mouse_raw_move();

mouse_state_s get_mouse_state();

#endif
