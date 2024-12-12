#include "input.h"

#include "engine.h"

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    engine->input_state.keys[key] = (action == GLFW_PRESS || action == GLFW_REPEAT) && action != GLFW_RELEASE;
    engine->input_state.keys_pressed[key] = action == GLFW_PRESS && action != GLFW_RELEASE;
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mode) {
    engine->input_state.buttons[button] = (action == GLFW_PRESS || action == GLFW_REPEAT) && action != GLFW_RELEASE;
    engine->input_state.buttons_pressed[button] = action == GLFW_PRESS && action != GLFW_RELEASE;
}

static void mouse_position_callback(GLFWwindow* window, double xpos, double ypos) {
    engine->input_state.mouse_pos.x = (float) xpos;
    engine->input_state.mouse_pos.y = (float) ypos;
}

void init_input() {
    glfwSetKeyCallback(engine->window.glfw_window, key_callback);
    glfwSetMouseButtonCallback(engine->window.glfw_window, mouse_button_callback);
    glfwSetCursorPosCallback(engine->window.glfw_window, mouse_position_callback);
}

void update_input() {
    for(usize i = 0; i < GLFW_KEY_LAST; i ++)
        engine->input_state.keys_pressed[i] = false;

    for(usize i = 0; i < GLFW_MOUSE_BUTTON_LAST; i ++)
        engine->input_state.buttons_pressed[i] = false;

    engine->input_state.last_mouse_pos = engine->input_state.mouse_pos;
}

bool is_key_down(int key) {
    return engine->input_state.keys[key];
}

bool is_key_pressed(int key) {
    return engine->input_state.keys_pressed[key];
}

bool is_button_down(int button) {
    return engine->input_state.buttons[button];
}

bool is_button_pressed(int button) {
    return engine->input_state.buttons[button];
}

v2f get_mouse_pos() {
    return engine->input_state.mouse_pos;
}

v2f get_mouse_absolute_move() {
    v2f pos = engine->input_state.mouse_pos;
    v2f last_pos = engine->input_state.last_mouse_pos;
    
    return (v2f) {
        .x = (pos.x - last_pos.x),
        .y = (pos.y - last_pos.y)
    };
}

v2f get_mouse_move() {
    v2f pos = engine->input_state.mouse_pos;
    v2f last_pos = engine->input_state.last_mouse_pos;
    
    return (v2f) {
        .x = ((pos.x - last_pos.x) / engine->window.width),
        .y = ((pos.y - last_pos.y) / engine->window.height)
    };
}

v2f get_mouse_raw_move() {
    v2f pos = engine->input_state.mouse_pos;
    v2f last_pos = engine->input_state.last_mouse_pos;
    v2f move = {
        .x = (pos.x - last_pos.x),
        .y = (pos.y - last_pos.y)
    };
    
    return (v2f) {
        .x = (move.x < 0.0f ? -1.0f : (move.x > 0.0f ? 1.0f : 0.0f)),
        .y = (move.y < 0.0f ? -1.0f : (move.y > 0.0f ? 1.0f : 0.0f)),
    };
}

mouse_state_s get_mouse_state() {
    return (mouse_state_s) {
        .pos = engine->input_state.mouse_pos,
        .absolute_move = get_mouse_absolute_move(),
        .move = get_mouse_move(),
        .raw_move = get_mouse_raw_move()
    };
}
