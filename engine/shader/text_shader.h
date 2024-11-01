#ifndef TEXT_SHADER_H
#define TEXT_SHADER_H

#include "shader.h"

typedef struct {
    shader_s program;

    uniform_t u_projection_view;
    uniform_t u_tex;
    uniform_t u_color;
} text_shader_s;

void init_text_shader(text_shader_s* shader);

#endif
