#ifndef DEFAULT_SHADER_H
#define DEFAULT_SHADER_H

#include "shader.h"

typedef struct {
    shader_s program;

    uniform_t u_layer;
    uniform_t u_projection_view;
    uniform_t u_color;
} default_shader_s;

void init_default_shader(default_shader_s* shader);

#endif
