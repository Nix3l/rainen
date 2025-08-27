#include "io.h"

#include "base_macros.h"
#include "errors/errors.h"
#include "util/util.h"
#include "gfx/gfx.h"

io_ctx_t io_ctx = {0};

// GLFW
static keycode_t gl_keycode(int key) {
    switch(key) {
        case GLFW_KEY_SPACE: return KEY_SPACE;
        case GLFW_KEY_APOSTROPHE: return KEY_APOSTROPHE;
        case GLFW_KEY_COMMA: return KEY_COMMA;
        case GLFW_KEY_MINUS: return KEY_MINUS;
        case GLFW_KEY_PERIOD: return KEY_PERIOD;
        case GLFW_KEY_SLASH: return KEY_SLASH;
        case GLFW_KEY_0: return KEY_0;
        case GLFW_KEY_1: return KEY_1;
        case GLFW_KEY_2: return KEY_2;
        case GLFW_KEY_3: return KEY_3;
        case GLFW_KEY_4: return KEY_4;
        case GLFW_KEY_5: return KEY_5;
        case GLFW_KEY_6: return KEY_6;
        case GLFW_KEY_7: return KEY_7;
        case GLFW_KEY_8: return KEY_8;
        case GLFW_KEY_9: return KEY_9;
        case GLFW_KEY_SEMICOLON: return KEY_SEMICOLON;
        case GLFW_KEY_EQUAL: return KEY_EQUAL;
        case GLFW_KEY_A: return KEY_A;
        case GLFW_KEY_B: return KEY_B;
        case GLFW_KEY_C: return KEY_C;
        case GLFW_KEY_D: return KEY_D;
        case GLFW_KEY_E: return KEY_E;
        case GLFW_KEY_F: return KEY_F;
        case GLFW_KEY_G: return KEY_G;
        case GLFW_KEY_H: return KEY_H;
        case GLFW_KEY_I: return KEY_I;
        case GLFW_KEY_J: return KEY_J;
        case GLFW_KEY_K: return KEY_K;
        case GLFW_KEY_L: return KEY_L;
        case GLFW_KEY_M: return KEY_M;
        case GLFW_KEY_N: return KEY_N;
        case GLFW_KEY_O: return KEY_O;
        case GLFW_KEY_P: return KEY_P;
        case GLFW_KEY_Q: return KEY_Q;
        case GLFW_KEY_R: return KEY_R;
        case GLFW_KEY_S: return KEY_S;
        case GLFW_KEY_T: return KEY_T;
        case GLFW_KEY_U: return KEY_U;
        case GLFW_KEY_V: return KEY_V;
        case GLFW_KEY_W: return KEY_W;
        case GLFW_KEY_X: return KEY_X;
        case GLFW_KEY_Y: return KEY_Y;
        case GLFW_KEY_Z: return KEY_Z;
        case GLFW_KEY_LEFT_BRACKET: return KEY_LEFT_BRACKET;
        case GLFW_KEY_BACKSLASH: return KEY_BACKSLASH;
        case GLFW_KEY_RIGHT_BRACKET: return KEY_RIGHT_BRACKET;
        case GLFW_KEY_GRAVE_ACCENT: return KEY_GRAVE_ACCENT;
        case GLFW_KEY_ESCAPE: return KEY_ESCAPE;
        case GLFW_KEY_ENTER: return KEY_ENTER;
        case GLFW_KEY_TAB: return KEY_TAB;
        case GLFW_KEY_BACKSPACE: return KEY_BACKSPACE;
        case GLFW_KEY_INSERT: return KEY_INSERT;
        case GLFW_KEY_DELETE: return KEY_DELETE;
        case GLFW_KEY_RIGHT: return KEY_RIGHT;
        case GLFW_KEY_LEFT: return KEY_LEFT;
        case GLFW_KEY_DOWN: return KEY_DOWN;
        case GLFW_KEY_UP: return KEY_UP;
        case GLFW_KEY_PAGE_UP: return KEY_PAGE_UP;
        case GLFW_KEY_PAGE_DOWN: return KEY_PAGE_DOWN;
        case GLFW_KEY_HOME: return KEY_HOME;
        case GLFW_KEY_END: return KEY_END;
        case GLFW_KEY_CAPS_LOCK: return KEY_CAPS_LOCK;
        case GLFW_KEY_SCROLL_LOCK: return KEY_SCROLL_LOCK;
        case GLFW_KEY_NUM_LOCK: return KEY_NUM_LOCK;
        case GLFW_KEY_PRINT_SCREEN: return KEY_PRINT_SCREEN;
        case GLFW_KEY_PAUSE: return KEY_PAUSE;
        case GLFW_KEY_F1: return KEY_F1;
        case GLFW_KEY_F2: return KEY_F2;
        case GLFW_KEY_F3: return KEY_F3;
        case GLFW_KEY_F4: return KEY_F4;
        case GLFW_KEY_F5: return KEY_F5;
        case GLFW_KEY_F6: return KEY_F6;
        case GLFW_KEY_F7: return KEY_F7;
        case GLFW_KEY_F8: return KEY_F8;
        case GLFW_KEY_F9: return KEY_F9;
        case GLFW_KEY_F10: return KEY_F10;
        case GLFW_KEY_F11: return KEY_F11;
        case GLFW_KEY_F12: return KEY_F12;
        case GLFW_KEY_F13: return KEY_F13;
        case GLFW_KEY_F14: return KEY_F14;
        case GLFW_KEY_F15: return KEY_F15;
        case GLFW_KEY_F16: return KEY_F16;
        case GLFW_KEY_F17: return KEY_F17;
        case GLFW_KEY_F18: return KEY_F18;
        case GLFW_KEY_F19: return KEY_F19;
        case GLFW_KEY_F20: return KEY_F20;
        case GLFW_KEY_F21: return KEY_F21;
        case GLFW_KEY_F22: return KEY_F22;
        case GLFW_KEY_F23: return KEY_F23;
        case GLFW_KEY_F24: return KEY_F24;
        case GLFW_KEY_F25: return KEY_F25;
        case GLFW_KEY_KP_0: return KEY_KP_0;
        case GLFW_KEY_KP_1: return KEY_KP_1;
        case GLFW_KEY_KP_2: return KEY_KP_2;
        case GLFW_KEY_KP_3: return KEY_KP_3;
        case GLFW_KEY_KP_4: return KEY_KP_4;
        case GLFW_KEY_KP_5: return KEY_KP_5;
        case GLFW_KEY_KP_6: return KEY_KP_6;
        case GLFW_KEY_KP_7: return KEY_KP_7;
        case GLFW_KEY_KP_8: return KEY_KP_8;
        case GLFW_KEY_KP_9: return KEY_KP_9;
        case GLFW_KEY_KP_DECIMAL: return KEY_KP_DECIMAL;
        case GLFW_KEY_KP_DIVIDE: return KEY_KP_DIVIDE;
        case GLFW_KEY_KP_MULTIPLY: return KEY_KP_MULTIPLY;
        case GLFW_KEY_KP_SUBTRACT: return KEY_KP_SUBTRACT;
        case GLFW_KEY_KP_ADD: return KEY_KP_ADD;
        case GLFW_KEY_KP_ENTER: return KEY_KP_ENTER;
        case GLFW_KEY_KP_EQUAL: return KEY_KP_EQUAL;
        case GLFW_KEY_LEFT_SHIFT: return KEY_LEFT_SHIFT;
        case GLFW_KEY_LEFT_CONTROL: return KEY_LEFT_CONTROL;
        case GLFW_KEY_LEFT_ALT: return KEY_LEFT_ALT;
        case GLFW_KEY_LEFT_SUPER: return KEY_LEFT_SUPER;
        case GLFW_KEY_RIGHT_SHIFT: return KEY_RIGHT_SHIFT;
        case GLFW_KEY_RIGHT_CONTROL: return KEY_RIGHT_CONTROL;
        case GLFW_KEY_RIGHT_ALT: return KEY_RIGHT_ALT;
        case GLFW_KEY_RIGHT_SUPER: return KEY_RIGHT_SUPER;
        case GLFW_KEY_MENU: return KEY_MENU;
        default: return KEY_UNKNOWN;
    }
}

static button_t gl_button(int button) {
    switch (button) {
        case GLFW_MOUSE_BUTTON_1: return BUTTON_0;
        case GLFW_MOUSE_BUTTON_2: return BUTTON_1;
        case GLFW_MOUSE_BUTTON_3: return BUTTON_2;
        case GLFW_MOUSE_BUTTON_4: return BUTTON_3;
        case GLFW_MOUSE_BUTTON_5: return BUTTON_4;
        case GLFW_MOUSE_BUTTON_6: return BUTTON_5;
        case GLFW_MOUSE_BUTTON_7: return BUTTON_6;
        case GLFW_MOUSE_BUTTON_8: return BUTTON_7;
        default: return BUTTON_UNKNOWN;
    }
}

static void gl_error_callback(int code, const char* description) {
    LOG_ERR("\n\t => GLFW ERR [%d]: %s\n", code, description);
}

static void gl_window_focus_callback(GLFWwindow* window, i32 focused) {
    io_ctx.window.focused = focused;
    UNUSED(window);
}

static void gl_window_minimize_callback(GLFWwindow* window, i32 minimized) {
    io_ctx.window.minimized = minimized;
    UNUSED(window);
}

static void gl_window_close_callback(GLFWwindow* window) {
    io_ctx.window.closing = true;
    UNUSED(window);
}

static void gl_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    keycode_t keycode = gl_keycode(key);
    hitmode_t hit = io_ctx.state.keys[keycode];
    hitmode_t new_mode = hit;

    if(action == GLFW_PRESS && hit == HITMODE_NONE) new_mode = HITMODE_PRESSED;
    if(action == GLFW_RELEASE && (hit == HITMODE_HELD || hit == HITMODE_PRESSED)) new_mode = HITMODE_RELEASED;
    if(hit == HITMODE_RELEASED) new_mode = hit;

    io_ctx.state.keys[keycode] = new_mode;

    UNUSED(window);
    UNUSED(scancode);
    UNUSED(mods);
}

static void gl_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if(yoffset > 0.0) io_ctx.state.scroll = 1.0;
    else if(yoffset < 0.0) io_ctx.state.scroll = -1.0;

    UNUSED(window);
    UNUSED(xoffset);
}

static void gl_mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    button_t buttoncode = gl_button(button);
    hitmode_t hit = io_ctx.state.buttons[buttoncode];
    hitmode_t new_mode = hit;

    if(action == GLFW_PRESS && hit == HITMODE_NONE) new_mode = HITMODE_PRESSED;
    if(action == GLFW_RELEASE && (hit == HITMODE_HELD || hit == HITMODE_PRESSED)) new_mode = HITMODE_RELEASED;
    if(hit == HITMODE_RELEASED) new_mode = hit;

    io_ctx.state.buttons[buttoncode] = new_mode;

    UNUSED(window);
    UNUSED(mods);
}

static void gl_mouse_position_callback(GLFWwindow* window, double xpos, double ypos) {
    io_ctx.state.mouse_pos.x = (f32) xpos;
    io_ctx.state.mouse_pos.y = io_ctx.window.height - (f32) ypos;

    UNUSED(window);
}

static void gl_detect_monitors() {
    i32 num_monitors;
    GLFWmonitor** glfw_monitors = glfwGetMonitors(&num_monitors);
    GLFWmonitor* primary_glfw_monitor = glfwGetPrimaryMonitor();

    io_ctx.monitors = vector_alloc_new(num_monitors, sizeof(monitor_t));
    for(i32 i = 0; i < num_monitors; i ++) {
        monitor_t* monitor = vector_push(&io_ctx.monitors);
        GLFWmonitor* glfw_monitor = glfw_monitors[i];
        monitor->gl_monitor.id = glfw_monitor;

        i32 num_video_modes;
        GLFWvidmode* video_modes = (GLFWvidmode*) glfwGetVideoModes(glfw_monitor, &num_video_modes);
        GLFWvidmode* active_mode = (GLFWvidmode*) glfwGetVideoMode(glfw_monitor);
        monitor->video_modes = vector_alloc_new(num_video_modes, sizeof(video_mode_t));
        for(i32 i = 0; i < num_video_modes; i ++) {
            GLFWvidmode gl_vidmode = video_modes[i];
            video_mode_t* mode = vector_push(&monitor->video_modes);
            mode->w = gl_vidmode.width;
            mode->h = gl_vidmode.height;
            mode->r_bits = gl_vidmode.redBits;
            mode->g_bits = gl_vidmode.greenBits;
            mode->b_bits = gl_vidmode.blueBits;
            mode->refresh_rate = gl_vidmode.refreshRate;

            // i could probably use memcmp but eh
            bool is_active = true;
            if(gl_vidmode.width != active_mode->width) is_active = false;
            if(gl_vidmode.height != active_mode->height) is_active = false;
            if(gl_vidmode.redBits != active_mode->redBits) is_active = false;
            if(gl_vidmode.greenBits != active_mode->greenBits) is_active = false;
            if(gl_vidmode.blueBits != active_mode->blueBits) is_active = false;
            if(gl_vidmode.refreshRate != active_mode->refreshRate) is_active = false;
            if(is_active) monitor->active_mode = mode;
        }

        if(primary_glfw_monitor == glfw_monitor) {
            monitor->primary = true;
            monitor->active = true; // primary monitor is active by default
            io_ctx.active_monitor = monitor;
            io_ctx.primary_monitor = monitor;
        } else {
            monitor->primary = false;
            monitor->active = false;
        }
    }
}

static void gl_window_new(u32 width, u32 height, char* name) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);
    glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CENTER_CURSOR, GLFW_TRUE);

    GLFWwindow* glfw_window = glfwCreateWindow(width, height, name, NULL, NULL);
    if(!glfw_window) {
        io_terminate();
        PANIC("couldnt create glfw window\n");
    }

    glfwMakeContextCurrent(glfw_window);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        io_terminate();
        PANIC("gload loading failed???\n");
    }

    glfwSetWindowFocusCallback(glfw_window, gl_window_focus_callback);
    glfwSetWindowIconifyCallback(glfw_window, gl_window_minimize_callback);
    glfwSetWindowCloseCallback(glfw_window, gl_window_close_callback);
    glfwSetKeyCallback(glfw_window, gl_key_callback);
    glfwSetScrollCallback(glfw_window, gl_scroll_callback);
    glfwSetMouseButtonCallback(glfw_window, gl_mouse_button_callback);
    glfwSetCursorPosCallback(glfw_window, gl_mouse_position_callback);

    io_ctx.window.gl_window = (gl_window_t) { .id = glfw_window, };
}

static void gl_window_set_cursor_enable(bool enable) {
    glfwSetInputMode(io_ctx.window.gl_window.id, GLFW_CURSOR, enable ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

static void gl_window_set_fullscreen(monitor_t* monitor, bool enable) {
    window_t* window = &io_ctx.window;
    if(enable) glfwSetWindowMonitor(window->gl_window.id, monitor->gl_monitor.id, 0, 0, window->width, window->height, monitor->active_mode->refresh_rate);
    else glfwSetWindowMonitor(window->gl_window.id, NULL, 0, 0, window->width, window->height, monitor->active_mode->refresh_rate);
}

static void gl_window_set_vsync(bool enable) {
    glfwSwapInterval(enable ? 1 : 0);
}

static void gl_window_set_size(u32 width, u32 height) {
    glfwSetWindowSize(io_ctx.window.gl_window.id, width, height);
}

static void gl_window_swap_buffers() {
    glfwSwapBuffers(io_ctx.window.gl_window.id);
}

static void gl_window_destroy() {
    glfwDestroyWindow(io_ctx.window.gl_window.id);
}

static void gl_poll_events() {
    glfwPollEvents();
}

static void gl_io_init() {
    ASSERT(glfwInit());
    glfwSetErrorCallback(gl_error_callback);
}

static void gl_io_terminate() {
    glfwTerminate();
}

// CONTEXT
void io_init() {
    switch(gfx_backend()) {
        case GFX_BACKEND_GL: gl_io_init(); break;
        default: UNREACHABLE; break;
    }
}

void io_terminate() {
    switch(gfx_backend()) {
        case GFX_BACKEND_GL: gl_io_terminate(); break;
        default: UNREACHABLE; break;
    }
}

// MONTORS
void monitors_detect() {
    switch(gfx_backend()) {
        case GFX_BACKEND_GL: gl_detect_monitors(); break;
        default: UNREACHABLE; break;
    }
}

// WINDOW
void window_new(u32 width, u32 height, char* name) {
    io_ctx.window = (window_t) {
        .name = name,
        .width = width,
        .height = height,

        .cursor_enabled = false,
        .fullscreen = false,
        .vsync = false,

        .focused = false,
        .minimized = false,
    };

    switch(gfx_backend()) {
        case GFX_BACKEND_GL: gl_window_new(width, height, name); break;
        default: UNREACHABLE; break;
    }
}

bool window_closing() {
    return io_ctx.window.closing;
}

void window_set_cursor_enable(bool enable) {
    io_ctx.window.cursor_enabled = enable;
    switch(gfx_backend()) {
        case GFX_BACKEND_GL: gl_window_set_cursor_enable(enable); break;
        default: UNREACHABLE; break;
    }
}

void window_set_fullscreen(monitor_t* monitor, bool enable) {
    io_ctx.window.fullscreen = enable;
    switch(gfx_backend()) {
        case GFX_BACKEND_GL: gl_window_set_fullscreen(monitor, enable); break;
        default: UNREACHABLE; break;
    }
}

void window_set_vsync(bool enable) {
    io_ctx.window.vsync = enable;
    switch(gfx_backend()) {
        case GFX_BACKEND_GL: gl_window_set_vsync(enable); break;
        default: UNREACHABLE; break;
    }
}

void window_set_size(u32 width, u32 height) {
    io_ctx.window.width = width;
    io_ctx.window.height = height;
    switch(gfx_backend()) {
        case GFX_BACKEND_GL: gl_window_set_size(width, height); break;
        default: UNREACHABLE; break;
    }
}

void window_destroy() {
    switch(gfx_backend()) {
        case GFX_BACKEND_GL: gl_window_destroy(); break;
        default: UNREACHABLE; break;
    }
}

// INPUT
void input_start_frame() {
    io_ctx.state.mouse_pos_last = io_ctx.state.mouse_pos;
    io_ctx.state.scroll = 0.0f;
    input_state_t old_state = io_ctx.state;

    switch(gfx_backend()) {
        case GFX_BACKEND_GL: gl_poll_events(); break;
        default: UNREACHABLE; break;
    }

    for(u32 i = 0; i < KEYS_NUM; i ++) {
        if(old_state.keys[i] == HITMODE_PRESSED && io_ctx.state.keys[i] == HITMODE_PRESSED)
            io_ctx.state.keys[i] = HITMODE_HELD;

        if(old_state.keys[i] == HITMODE_RELEASED && io_ctx.state.keys[i] == HITMODE_RELEASED)
            io_ctx.state.keys[i] = HITMODE_NONE;
    }

    for(u32 i = 0; i < BUTTONS_NUM; i ++) {
        if(old_state.buttons[i] == HITMODE_PRESSED && io_ctx.state.buttons[i] == HITMODE_PRESSED)
            io_ctx.state.buttons[i] = HITMODE_HELD;

        if(old_state.buttons[i] == HITMODE_RELEASED && io_ctx.state.buttons[i] == HITMODE_RELEASED)
            io_ctx.state.buttons[i] = HITMODE_NONE;
    }

    v2f diff = v2f_sub(io_ctx.state.mouse_pos, io_ctx.state.mouse_pos_last);
    io_ctx.state.mouse_move_absolute = diff;

    io_ctx.state.mouse_move = v2f_new(
        diff.x / (f32) io_ctx.window.width,
        diff.y / (f32) io_ctx.window.height
    );

    io_ctx.state.mouse_move_raw = v2f_new(
        diff.x == 0.0f ? 0.0f : diff.x / diff.x,
        diff.y == 0.0f ? 0.0f : diff.y / diff.y
    );
}

void window_swap_buffers() {
    switch(gfx_backend()) {
        case GFX_BACKEND_GL: gl_window_swap_buffers(); break;
        default: UNREACHABLE; break;
    }
}

bool input_key_down(keycode_t key) {
    return io_ctx.state.keys[key] == HITMODE_PRESSED || io_ctx.state.keys[key] == HITMODE_HELD;
}

bool input_key_pressed(keycode_t key) {
    return io_ctx.state.keys[key] == HITMODE_PRESSED;
}

bool input_key_held(keycode_t key) {
    return io_ctx.state.keys[key] == HITMODE_HELD;
}

bool input_key_released(keycode_t key) {
    return io_ctx.state.keys[key] == HITMODE_RELEASED;
}

bool input_button_down(button_t button) {
    return io_ctx.state.buttons[button] == HITMODE_PRESSED || io_ctx.state.buttons[button] == HITMODE_HELD;
}

bool input_button_pressed(button_t button) {
    return io_ctx.state.buttons[button] == HITMODE_PRESSED;
}

bool input_button_held(button_t button) {
    return io_ctx.state.buttons[button] == HITMODE_HELD;
}

bool input_button_released(button_t button) {
    return io_ctx.state.buttons[button] == HITMODE_RELEASED;
}

f32 input_get_scroll() {
    return io_ctx.state.scroll;
}

v2f input_mouse_pos() {
    return io_ctx.state.mouse_pos;
}

v2f input_mouse_move() {
    return io_ctx.state.mouse_move;
}

v2f input_mouse_move_raw() {
    return io_ctx.state.mouse_move_raw;
}

v2f input_mouse_move_absolute() {
    return io_ctx.state.mouse_move_absolute;
}

// DRAG
void input_drag(mouse_drag_t* drag) {
    switch(drag->state) {
        case DRAG_STATE_IDLE:
            if(drag->drag_button == BUTTON_UNKNOWN) drag->drag_button = BUTTON_LEFT;
            if(input_button_pressed(drag->drag_button)) {
                drag->state = DRAG_STATE_DRAGGING;
                drag->start = input_mouse_pos();
            }
        break;

        case DRAG_STATE_DRAGGING:
            if(drag->cancel_key != KEY_UNKNOWN && input_key_pressed(drag->cancel_key)) {
                drag->state = DRAG_STATE_CANCELLED;
                drag->start = v2f_ZERO;
                drag->end = v2f_ZERO;
                drag->min = v2f_ZERO;
                drag->max = v2f_ZERO;
                drag->amount = 0.0f;
                break;
            }

            drag->end = input_mouse_pos();
            drag->min = v2f_new(MIN(drag->start.x, drag->end.x), MIN(drag->start.y, drag->end.y));
            drag->max = v2f_new(MAX(drag->start.x, drag->end.x), MAX(drag->start.y, drag->end.y));

            v2f diff = v2f_sub(drag->end, drag->start);
            drag->amount = v2f_dot(diff, diff);

            if(!input_button_down(drag->drag_button)) drag->state = DRAG_STATE_ACCEPTED;
        break;

        case DRAG_STATE_ACCEPTED:
        case DRAG_STATE_CANCELLED:
            drag->state = DRAG_STATE_IDLE;
            drag->start = v2f_ZERO;
            drag->end = v2f_ZERO;
            drag->min = v2f_ZERO;
            drag->max = v2f_ZERO;
            drag->amount = 0.0f;
        break;

        default: LOG_ERR_CODE(ERR_IO_UNKNOWN_DRAG_STATE); return;
    }
}

void input_drag_interrupt(mouse_drag_t* drag) {
    drag->state = DRAG_STATE_CANCELLED;
    drag->start = v2f_ZERO;
    drag->end = v2f_ZERO;
    drag->min = v2f_ZERO;
    drag->max = v2f_ZERO;
    drag->amount = 0.0f;
}
