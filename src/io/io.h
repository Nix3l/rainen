#ifndef _IO_H
#define _IO_H

#include "base.h"
#include "memory/memory.h"

// TODO(nix3l): add KEY_ANY and BUTTON_ANY

// KEY/BUTTON DEFINITIONS
typedef enum keycode_t {
    KEY_UNKNOWN = 0,
    KEY_SPACE,
    KEY_APOSTROPHE,
    KEY_COMMA,
    KEY_MINUS,
    KEY_PERIOD,
    KEY_SLASH,
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_SEMICOLON,
    KEY_EQUAL,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_LEFT_BRACKET,
    KEY_BACKSLASH,
    KEY_RIGHT_BRACKET,
    KEY_GRAVE_ACCENT,
    KEY_ESCAPE,
    KEY_ENTER,
    KEY_TAB,
    KEY_BACKSPACE,
    KEY_INSERT,
    KEY_DELETE,
    KEY_RIGHT,
    KEY_LEFT,
    KEY_DOWN,
    KEY_UP,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,
    KEY_HOME,
    KEY_END,
    KEY_CAPS_LOCK,
    KEY_SCROLL_LOCK,
    KEY_NUM_LOCK,
    KEY_PRINT_SCREEN,
    KEY_PAUSE,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_F13,
    KEY_F14,
    KEY_F15,
    KEY_F16,
    KEY_F17,
    KEY_F18,
    KEY_F19,
    KEY_F20,
    KEY_F21,
    KEY_F22,
    KEY_F23,
    KEY_F24,
    KEY_F25,
    KEY_KP_0,
    KEY_KP_1,
    KEY_KP_2,
    KEY_KP_3,
    KEY_KP_4,
    KEY_KP_5,
    KEY_KP_6,
    KEY_KP_7,
    KEY_KP_8,
    KEY_KP_9,
    KEY_KP_DECIMAL,
    KEY_KP_DIVIDE,
    KEY_KP_MULTIPLY,
    KEY_KP_SUBTRACT,
    KEY_KP_ADD,
    KEY_KP_ENTER,
    KEY_KP_EQUAL,
    KEY_LEFT_SHIFT,
    KEY_LEFT_CONTROL,
    KEY_LEFT_ALT,
    KEY_LEFT_SUPER,
    KEY_RIGHT_SHIFT,
    KEY_RIGHT_CONTROL,
    KEY_RIGHT_ALT,
    KEY_RIGHT_SUPER,
    KEY_MENU,
    KEYS_NUM,
} keycode_t;

typedef enum button_t {
    BUTTON_UNKNOWN = 0,
    BUTTON_0,
    BUTTON_1,
    BUTTON_2,
    BUTTON_3,
    BUTTON_4,
    BUTTON_5,
    BUTTON_6,
    BUTTON_7,
    BUTTON_LEFT = BUTTON_0,
    BUTTON_RIGHT = BUTTON_1,
    BUTTON_MIDDLE = BUTTON_2,
    BUTTONS_NUM,
} button_t;

typedef enum hitmode_t {
    HITMODE_NONE = 0,
    HITMODE_PRESSED,
    HITMODE_HELD,
    HITMODE_RELEASED,
    HITMODE_NUM,
} hitmode_t;

// BACKEND SPECIFIC STUFF
typedef struct gl_window_t {
    GLFWwindow* id;
} gl_window_t;

typedef struct gl_monitor_t {
    GLFWmonitor* id;
} gl_monitor_t;

// MONITOR
typedef struct video_mode_t {
    u32 w, h;
    u32 r_bits, g_bits, b_bits;
    u32 refresh_rate;
} video_mode_t;

typedef struct monitor_t {
    gl_monitor_t gl_monitor;

    vector_t video_modes;
    video_mode_t* active_mode;

    bool primary;
    bool active;
} monitor_t;

void monitors_detect();

// WINDOW
typedef struct window_t {
    gl_window_t gl_window;

    char* name;
    u32 width, height;

    bool cursor_enabled;
    bool fullscreen;
    monitor_t* monitor; // only set if fullscreen is true
    bool vsync;
    bool focused;
    bool minimized;
    bool closing;
} window_t;

void window_new(u32 width, u32 height, char* name);

bool window_closing();

void window_set_cursor_enable(bool enable);
void window_set_fullscreen(monitor_t* monitor, bool enable);
void window_set_vsync(bool enable);
void window_set_size(u32 width, u32 height);

void window_swap_buffers();

void window_destroy();

typedef struct input_state_t {
    hitmode_t keys[KEYS_NUM];
    hitmode_t buttons[BUTTONS_NUM];

    f32 scroll;
    f32 scroll_last;

    v2f mouse_pos;
    v2f mouse_pos_last;

    v2f mouse_move; // normalized (divide by width/height of window)
    v2f mouse_move_raw; // either -1, 0, or 1
    v2f mouse_move_absolute; // movement in pixels
} input_state_t;

void input_start_frame();

bool input_key_down(keycode_t key);
bool input_key_pressed(keycode_t key);
bool input_key_held(keycode_t key);
bool input_key_released(keycode_t key);

bool input_button_down(button_t button);
bool input_button_pressed(button_t button);
bool input_button_held(button_t button);
bool input_button_released(button_t button);

f32 input_get_scroll();

// bottom left is [0, 0]
v2f input_mouse_pos();
v2f input_mouse_move();
v2f input_mouse_move_raw();
v2f input_mouse_move_absolute();

// MOUSE DRAG
typedef enum drag_state_t {
    DRAG_STATE_IDLE = 0,
    DRAG_STATE_DRAGGING,
    DRAG_STATE_ACCEPTED,
    DRAG_STATE_CANCELLED,
} drag_state_t;

typedef struct mouse_drag_t {
    drag_state_t state;

    keycode_t cancel_key;
    button_t drag_button;

    v2f start;
    v2f end;
    v2f min;
    v2f max;
    f32 amount;
} mouse_drag_t;

void input_drag(mouse_drag_t* drag);
void input_drag_interrupt(mouse_drag_t* drag);

typedef struct io_ctx_t {
    arena_t rations;

    vector_t monitors;
    monitor_t* active_monitor;
    monitor_t* primary_monitor;

    window_t window;

    input_state_t state;
} io_ctx_t;

extern io_ctx_t io_ctx;

void io_init();
void io_terminate();

#endif /* ifndef _IO_H */
