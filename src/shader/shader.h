#ifndef SHADER_H
#define SHADER_H

#include "base.h"
#include "memory/memory.h"

// TODO(nix3l): is this the best way of doing compute shaders?
// they have a lot of similarities with normal shaders
// so it feels like theres a lot of redundant code but idk

typedef GLuint uniform_t;

typedef struct {
    // metadata
    char* name;
    char* vertex_full_path;
    char* fragment_full_path;

    GLuint program_id;

    // NOTE(nix3l): there might be no real reason to hold on to these after creation
    // doesnt hurt to keep them i guess
    // see create_shader in shader.c for more info
    GLuint vertex_id;
    GLuint fragment_id;

    void (*load_uniforms) (void*); // optional user data can be passed as a parameter
} shader_s;

typedef struct {
    v3i work_groups;

    GLuint program_id;
} compute_shader_s;

// takes in source code and compiles a shader accordingly
shader_s create_shader(
        char* name,
        char* vertex_src, char* fragment_src,
        void (*bind_attributes) (),
        void (*load_uniforms) (void*));

// takes in file paths on disk and compiles a shader accordingly
shader_s load_and_create_shader(
        char* name,
        char* vertex_path, char* fragment_path,
        void (*bind_attributes) (),
        void (*load_uniforms) (void*),
        arena_s* arena);

compute_shader_s create_compute_shader(char* src, v3i work_groups);
compute_shader_s load_and_create_compute_shader(char* src_path, v3i work_groups, arena_s* arena);

void destroy_shader(shader_s* shader);
void destroy_compute_shader(compute_shader_s* shader);

void shader_start(shader_s* shader);
void shader_stop();

void compute_shader_start(compute_shader_s* shader);
void compute_shader_stop();

void compute_shader_dispatch(compute_shader_s* shader);
void compute_shader_dispatch_groups(u32 x, u32 y, u32 z);

void shader_bind_attribute(shader_s* shader, GLuint attribute, char* attribute_name);

uniform_t shader_get_uniform(shader_s* shader, char* uniform_name);
uniform_t compute_shader_get_uniform(compute_shader_s* shader, char* uniform_name);
void shader_load_int(uniform_t uniform, i32 value);
void shader_load_uint(uniform_t uniform, u32 value);
void shader_load_float(uniform_t uniform, f32 value);
void shader_load_bool(uniform_t uniform, bool value);
void shader_load_vec2(uniform_t uniform, vec2s value);
void shader_load_vec3(uniform_t uniform, vec3s value);
void shader_load_ivec2(uniform_t uniform, ivec2s value);
void shader_load_ivec3(uniform_t uniform, ivec3s value);
void shader_load_mat4(uniform_t uniform, mat4s value);

// TODO(nix3l): make wrapper functions for glUniform*v()

#endif
