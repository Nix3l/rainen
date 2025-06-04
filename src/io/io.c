#include "io.h"

#include "memory/memory.h"
#include "util/util.h"

io_ctx_t io_ctx = {0};
window_t* game_window = &io_ctx.window;

// GLFW CALLBACKS
static void error_callback(int code, const char* description) {
    LOG_ERR("\nGLFW ERR [%d]: %s\n", code, description);
}

// WINDOW CALLBACKS
static void window_focus_callback(GLFWwindow* window, i32 focused) {
    game_window->focused = focused;
}

static void window_minimize_callback(GLFWwindow* window, i32 minimized) {
    game_window->minimized = minimized;
}

// INPUT CALLBACKS
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    io_ctx.keys[key] = (action == GLFW_PRESS || action == GLFW_REPEAT) && action != GLFW_RELEASE;
    io_ctx.keys_pressed[key] = action == GLFW_PRESS && (action != GLFW_RELEASE || action == GLFW_REPEAT);
    io_ctx.keys_held[key] = action == GLFW_REPEAT && action != GLFW_RELEASE;
    io_ctx.keys_released[key] = action == GLFW_RELEASE;
}

// TODO(nix3l): fix this, no GLFW_REPEAT for mouse callbacks
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mode) {
    io_ctx.mouse_buttons[button] = (action == GLFW_PRESS) && action != GLFW_RELEASE;
    io_ctx.mouse_buttons_pressed[button] = action == GLFW_PRESS && action != GLFW_RELEASE;
    io_ctx.mouse_buttons_held[button] = action == GLFW_REPEAT && action != GLFW_RELEASE;
    io_ctx.mouse_buttons_released[button] = action == GLFW_RELEASE;
}

static void mouse_position_callback(GLFWwindow* window, double xpos, double ypos) {
    io_ctx.mouse_pos.x = (float) xpos;
    io_ctx.mouse_pos.y = (float) ypos;
}

// GLFW (initialisation/termination)
void io_init() {
    ASSERT(glfwInit());
    glfwSetErrorCallback(error_callback);

    io_ctx.initialized = false;
}

void io_terminate() {
    glfwTerminate();
}

// MONTORS
void monitors_detect() {
    i32 num_monitors;
    GLFWmonitor** glfw_monitors = glfwGetMonitors(&num_monitors);
    GLFWmonitor* primary_glfw_monitor = glfwGetPrimaryMonitor();

    io_ctx.monitors = vector_alloc_new(num_monitors, sizeof(monitor_t));
    for(i32 i = 0; i < num_monitors; i ++) {
        monitor_t* monitor = vector_push(&io_ctx.monitors);
        GLFWmonitor* glfw_monitor = glfw_monitors[i];
        monitor->glfw_monitor = glfw_monitor;

        glfwGetMonitorPhysicalSize(glfw_monitor, &monitor->physical_width, &monitor->physical_height);

        i32 num_video_modes;
        GLFWvidmode* video_modes = (GLFWvidmode*) glfwGetVideoModes(glfw_monitor, &num_video_modes);
        monitor->video_modes = vector_new(video_modes, num_video_modes, sizeof(GLFWvidmode));

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

// WINDOW
window_t window_new(monitor_t* monitor, u32 width, u32 height, char* name) {
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

    if(!io_ctx.initialized) {
        if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            io_terminate();
            PANIC("gload loading failed???\n");
        } else {
            io_ctx.initialized = true;
        }
    }

    glfwSetWindowFocusCallback(glfw_window, window_focus_callback);
    glfwSetWindowIconifyCallback(glfw_window, window_minimize_callback);
    glfwSetKeyCallback(glfw_window, key_callback);
    glfwSetMouseButtonCallback(glfw_window, mouse_button_callback);
    glfwSetCursorPosCallback(glfw_window, mouse_position_callback);

    return (window_t) {
        .glfw_window = glfw_window,

        .name = name,
        .width = width,
        .height = height,

        .cursor_enabled = false,
        .fullscreen = false,
        .vsync = false,

        .focused = false,
        .minimized = false,
    };
}

bool window_closing(window_t* window) {
    return glfwWindowShouldClose(window->glfw_window);
}

void window_enable_cursor(window_t* window, bool enable) {
    window->cursor_enabled = enable;
    glfwSetInputMode(window->glfw_window, GLFW_CURSOR, enable ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

void window_set_fullscreen(window_t* window, monitor_t* monitor, bool enable) {
    window->fullscreen = enable;
    if(enable)
        glfwSetWindowMonitor(window->glfw_window, monitor->glfw_monitor, 0, 0, window->width, window->height, monitor->active_mode->refreshRate);
    else
        glfwSetWindowMonitor(window->glfw_window, NULL, 0, 0, window->width, window->height, monitor->active_mode->refreshRate);
}

void window_set_vsync(window_t* window, bool enable) {
    window->vsync = enable;
    glfwSwapInterval(enable ? 1 : 0);
}

void window_set_size(window_t* window, u32 width, u32 height) {
    window->width = width;
    window->height = height;
    glfwSetWindowSize(window->glfw_window, width, height);
}

void window_destroy(window_t* window) {
    glfwDestroyWindow(window->glfw_window);
}

// INPUT
void input_start_frame(io_ctx_t* ctx) {
    glfwPollEvents();

    v2f diff = v2f_sub(ctx->mouse_pos, ctx->mouse_pos_last);
    ctx->mouse_move_absolute = diff;
    ctx->mouse_move = v2f_new(
        diff.x / (f32) ctx->window.width,
        diff.y / (f32) ctx->window.height
    );
    ctx->mouse_move_raw = v2f_new(
        diff.x == 0.0f ? 0.0f : diff.x / diff.x,
        diff.y == 0.0f ? 0.0f : diff.y / diff.y
    );
}

void input_end_frame(io_ctx_t* ctx) {
    for(u32 i = 0; i < GLFW_KEY_LAST; i ++) {
        ctx->keys_pressed[i] = false;
        ctx->keys_released[i] = false;
    }

    for(u32 i = 0; i < GLFW_MOUSE_BUTTON_LAST; i ++) {
        ctx->mouse_buttons_pressed[i] = false;
        ctx->mouse_buttons_released[i] = false;
    }

    ctx->mouse_pos_last = ctx->mouse_pos;
}

bool input_key_down(i32 key) {
    return io_ctx.keys[key];
}

bool input_key_pressed(i32 key) {
    return io_ctx.keys_pressed[key];
}

bool input_key_held(i32 key) {
    return io_ctx.keys_held[key];
}

bool input_key_released(i32 key) {
    return io_ctx.keys_released[key];
}

bool input_mouse_down(i32 mouse_button) {
    return io_ctx.mouse_buttons[mouse_button];
}

bool input_mouse_pressed(i32 mouse_button) {
    return io_ctx.mouse_buttons_pressed[mouse_button];
}
bool input_mouse_held(i32 mouse_button) {
    return io_ctx.mouse_buttons_held[mouse_button];
}
bool input_mouse_released(i32 mouse_button) {
    return io_ctx.mouse_buttons_released[mouse_button];
}

v2f input_mouse_pos() {
    return io_ctx.mouse_pos;
}

v2f input_mouse_move() {
    return io_ctx.mouse_move;
}

v2f input_mouse_move_raw() {
    return io_ctx.mouse_move_raw;
}

v2f input_mouse_move_absolute() {
    return io_ctx.mouse_move_absolute;
}
