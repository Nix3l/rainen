#ifndef _GFX_H
#define _GFX_H

// TODO(nix3l):
//  => should i go only 2d, or also 3d? (i feel like 3d is a better choice)
//  => how should i go about doing the rendering? custom pipelines or without pipelines altogether?
//  => find a new way to do uniforms in shaders (the way i used to do it is very annoying)
//  => how to do compute passes? shader storage buffers?
//  => storing textures? meshes? (have to make as api agnostic as i can)
//
// essentially, make a full abstraction of what features the apis give me,
// so i can (if i ever need to) implement other apis
// i would like to eventually get into vulkan so that should be a priority here

#include "base.h"
#include "memory/memory.h"

#if OS_LINUX || OS_WINDOWS
#define GFX_SUPPORT_GL (1)
#endif

#ifndef GFX_SUPPORT_GL
#define GFX_SUPPORT_GL (0)
#endif

enum {
    // compile time constants/limits
    GFX_MAX_MESHES = 4096,
    GFX_MAX_VERTEX_ATTRIBS = 8,
};

typedef enum gfx_backend_t {
    GFX_BACKEND_GL = 0,
    GFX_BACKEND_NUM,
} gfx_backend_t;

typedef struct gfx_backend_info_t {
    gfx_backend_t backend;
    u32 version_major;
    u32 version_minor;
    const char version_str[16];
    const char name[8];
    const char name_pretty[16];
    bool supported;
    u32 mesh_id_size;
} gfx_backend_info_t;

// backend specific stuff goes here
typedef struct gl_mesh_info_t {
    u32 vao;
    u32 vbos[GFX_MAX_VERTEX_ATTRIBS];
    u32 index_vbo;
} gl_mesh_info_t;

// gfx lib context/state
typedef struct gfx_ctx_t {
    gfx_backend_t backend;
    gfx_backend_info_t backend_info[GFX_BACKEND_NUM];

    // in order to keep this as api agnostic as i can,
    // instead of storing the identifier/id given by the api,
    // i store it in this pool and retrieve it whenever necessary
    mempool_t* mesh_pool;
} gfx_ctx_t;

extern gfx_ctx_t gfx_ctx;

void gfx_init(gfx_backend_t backend);
void gfx_terminate();

gfx_backend_t gfx_backend();
gfx_backend_info_t gfx_backend_info();

typedef enum {
    // NOTE(nix3l): format goes as follows:
    // list the attributes with their dimensions in order
    // X2 -> position | 2D
    // X3 -> position | 3D
    // T2 -> uvs      | 2D
    // N3 -> normals  | 3D
    // C1 -> colours  | 1D, r/a
    // C2 -> colours  | 2D, rg/ga
    // C3 -> colours  | 3D, rgb
    // C4 -> colours  | 4D, rgba
    // etc...
    MESH_FORMAT_DEFAULT = 0,
    MESH_FORMAT_X3T2N3,
} mesh_format_t;

typedef enum mesh_index_type_t {
    MESH_INDEX_TYPE_DEFAULT = 0,
    MESH_INDEX_TYPE_NONE,
    MESH_INDEX_TYPE_32b,
} mesh_index_type_t;

typedef enum {
    MESH_PRIMITIVE_DEFAULT = 0,
    MESH_PRIMITIVE_TRIANGLES,
    MESH_PRIMITIVE_LINES,
    MESH_PRIMITIVE_POINTS,
} mesh_primitive_t;

typedef enum mesh_winding_order_t {
    MESH_WINDING_DEFAULT = 0,
    MESH_WINDING_CW,
    MESH_WINDING_CCW,
} mesh_winding_order_t;

typedef struct mesh_t {
    handle_t handle;
    mesh_format_t format;
    mesh_index_type_t index_type;
    mesh_primitive_t primitive;
    mesh_winding_order_t winding;
    u32 count; // either vertex count or index count depending on index_type
} mesh_t;

typedef struct mesh_attribute_t {
    u32 dimensions;
    range_t data;
} mesh_attribute_t;

typedef struct mesh_info_t {
    mesh_format_t format;
    mesh_index_type_t index_type;
    mesh_primitive_t primitive;
    mesh_winding_order_t winding;
    mesh_attribute_t attributes[GFX_MAX_VERTEX_ATTRIBS];
    range_t indices;
} mesh_info_t;

// for use in mesh_info_t
mesh_attribute_t mesh_attribute(void* data, u32 bytes, u32 dimensions);
// creates a new mesh
// if index_type is MESH_INDEX_TYPE_NONE, the indices range is ignored
// format is not checked against the supplied attributes, so make sure you specify the correct format
mesh_t mesh_new(mesh_info_t info);
// deletes the resources from gpu memory and makes the mesh unusable
// removes the mesh's id from the pool
void mesh_destroy(mesh_t* mesh);

#endif /* ifndef _GFX_H */
