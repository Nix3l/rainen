#ifndef _GFX_H
#define _GFX_H

#include "base.h"
#include "memory/memory.h"

#if OS_LINUX || OS_WINDOWS
#define GFX_SUPPORT_GL (1)
#endif

#ifndef GFX_SUPPORT_GL
#define GFX_SUPPORT_GL (0)
#endif

// TODO(nix3l):
//  => redo resource pool system

#define GFX_INVALID_ID (0)

enum {
    // compile time constants/limits
    GFX_MAX_MESHES = 4096,
    GFX_MAX_VERTEX_ATTRIBS = 8,
    GFX_MAX_TEXTURES = 256, // TODO(nix3l): change this
    GFX_MAX_SAMPLERS = 64,
    GFX_MAX_SAMPLER_SLOTS = 16,
    GFX_MAX_ATTACHMENT_OBJECTS = 8,
    GFX_MAX_COLOUR_ATTACHMENTS = 8,
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
    u32 attachments_internal_size;
    u32 shader_internal_size;
} gfx_backend_info_t;

// each object comes with alloc,init,discard,destroy,new functions
//  alloc   => reserve a data slot for the object
//  init    => initialize the object data & gpu data
//  discard => remove the gpu data associated with an object
//  destroy => discard the gpu data & free the data slot for the object
//  new     => combination of alloc & init

// ids point to resource slot
typedef struct mesh_t        { handle_t id; } mesh_t;
typedef struct texture_t     { handle_t id; } texture_t;
typedef struct sampler_t     { handle_t id; } sampler_t;
typedef struct attachments_t { handle_t id; } attachments_t;
typedef struct shader_t      { handle_t id; } shader_t;

typedef enum gfx_res_state_t {
    GFX_RES_STATE_FREE  = 0,
    GFX_RES_STATE_ALLOC = 1,
    GFX_RES_STATE_INIT  = 2,
    // TODO: valid? discarded?
} gfx_res_state_t;

typedef struct gfx_res_slot_t {
    gfx_res_state_t state;
    handle_t data_handle;
    handle_t internal_handle;
} gfx_res_slot_t;

typedef struct gfx_respool_t {
    u32 capacity;
    pool_t res_pool;
    pool_t data_pool;
    pool_t internal_pool;
} gfx_respool_t;

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
    MESH_FORMAT_X2,
    MESH_FORMAT_X2T2,
    MESH_FORMAT_X3T2N3,
} mesh_format_t;

typedef enum mesh_index_type_t {
    MESH_INDEX_UNDEFINED = 0,
    MESH_INDEX_NONE,
    MESH_INDEX_32b,
} mesh_index_type_t;

typedef enum {
    MESH_PRIMITIVE_UNDEFINED = 0, // will be assumed triangles
    MESH_PRIMITIVE_TRIANGLES,
    MESH_PRIMITIVE_LINES,
    MESH_PRIMITIVE_POINTS,
} mesh_primitive_t;

typedef enum mesh_winding_order_t {
    MESH_WINDING_UNDEFINED = 0, // will be assumed CCW
    MESH_WINDING_CW,
    MESH_WINDING_CCW,
} mesh_winding_order_t;

typedef struct mesh_data_t {
    mesh_format_t format;
    mesh_index_type_t index_type;
    mesh_primitive_t primitive;
    mesh_winding_order_t winding;
    u32 count; // either vertex count or index count depending on index_type
} mesh_data_t;

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
    u32 count;
} mesh_info_t;

// for use in mesh_info_t
mesh_attribute_t mesh_attribute(void* data, u32 bytes, u32 dimensions);

mesh_t mesh_alloc();
void mesh_init(mesh_t mesh, mesh_info_t info);
void mesh_discard(mesh_t mesh);
void mesh_destroy(mesh_t mesh);

// creates a new mesh
// if index_type is MESH_INDEX_TYPE_NONE, the indices range is ignored, and vertex_count *must* be supplied
// if index_type is undefined, it is implied using the indices range
// format is not checked against the supplied attributes, so make sure you specify the correct format
// format *must* be supplied
// if primitive not supplied, assumed to be triangles
// if winding order not supplied, assumed to be CCW
mesh_t mesh_new(mesh_info_t info);

mesh_data_t* mesh_get_data(mesh_t mesh);

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
    TEXTURE_WRAP_UNDEFINED = 0, // will be assumed clamp to edge
    TEXTURE_WRAP_REPEAT,
    TEXTURE_WRAP_MIRRORED_REPEAT,
    TEXTURE_WRAP_CLAMP_TO_EDGE,
} texture_wrap_t;

typedef struct texture_data_t {
    texture_type_t type;
    texture_format_t format;
    u32 width;
    u32 height;
    u32 mipmaps;
} texture_data_t;

typedef struct texture_info_t {
    texture_type_t type;
    texture_format_t format;
    u32 width;
    u32 height;
    u32 mipmaps;
    range_t data;
} texture_info_t;

texture_t texture_alloc();
void texture_init(texture_t texture, texture_info_t info);
void texture_discard(texture_t texture);
void texture_destroy(texture_t texture);
texture_t texture_new(texture_info_t info);

texture_data_t* texture_get_data(texture_t texture);

// SAMPLERS
typedef struct sampler_data_t {
    texture_filter_t min_filter;
    texture_filter_t mag_filter;
    texture_wrap_t u_wrap;
    texture_wrap_t v_wrap;
} sampler_data_t;

typedef struct sampler_info_t {
    texture_filter_t filter;
    texture_filter_t min_filter;
    texture_filter_t mag_filter;
    texture_wrap_t wrap;
    texture_wrap_t u_wrap;
    texture_wrap_t v_wrap;
} sampler_info_t;

sampler_t sampler_alloc();
void sampler_init(sampler_t sampler, sampler_info_t info);
void sampler_discard(sampler_t sampler);
void sampler_destroy(sampler_t sampler);

// if filter is defined, min_filter and mag_filter are ignored
// if wrap is defined, u_wrap and v_wrap are ignored
sampler_t sampler_new(sampler_info_t info);

sampler_data_t* sampler_get_data(sampler_t sampler);

// RENDER ATTACHMENTS
typedef struct attachments_data_t {
    u32 num_colours;
    texture_t colours[GFX_MAX_COLOUR_ATTACHMENTS];
    texture_t depth_stencil;
} attachments_data_t;

typedef struct attachments_info_t {
    texture_t colours[GFX_MAX_COLOUR_ATTACHMENTS];
    texture_t depth_stencil;
} attachments_info_t;

attachments_t attachments_alloc();
void attachments_init(attachments_t attachments, attachments_info_t info);
void attachments_discard(attachments_t attachments);
void attachments_destroy(attachments_t attachments);

attachments_t attachments_new(attachments_info_t info);

attachments_data_t* attachments_get_data(attachments_t attachments);

// TODO(nix3l)
void attachments_clear_colour(v4f col);
void attachments_clear_depth_stencil();

void attachments_blit_colour(attachments_t dest, attachments_t src, u32 dest_att, u32 src_att);
void attachments_blit_depth_stencil(attachments_t dest, attachments_t src);

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

typedef struct shader_data_t {
    char name[8];
    char pretty_name[16];
    shader_pass_t vertex_pass;
    shader_pass_t fragment_pass;
    // TODO(nix3l): compute
    shader_vertex_attribute_t attribs[GFX_MAX_VERTEX_ATTRIBS];
    uniform_block_t uniform_block;
} shader_data_t;

typedef struct shader_info_t {
    char name[8];
    char pretty_name[16];
    range_t vertex_src;
    range_t fragment_src;
    shader_vertex_attribute_t attribs[GFX_MAX_VERTEX_ATTRIBS];
    uniform_t uniforms[GFX_MAX_UNIFORMS];
} shader_info_t;

shader_t shader_alloc();
void shader_init(shader_t shader, shader_info_t info);
void shader_discard(shader_t shader);
void shader_destroy(shader_t shader);

shader_t shader_new(shader_info_t info);

shader_data_t* shader_get_data(shader_t shader);

// updates the shader's uniforms with the given data
// all uniforms must be updated at once
// data struct should be identical to uniform struct in shader
void shader_update_uniforms(shader_t shader, range_t data);

// RENDERING
typedef struct sampler_slot_t {
    texture_t texture;
    sampler_t sampler;
} sampler_slot_t;

typedef struct render_bindings_t {
    mesh_t mesh;
    sampler_slot_t texture_samplers[GFX_MAX_SAMPLER_SLOTS];
} render_bindings_t;

typedef struct render_target_t {
    bool enable;
    bool disable_clear;
    bool override_clear_col;
    v4f clear_col; // overrides the default clear colour in the pipeline
} render_target_t;

typedef enum cull_face_t {
    CULL_FACE_UNDEFINED = 0, // will be assumed back
    CULL_FACE_FRONT,
    CULL_FACE_BACK,
    CULL_FACE_FRONT_AND_BACK,
} cull_face_t;

typedef struct render_cull_state_t {
    bool enable;
    cull_face_t face;
} render_cull_state_t;

typedef enum depth_func_t {
    DEPTH_FUNC_UNDEFINED = 0, // will be assumed less
    DEPTH_FUNC_NEVER,
    DEPTH_FUNC_ALWAYS,
    DEPTH_FUNC_LESS,
    DEPTH_FUNC_GREATER,
    DEPTH_FUNC_LESS_EQUAL,
    DEPTH_FUNC_GREATER_EQUAL,
    DEPTH_FUNC_EQUAL,
    DEPTH_FUNC_NOT_EQUAL,
} depth_func_t;

typedef struct render_depth_state_t {
    bool enable;
    depth_func_t func;
} render_depth_state_t;

typedef enum blend_func_t {
    BLEND_FUNC_UNDEFINED = 0,
    BLEND_FUNC_ZERO,
    BLEND_FUNC_ONE,
    BLEND_FUNC_SRC_COLOUR,
    BLEND_FUNC_SRC_ONE_MINUS_COLOUR,
    BLEND_FUNC_SRC_ALPHA,
    BLEND_FUNC_SRC_ONE_MINUS_ALPHA,
    BLEND_FUNC_DST_COLOUR,
    BLEND_FUNC_DST_ONE_MINUS_COLOUR,
    BLEND_FUNC_DST_ALPHA,
    BLEND_FUNC_DST_ONE_MINUS_ALPHA,
} blend_func_t;

typedef struct render_blend_state_t {
    bool enable;
    blend_func_t src_func;
    blend_func_t dst_func;
} render_blend_state_t;

typedef struct render_clear_state_t {
    bool depth;
    bool stencil;
    bool colour;
    v4f clear_col; // default clear colour for all render targets
} render_clear_state_t;

typedef struct render_pipeline_t {
    render_depth_state_t depth;
    render_cull_state_t cull;
    render_blend_state_t blend;
    render_clear_state_t clear;
    attachments_t draw_attachments;
    render_target_t colour_targets[GFX_MAX_COLOUR_ATTACHMENTS];
    shader_t shader;
} render_pipeline_t;

void gfx_activate_pipeline(render_pipeline_t pipeline);
void gfx_supply_bindings(render_bindings_t bindings);
void gfx_draw();

// VIEWPORT
void gfx_viewport(u32 x, u32 y, u32 w, u32 h);

// CONTEXT
typedef struct gfx_ctx_t {
    gfx_backend_t backend;
    gfx_backend_info_t backend_info[GFX_BACKEND_NUM];

    render_bindings_t active_bindings;
    render_pipeline_t active_pipeline;

    gfx_respool_t* mesh_pool;
    gfx_respool_t* texture_pool;
    gfx_respool_t* sampler_pool;
    gfx_respool_t* attachments_pool;
    gfx_respool_t* shader_pool;
} gfx_ctx_t;

extern gfx_ctx_t gfx_ctx;

#endif /* ifndef _GFX_H */
