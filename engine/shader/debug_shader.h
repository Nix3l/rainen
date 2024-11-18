#ifndef DEBUG_SHADER_H
#define DEBUG_SHADER_H

#include "shader.h"

typedef struct {
    shader_s program;

    uniform_t u_projection_view;
    uniform_t u_transformation;
    uniform_t u_color;
} debug_shader_s;

void init_debug_shader(debug_shader_s* shader);

#endif
