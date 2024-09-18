#include "window.h"
#include "util/log.h"
#include "game.h"

static void glfw_error_callback(int error, const char* text) {
    LOG_ERR("GLFW ERROR [%d]: `%s`\n", error, text);
}

void create_window(u32 width, u32 height, char* title) {
    ASSERT(glfwInit());

    // hint to opengl v4.3 core (needed for compute shaders)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    game_state->window.width = width;
    game_state->window.height = height;
    game_state->window.title = title;

    game_state->window.cursor_hidden = false;

    game_state->window.glfw_window = glfwCreateWindow(width, height, title, NULL, NULL);
    if(!game_state->window.glfw_window) {
        glfwTerminate();
        LOG_ERR("could not create window!\n");
        ASSERT_BREAK(!window->window);
    }

    glfwMakeContextCurrent(game_state->window.glfw_window);

    // load all the opengl functions from the drivers with glad
    ASSERT(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress));

    glfwSetErrorCallback(glfw_error_callback);
}

void window_set_cursor_visibility(window_s* window, bool visibility) {
    window->cursor_hidden = visibility;
    glfwSetInputMode(window->glfw_window, GLFW_CURSOR, window->cursor_hidden ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void destroy_window() {
    glfwDestroyWindow(game_state->window.glfw_window);
    glfwTerminate();
}
