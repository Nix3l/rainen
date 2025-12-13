#ifndef _DEBUG_RENDER_H 

#include "base.h"
#include "render/render.h"

typedef enum debug_render_shape_t {
    DEBUG_SHAPE_INVALID = 0,
    DEBUG_SHAPE_RECT,
    DEBUG_SHAPE_CIRCLE,
} debug_render_shape_t;

typedef enum debug_render_color_t {
    DEBUG_COLOR_RED = 0,
    DEBUG_COLOR_ORANGE,
    DEBUG_COLOR_YELLOW,
    DEBUG_COLOR_GREEN,
    DEBUG_COLOR_BLUE,
    DEBUG_COLOR_AQUA,
    DEBUG_COLOR_PURPLE,
    DEBUG_COLOR_BLACK,
    DEBUG_COLOR_WHITE,
    DEBUG_COLOR_RANDOM,
} debug_render_color_t;

typedef union debug_render_data_t {
    struct {
        v2f centre;
        v2f size;
        f32 rot;
    } rect;

    struct {
        v2f centre;
        f32 radius;
    } circle;
} debug_render_data_t;

typedef struct debug_render_call_t {
    debug_render_shape_t shape;
    debug_render_data_t data;
    debug_render_color_t color;
} debug_render_call_t;

void debug_render(debug_render_call_t call);
void debug_render_dispatch();

typedef struct debug_render_ctx_t {
    renderer_t renderer;
} debug_render_ctx_t;

extern debug_render_ctx_t debug_render_ctx;

void debug_render_init();
void debug_render_terminate();

#endif
