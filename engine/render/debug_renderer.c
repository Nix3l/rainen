#include "debug_renderer.h"
#include "engine.h"

void render_debug_rect(v2f position, v2f size, v3f col) {
    push_draw_call(
        engine_state->debug_group,
        &engine_state->primitive_square,
        NULL,
        position,
        0.0f,
        V2F(size.x / 2.0f, size.y / 2.0f),
        0.0f,
        0,
        V4F(col.x, col.y, col.z, 1.0f)
    );
}

void render_debug_line(v2f start, v2f end, f32 stroke, v3f col) {
    f32 hypotenuse = glms_vec2_distance(start, end);
    f32 opposite = end.y - start.y;
    f32 adjacent = end.x - start.x;
    f32 angle = atan2f(opposite, adjacent);

    push_draw_call(
        engine_state->debug_group,
        &engine_state->primitive_line,
        NULL,
        start,
        angle,
        V2F(hypotenuse, 1.0f),
        stroke,
        0,
        V4F(col.x, col.y, col.z, 1.0f)
    );
}

void render_debug_point(v2f position, f32 size, v3f col) {
    push_draw_call(
        engine_state->debug_group,
        &engine_state->primitive_point,
        NULL,
        position,
        0.0f,
        V2F_ONE,
        size,
        0,
        V4F(col.x, col.y, col.z, 1.0f)
    );
}
