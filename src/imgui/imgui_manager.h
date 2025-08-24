#ifndef _IMGUI_MANAGER_H
#define _IMGUI_MANAGER_H

#include "base.h"
#include "gfx/gfx.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

#define CIMGUI_USE_OPENGL3
#define CIMGUI_USE_GLFW
#include "cimgui_impl.h"

#define imv2f(_x, _y) ((ImVec2) { .x = _x, .y = _y })
#define imv2f_ZERO ((ImVec2) { .x = 0.0f, .y = 0.0f })

void imgui_init();
void imgui_terminate();

void imgui_start_frame();
void imgui_show();

void imgui_texture_image(texture_t texture, v2f size);
void imgui_texture_image_range(texture_t texture, v2f size, v2f uv0, v2f uv1);

#endif
