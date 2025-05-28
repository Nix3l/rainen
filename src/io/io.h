#ifndef _IO_H
#define _IO_H

#include "base.h"
#include "memory/memory.h"

void io_init();
void io_terminate();

// NOTE(nix3l): no detecting if monitor is connected/disconnected at runtime atm
typedef struct monitor_t {
    GLFWmonitor* glfw_monitor;

    i32 physical_width, physical_height; // in mm

    vector_t video_modes;
    GLFWvidmode* active_mode;

    bool primary;
    bool active;
} monitor_t;

void monitors_detect();

typedef struct window_t {
    GLFWwindow* glfw_window;
    monitor_t* monitor;

    char* name;
    u32 width, height;

    bool cursor_enabled;
    bool fullscreen;
    bool vsync;

    bool focused;
    bool minimized;
} window_t;

window_t window_new(monitor_t* monitor, u32 width, u32 height, char* name);

bool window_closing(window_t* window);

void window_enable_cursor(window_t* window, bool enable);
void window_set_fullscreen(window_t* window, monitor_t* monitor, bool enable);
void window_set_vsync(window_t* window, bool enable);
void window_set_size(window_t* window, u32 width, u32 height);

void window_destroy(window_t* window);

// TODO(nix3l):
//  => move the input state to a struct
//  => add more callbacks for window state
//  => add support for mouse scroll
//  => add support for gamepads

typedef struct io_ctx_t {
    bool initialized;

    vector_t monitors;
    monitor_t* active_monitor;
    monitor_t* primary_monitor;

    window_t window;

    bool keys[GLFW_KEY_LAST];
    bool keys_pressed[GLFW_KEY_LAST];
    bool keys_held[GLFW_KEY_LAST];
    bool keys_released[GLFW_KEY_LAST];
    bool mouse_buttons[GLFW_MOUSE_BUTTON_LEFT];
    bool mouse_buttons_pressed[GLFW_MOUSE_BUTTON_LEFT];
    bool mouse_buttons_held[GLFW_MOUSE_BUTTON_LEFT];
    bool mouse_buttons_released[GLFW_MOUSE_BUTTON_LEFT];

    v2f mouse_pos;
    v2f mouse_pos_last;

    v2f mouse_move; // normalized (divide by width/height of window)
    v2f mouse_move_raw; // either -1, 0, or 1
    v2f mouse_move_absolute; // movement in pixels
} io_ctx_t;

void input_start_frame(io_ctx_t* ctx);
void input_end_frame(io_ctx_t* ctx);

bool input_key_down(i32 key);
bool input_key_pressed(i32 key);
bool input_key_held(i32 key);
bool input_key_released(i32 key);

bool input_mouse_down(i32 mouse_button);
bool input_mouse_pressed(i32 mouse_button);
bool input_mouse_held(i32 mouse_button);
bool input_mouse_released(i32 mouse_button);

v2f input_mouse_pos();
v2f input_mouse_move();
v2f input_mouse_move_raw();
v2f input_mouse_move_absolute();

extern io_ctx_t io_ctx;
extern window_t* game_window;

#endif /* ifndef _IO_H */
