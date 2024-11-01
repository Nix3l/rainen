#include "text_shader.h"

#include "engine.h"
#include "util/log.h"

static void bind_attributes(shader_s* shader) {
    shader_bind_attribute(shader, MESH_ATTRIBUTE_VERTICES, "vs_position");
    shader_bind_attribute(shader, MESH_ATTRIBUTE_UVS, "vs_uvs");
}

static void load_uniforms(void* _call, void* _data) {
    draw_call_s* call = _call;
    text_shader_s* uniforms = &engine_state->text_shader;

    shader_load_mat4(uniforms->u_projection_view, call->camera->projection_view);
    shader_load_int(uniforms->u_tex, 0);
    shader_load_vec4(uniforms->u_color, call->color);
}

void init_text_shader(text_shader_s* shader) {
    shader->program = load_and_create_shader(
            "text",
            "shader/text_vs.glsl",
            "shader/text_fs.glsl",
            bind_attributes,
            load_uniforms,
            &engine_state->frame_arena
        );
    
    shader_s* program = &shader->program;
    shader->u_projection_view = shader_get_uniform(program, "projection_view");
    shader->u_tex             = shader_get_uniform(program, "tex");
    shader->u_color           = shader_get_uniform(program, "color");
}
