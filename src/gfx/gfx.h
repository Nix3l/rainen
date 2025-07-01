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

// TODO(nix3l):
//  => better gfx resource handling (alloc,init,deinit,destroy)

enum {
    // compile time constants/limits
    GFX_MAX_MESHES = 4096,
    GFX_MAX_VERTEX_ATTRIBS = 8,
    GFX_MAX_TEXTURES = 256, // TODO(nix3l): change this
    GFX_MAX_SAMPLERS = 32,
    GFX_MAX_SHADERS = 8,
    GFX_MAX_UNIFORMS = 64,
};

typedef enum gfx_backend_t {
    GFX_BACKEND_INVALID = 0,
    GFX_BACKEND_GL,
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
    u32 mesh_internal_size;
    u32 texture_internal_size;
    u32 sampler_internal_size;
    u32 shader_internal_size;
} gfx_backend_info_t;

// backend specific stuff goes here
typedef struct gl_mesh_info_t {
    u32 vao;
    u32 vbos[GFX_MAX_VERTEX_ATTRIBS];
    u32 index_vbo;
} gl_mesh_info_t;

typedef struct gl_texture_info_t {
    u32 id;
} gl_texture_info_t;

typedef struct gl_sampler_info_t {
    u32 id;
} gl_sampler_info_t;

typedef struct gl_shader_info_t {
    u32 program;
} gl_shader_info_t;

// gfx lib context/state
typedef struct gfx_ctx_t {
    gfx_backend_t backend;
    gfx_backend_info_t backend_info[GFX_BACKEND_NUM];

    // in order to keep this as api agnostic as i can,
    // instead of storing the identifier/id given by the api,
    // i store it in this pool and retrieve it whenever necessary
    pool_t* mesh_pool;
    pool_t* texture_pool;
    pool_t* sampler_pool;
    pool_t* shader_pool;
} gfx_ctx_t;

extern gfx_ctx_t gfx_ctx;

void gfx_init(gfx_backend_t backend);
void gfx_terminate();

gfx_backend_t gfx_backend();
gfx_backend_info_t gfx_backend_info();

// MESH
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
    MESH_FORMAT_INVALID = 0,
    MESH_FORMAT_X3T2N3,
} mesh_format_t;

typedef enum mesh_index_type_t {
    MESH_INDEX_TYPE_UNDEFINED = 0,
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
    handle_t gfx_handle;
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
    u32 vertex_count;
} mesh_info_t;

// for use in mesh_info_t
mesh_attribute_t mesh_attribute(void* data, u32 bytes, u32 dimensions);

// creates a new mesh
// if index_type is MESH_INDEX_TYPE_NONE, the indices range is ignored, and vertex_count *must* be supplied
// if index_type is undefined, it is implied using the indices range
// format is not checked against the supplied attributes, so make sure you specify the correct format
// format *must* be supplied
// if primitive not supplied, assumed to be triangles
// if winding order not supplied, assumed to be CCW
mesh_t mesh_new(mesh_info_t info);

// deletes the resources from gpu memory and makes the mesh unusable
// removes the mesh's gfx info from the pool
void mesh_destroy(mesh_t* mesh);

// TEXTURE
// NOTE(nix3l): be careful when updating this, might break some internal translation functions
typedef enum texture_format_t {
    TEXTURE_FORMAT_UNDEFINED = 0,

    // one channel
    TEXTURE_FORMAT_R8,
    TEXTURE_FORMAT_R8I,
    TEXTURE_FORMAT_R8UI,

    TEXTURE_FORMAT_R16,
    TEXTURE_FORMAT_R16F,
    TEXTURE_FORMAT_R16I,
    TEXTURE_FORMAT_R16UI,

    TEXTURE_FORMAT_R32F,
    TEXTURE_FORMAT_R32I,
    TEXTURE_FORMAT_R32UI,

    // 2 channels
    TEXTURE_FORMAT_RG8,
    TEXTURE_FORMAT_RG8I,
    TEXTURE_FORMAT_RG8UI,

    TEXTURE_FORMAT_RG16,
    TEXTURE_FORMAT_RG16F,
    TEXTURE_FORMAT_RG16I,
    TEXTURE_FORMAT_RG16UI,

    TEXTURE_FORMAT_RG32F,
    TEXTURE_FORMAT_RG32I,
    TEXTURE_FORMAT_RG32UI,

    // 3 channels
    TEXTURE_FORMAT_RGB8,
    TEXTURE_FORMAT_RGB8I,
    TEXTURE_FORMAT_RGB8UI,

    TEXTURE_FORMAT_RGB16F,
    TEXTURE_FORMAT_RGB16I,
    TEXTURE_FORMAT_RGB16UI,

    TEXTURE_FORMAT_RGB32F,
    TEXTURE_FORMAT_RGB32I,
    TEXTURE_FORMAT_RGB32UI,

    // 4 channels
    TEXTURE_FORMAT_RGBA8,
    TEXTURE_FORMAT_RGBA8I,
    TEXTURE_FORMAT_RGBA8UI,

    TEXTURE_FORMAT_RGBA16F,
    TEXTURE_FORMAT_RGBA16I,
    TEXTURE_FORMAT_RGBA16UI,

    TEXTURE_FORMAT_RGBA32F,
    TEXTURE_FORMAT_RGBA32I,
    TEXTURE_FORMAT_RGBA32UI,

    // depth/stencil
    TEXTURE_FORMAT_DEPTH,
    TEXTURE_FORMAT_DEPTH_STENCIL,
} texture_format_t;

typedef enum texture_type_t {
    TEXTURE_TYPE_UNDEFINED = 0, // will be assumed 2D
    TEXTURE_TYPE_2D,
} texture_type_t;

typedef enum texture_filter_t {
    TEXTURE_FILTER_UNDEFINED = 0, // will be assumed nearest
    TEXTURE_FILTER_NEAREST,
    TEXTURE_FILTER_LINEAR,
} texture_filter_t;

typedef enum texture_wrap_t {
    TEXTURE_WRAP_UNDEFINED = 0,
    TEXTURE_WRAP_REPEAT,
    TEXTURE_WRAP_MIRRORED_REPEAT,
    TEXTURE_WRAP_CLAMP_TO_EDGE,
} texture_wrap_t;

typedef struct texture_t {
    handle_t gfx_handle;
    texture_type_t type;
    texture_format_t format;
    u32 width;
    u32 height;
    u32 mipmaps;
} texture_t;

typedef struct texture_info_t {
    texture_type_t type;
    texture_format_t format;
    u32 width;
    u32 height;
    u32 mipmaps;
    range_t data;
} texture_info_t;

texture_t texture_new(texture_info_t info);
void texture_destroy(texture_t* texture);

// SAMPLERS
typedef struct sampler_t {
    handle_t gfx_handle;
    texture_filter_t min_filter;
    texture_filter_t mag_filter;
    texture_wrap_t u_wrap;
    texture_wrap_t v_wrap;
} sampler_t;

typedef struct sampler_info_t {
    texture_filter_t filter;
    texture_filter_t min_filter;
    texture_filter_t mag_filter;
    texture_wrap_t wrap;
    texture_wrap_t u_wrap;
    texture_wrap_t v_wrap;
} sampler_info_t;

// if filter is defined, min_filter and mag_filter are ignored
// if wrap is defined, u_wrap and v_wrap are ignored
sampler_t sampler_new(sampler_info_t info);

void sampler_destroy(sampler_t* sampler);

// SHADER
typedef struct shader_vertex_attribute_t {
    const char* name;
} shader_vertex_attribute_t;

typedef enum uniform_type_t {
    UNIFORM_TYPE_INVALID = 0,
    UNIFORM_TYPE_i32,
    UNIFORM_TYPE_u32,
    UNIFORM_TYPE_f32,
    UNIFORM_TYPE_v2f,
    UNIFORM_TYPE_v3f,
    UNIFORM_TYPE_v4f,
    UNIFORM_TYPE_v2i,
    UNIFORM_TYPE_v3i,
    UNIFORM_TYPE_v4i,
    UNIFORM_TYPE_mat4,
} uniform_type_t;

// TODO(nix3l): uniform array support
typedef struct uniform_t {
    const char* name;
    u32 glid;
    uniform_type_t type;
} uniform_t;

typedef struct uniform_block_t {
    u32 num;
    uniform_t uniforms[GFX_MAX_UNIFORMS];
} uniform_block_t;

typedef enum shader_pass_type_t {
    SHADER_PASS_INVALID = 0,
    SHADER_PASS_VERTEX,
    SHADER_PASS_FRAGMENT,
    SHADER_PASS_COMPUTE,
} shader_pass_type_t;

typedef struct shader_pass_t {
    shader_pass_type_t type;
    range_t src;
} shader_pass_t;

typedef struct shader_t {
    handle_t gfx_handle;
    char name[8];
    char pretty_name[16];
    shader_pass_t vertex_pass;
    shader_pass_t fragment_pass;
    // TODO(nix3l): compute
    shader_vertex_attribute_t attribs[GFX_MAX_VERTEX_ATTRIBS];
    uniform_block_t uniform_block;
} shader_t;

typedef struct shader_info_t {
    char name[8];
    char pretty_name[16];
    range_t vertex_src;
    range_t fragment_src;
    shader_vertex_attribute_t attribs[GFX_MAX_VERTEX_ATTRIBS];
    uniform_t uniforms[GFX_MAX_UNIFORMS];
} shader_info_t;

shader_t shader_new(shader_info_t info);

void shader_destroy(shader_t* shader);

// updates the shader's uniforms with the given data
// all uniforms must be updated at once
// data struct should be identical to uniform struct in shader
void shader_update_uniforms(shader_t* shader, range_t data);

#endif /* ifndef _GFX_H */
