#include "window.h"

#include "engine.h"
#include "util/log.h"

static void glfw_error_callback(int error, const char* text) {
    LOG_ERR("GLFW ERROR [%d]: `%s`\n", error, text);
}

void create_window(u32 width, u32 height, char* title) {
    ASSERT(glfwInit());

    // hint to opengl v4.3 core (needed for compute shaders)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    engine->window.width = width;
    engine->window.height = height;
    engine->window.title = title;

    engine->window.cursor_hidden = false;

    engine->window.glfw_window = glfwCreateWindow(width, height, title, NULL, NULL);
    if(!engine->window.glfw_window) {
        glfwTerminate();
        LOG_ERR("could not create window!\n");
        ASSERT_BREAK(!window->window);
    }

    glfwMakeContextCurrent(engine->window.glfw_window);

    // load all the opengl functions from the drivers with glad
    ASSERT(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress));

    glfwSetErrorCallback(glfw_error_callback);
}

void window_set_cursor_visibility(window_s* window, bool visibility) {
    window->cursor_hidden = visibility;
    glfwSetInputMode(window->glfw_window, GLFW_CURSOR, window->cursor_hidden ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void destroy_window() {
    glfwDestroyWindow(engine->window.glfw_window);
    glfwTerminate();
}
