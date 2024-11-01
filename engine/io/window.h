#ifndef WINDOW_H
#define WINDOW_H

#include "base.h"
#include "memory/memory.h"

typedef struct {
    GLFWwindow* glfw_window;

    char* title;
    u32 width;
    u32 height;

    bool cursor_hidden;
} window_s;

void create_window(u32 width, u32 height, char* title);
void destroy_window();

void window_set_cursor_visibility(window_s* window, bool visibility);

#endif
