#include "default_shader.h"
#include "game.h"

static void bind_attributes(void* shader) {
    shader_bind_attribute(shader, 0, "vs_position");
    shader_bind_attribute(shader, 1, "vs_uvs");
}

static void load_uniforms(void* data) {
    draw_call_s* call = data;

    default_shader_s* uniforms = &game_state->default_shader;

    shader_load_int(uniforms->u_layer, call->layer);
    shader_load_mat4(uniforms->u_projection_view, game_state->camera.projection_view);
    shader_load_vec4(uniforms->u_color, call->color);
}

void init_default_shader(default_shader_s* shader) {
    shader->program = load_and_create_shader(
            "default",
            "shader/vs.glsl",
            "shader/fs.glsl",
            bind_attributes,
            load_uniforms,
            &game_state->frame_arena
        );
    
    shader_s* program = &shader->program;
    shader->u_layer           = shader_get_uniform(program, "z_layer");
    shader->u_projection_view = shader_get_uniform(program, "projection_view");
    shader->u_color           = shader_get_uniform(program, "color");
}
