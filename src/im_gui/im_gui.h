#ifndef IM_GUI_MANAGER_H
#define IM_GUI_MANAGER_H

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

#define CIMGUI_USE_GLFW
#define CIMGUI_USE_OPENGL3
#include "cimgui_impl.h"

void init_imgui();
void shutdown_imgui();

void update_imgui();
void render_imgui();

#endif
