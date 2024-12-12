#include "debug_shader.h"

#include "engine.h"

static void bind_attributes(shader_s* shader) {
    shader_bind_attribute(shader, MESH_ATTRIBUTE_VERTICES, "vs_position");
}

static void load_uniforms(void* _call, void* _unused) {
    draw_call_s* call = _call;

    debug_shader_s* shader = &engine->debug_shader;
    shader_load_mat4(shader->u_projection_view, call->camera->projection_view);
    shader_load_mat4(shader->u_transformation, call->transformation);
    shader_load_vec4(shader->u_color, call->color);
}

void init_debug_shader(debug_shader_s* shader) {
    shader->program = load_and_create_shader(
            "debug",
            "shader/debug_vs.glsl",
            "shader/debug_fs.glsl",
            bind_attributes,
            load_uniforms,
            &engine->frame_arena);

    shader_s* program = &shader->program;
    shader->u_projection_view = shader_get_uniform(program, "projection_view");
    shader->u_transformation  = shader_get_uniform(program, "transformation");
    shader->u_color           = shader_get_uniform(program, "color");
}
