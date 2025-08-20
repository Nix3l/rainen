#ifndef _IMGUI_MANAGER_H
#define _IMGUI_MANAGER_H

#include "base.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

#define CIMGUI_USE_OPENGL3
#define CIMGUI_USE_GLFW
#include "cimgui_impl.h"

void imgui_init();
void imgui_terminate();

void imgui_start_frame();
void imgui_show();

#endif
