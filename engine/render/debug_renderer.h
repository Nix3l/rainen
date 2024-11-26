#ifndef DEBUG_RENDERER_H
#define DEBUG_RENDERER_H

#include "renderer.h"

#define COL_BLACK  V3F_RGB(27.0f, 27.0f, 27.0f)
#define COL_WHITE  V3F_RGB(221.0f, 199.0f, 161.0f)
#define COL_RED    V3F_RGB(234.0f, 105.0f, 98.0f)
#define COL_GREEN  V3F_RGB(137.0f, 180.0f, 130.0f)
#define COL_BLUE   V3F_RGB(125.0f, 174.0f, 163.0f)
#define COL_YELLOW V3F_RGB(216.0f, 166.0f, 87.0f)
#define COL_ORANGE V3F_RGB(231.0f, 138.0f, 78.0f)
#define COL_PURPLE V3F_RGB(211.0f, 134.0f, 155.0f)

void render_debug_rect(v2f position, v2f size, v3f col);
void render_debug_line(v2f start, v2f end, f32 stroke, v3f col);
void render_debug_point(v2f position, f32 size, v3f col);
// TODO(nix3l): void render_debug_circle(v2f position, f32 radius, v3f col);

#endif
