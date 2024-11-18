#ifndef MESH_H
#define MESH_H

#include "base.h"
#include "memory/memory.h"

typedef enum {
    MESH_NONE = 0x00,
    MESH_VERTICES = 0x01,
    MESH_UVS = 0x02,
    MESH_NORMALS = 0x04,
    MESH_COLORS = 0x08,
    MESH_INDICES = 0x10,
} mesh_data_flags_t;

typedef enum {
    MESH_TRIANGLES = GL_TRIANGLES,
    MESH_LINES = GL_LINES,
    MESH_POINTS = GL_POINTS,
} mesh_primitive_t;

typedef enum {
    MESH_ATTRIBUTE_VERTICES = 0,
    MESH_ATTRIBUTE_UVS = 1,
    MESH_ATTRIBUTE_NORMALS = 2,
    MESH_ATTRIBUTE_COLORS = 3,
} mesh_attribute_t;

typedef struct {
    // data
    mesh_data_flags_t data;
    mesh_primitive_t primitive;

    u32 index_count; // only for indices
    u32 vertex_count; // same for all vertex data

    // opengl data
    GLuint vao;

    GLuint vertices_vbo;
    GLuint uvs_vbo;
    GLuint normals_vbo;
    GLuint colors_vbo;

    GLuint indices_vbo;
} mesh_s;

mesh_s create_mesh(
        f32* vertex_data, f32* uvs_data, f32* normals_data, f32* colors_data, 
        GLuint* indices,
        u32 num_indices, u32 num_vertices);

mesh_s create_mesh_arrays(
        f32* vertex_data, f32* uvs_data, f32* normals_data, f32* colors_data, 
        u32 num_vertices);

void mesh_enable_attributes(mesh_s* mesh);
void mesh_disable_attributes(mesh_s* mesh);

void destroy_mesh(mesh_s* mesh);

// LOADING FROM DISK
mesh_s load_mesh_from_file(char* filepath, arena_s* arena);

// PRIMITIVES
mesh_s primitive_unit_square();
mesh_s primitive_line();
mesh_s primitive_point();

// NOTE(nix3l): from old projects. dont really need but dont really have to throw away
#if 0
mesh_s primitive_plane_mesh(v3f bottom_left, v2i num_vertices, v2f world_size, arena_s* arena);
mesh_s primitive_cube_mesh();
#endif

#endif
