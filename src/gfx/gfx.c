#include "gfx.h"
#include "base.h"
#include "memory/memory.h"
#include "util/util.h"

gfx_ctx_t gfx_ctx;

// start of black magic --------------

// BACKEND_FUNC_XMACRO(function name, parameters)
// will get expanded into all the necessary function definitions
#define BACKEND_FUNCS_LIST \
    BACKEND_FUNC_XMACRO(mesh_init, mesh_data_t* mesh, mesh_info_t info) \
    BACKEND_FUNC_XMACRO(mesh_destroy, mesh_data_t* mesh) \
    BACKEND_FUNC_XMACRO(texture_init, texture_data_t* texture, texture_info_t info) \
    BACKEND_FUNC_XMACRO(texture_destroy, texture_data_t* texture) \
    BACKEND_FUNC_XMACRO(sampler_init, sampler_data_t* sampler, sampler_info_t info) \
    BACKEND_FUNC_XMACRO(sampler_destroy, sampler_data_t* sampler) \
    BACKEND_FUNC_XMACRO(attachments_init, attachments_data_t* attachments, attachments_info_t info) \
    BACKEND_FUNC_XMACRO(attachments_destroy, attachments_data_t* attachments) \
    BACKEND_FUNC_XMACRO(shader_init, shader_data_t* shader, shader_info_t info) \
    BACKEND_FUNC_XMACRO(shader_destroy, shader_data_t* shader) \
    BACKEND_FUNC_XMACRO(shader_update_uniforms, shader_data_t* shader, range_t uniforms) \
    BACKEND_FUNC_XMACRO(activate_pipeline, render_pipeline_t pipeline) \
    BACKEND_FUNC_XMACRO(activate_bindings, render_bindings_t bindings) \
    BACKEND_FUNC_XMACRO(draw, mesh_t mesh) \
    BACKEND_FUNC_XMACRO(viewport, u32 x, u32 y, u32 w, u32 h) \

#define BACKEND_FUNC_XMACRO(_name, ...) typedef void (*_name ## _func) (__VA_ARGS__);
BACKEND_FUNCS_LIST;
#undef BACKEND_FUNC_XMACRO

#define BACKEND_FUNC_XMACRO(_name, ...) _name ## _func _name;
typedef struct backend_jumptable_t {
    BACKEND_FUNCS_LIST;
} backend_jumptable_t;
#undef BACKEND_FUNC_XMACRO

static backend_jumptable_t backend_jumptables[GFX_BACKEND_NUM] = {0};
static backend_jumptable_t* backend = NULL;

// define backend-specific functions
// gl
#define BACKEND_FUNC_XMACRO(_name, ...) static void gl_ ## _name(__VA_ARGS__);
BACKEND_FUNCS_LIST;
#undef BACKEND_FUNC_XMACRO

static void init_jumptables() {
    #define BACKEND_FUNC_XMACRO(_name, ...) ._name = gl_ ## _name,
    backend_jumptables[GFX_BACKEND_GL] = (backend_jumptable_t) {
        BACKEND_FUNCS_LIST
    };
    #undef BACKEND_FUNC_XMACRO

    backend = &backend_jumptables[gfx_ctx.backend];
}

#undef BACKEND_FUNCS_LIST
#undef BACKEND_FUNC_XMACRO

// end of black magic ----------------

static gfx_respool_t mesh_pool;
static gfx_respool_t texture_pool;
static gfx_respool_t sampler_pool;
static gfx_respool_t attachments_pool;
static gfx_respool_t shader_pool;

static gfx_respool_t gfx_respool_alloc(u32 capacity, u32 res_bytes, u32 internal_bytes) {
    gfx_respool_t pool = (gfx_respool_t) {
        .capacity = capacity,
        .data_pool = pool_alloc(capacity, res_bytes, EXPAND_TYPE_IMMUTABLE),
        .gfx_pool = pool_alloc(capacity, internal_bytes, EXPAND_TYPE_IMMUTABLE),
    };

    // reserve the 0 index for invalid ids
    (void) pool_push(&pool.data_pool, NULL);
    (void) pool_push(&pool.gfx_pool, NULL);

    return pool;
}

// TODO(nix3l): inline these?
static void* gfx_respool_push_data(gfx_respool_t* pool, handle_t* slot) {
    return pool_push(&pool->data_pool, slot);
}

static void* gfx_respool_push_internal(gfx_respool_t* pool, handle_t* slot) {
    return pool_push(&pool->gfx_pool, slot);
}

static void* gfx_respool_data(gfx_respool_t* pool, handle_t slot) {
    return pool_get(&pool->data_pool, slot);
}

static void* gfx_respool_internal(gfx_respool_t* pool, handle_t slot) {
    return pool_get(&pool->gfx_pool, slot);
}

static void gfx_respool_free_data(gfx_respool_t* pool, handle_t slot) {
    pool_free(&pool->data_pool, slot);
}

static void gfx_respool_free_internal(gfx_respool_t* pool, handle_t slot) {
    pool_free(&pool->gfx_pool, slot);
}

static void gfx_respool_destroy(gfx_respool_t* pool) {
    pool->capacity = 0;
    pool_destroy(&pool->data_pool);
    pool_destroy(&pool->gfx_pool);
}

void gfx_init(gfx_backend_t backend) {
    gfx_backend_info_t opengl_core_info = (gfx_backend_info_t) {
        .backend = GFX_BACKEND_GL,
        .version_major = 4,
        .version_minor = 3,
        .version_str = "4.3",
        .name = "glcore",
        .name_pretty = "OpenGL",
        .supported = GFX_SUPPORT_GL,
        .mesh_internal_size = sizeof(gl_mesh_internal_t),
        .texture_internal_size = sizeof(gl_texture_internal_t),
        .sampler_internal_size = sizeof(gl_sampler_internal_t),
        .attachments_internal_size = sizeof(gl_attachments_internal_t),
        .shader_internal_size = sizeof(gl_shader_internal_t),
    };

    gfx_ctx = (gfx_ctx_t) {
        .backend = backend,
        .backend_info = {
            (gfx_backend_info_t) {0},
            opengl_core_info,
        },
    };

    gfx_backend_info_t curr_backend_info = gfx_ctx.backend_info[backend];
    if(!curr_backend_info.supported) PANIC("chosen backend is not supported on current OS\n");

    // for now, keep immutable
    mesh_pool = gfx_respool_alloc(GFX_MAX_MESHES, sizeof(mesh_data_t), curr_backend_info.mesh_internal_size);
    gfx_ctx.mesh_pool = &mesh_pool;

    texture_pool = gfx_respool_alloc(GFX_MAX_TEXTURES, sizeof(texture_data_t), curr_backend_info.texture_internal_size);
    gfx_ctx.texture_pool = &texture_pool;

    sampler_pool = gfx_respool_alloc(GFX_MAX_SAMPLERS, sizeof(sampler_data_t), curr_backend_info.sampler_internal_size);
    gfx_ctx.sampler_pool = &sampler_pool;

    attachments_pool = gfx_respool_alloc(GFX_MAX_ATTACHMENT_OBJECTS, sizeof(attachments_data_t), curr_backend_info.attachments_internal_size);
    gfx_ctx.attachments_pool = &attachments_pool;

    shader_pool = gfx_respool_alloc(GFX_MAX_SHADERS, sizeof(shader_data_t), curr_backend_info.shader_internal_size);
    gfx_ctx.shader_pool = &shader_pool;

    init_jumptables();
}

void gfx_terminate() {
    gfx_respool_destroy(gfx_ctx.mesh_pool);
    gfx_respool_destroy(gfx_ctx.texture_pool);
    gfx_respool_destroy(gfx_ctx.sampler_pool);
    gfx_respool_destroy(gfx_ctx.shader_pool);
}

gfx_backend_t gfx_backend() {
    return gfx_ctx.backend;
}

gfx_backend_info_t gfx_backend_info() {
    return gfx_ctx.backend_info[gfx_ctx.backend];
}

// API
static u32 mesh_format_num_attributes(mesh_format_t format) {
    switch(format) {
        case MESH_FORMAT_X2: return 1;
        case MESH_FORMAT_X2T2: return 2;
        case MESH_FORMAT_X3T2N3: return 3;
        default: UNREACHABLE; return 0;
    }
}

mesh_attribute_t mesh_attribute(void* data, u32 size, u32 dimensions) {
    return (mesh_attribute_t) {
        .dimensions = dimensions,
        .data = range_new(data, size),
    };
}

mesh_t mesh_alloc() {
    mesh_t mesh = {0};
    mesh_data_t* mesh_data = gfx_respool_push_data(gfx_ctx.mesh_pool, &mesh.id);
    mem_clear(mesh_data, sizeof(mesh_data_t));
    return mesh;
}

void mesh_init(mesh_t mesh, mesh_info_t info) {
    mesh_data_t* mesh_data = gfx_respool_data(gfx_ctx.mesh_pool, mesh.id);

    if(info.format == MESH_FORMAT_INVALID) {
        LOG_ERR("mesh format *must* be supplied\n");
        return;
    }

    if(info.index_type == MESH_INDEX_UNDEFINED)
        info.index_type = info.indices.size == 0 ? MESH_INDEX_NONE : MESH_INDEX_32b;

    if(info.primitive == MESH_PRIMITIVE_UNDEFINED)
        info.primitive = MESH_PRIMITIVE_TRIANGLES;

    if(info.winding == MESH_WINDING_UNDEFINED)
        info.winding = MESH_WINDING_CCW;

    mesh_data->format = info.format;
    mesh_data->index_type = info.index_type;
    mesh_data->primitive = info.primitive;
    mesh_data->winding = info.winding;
    mesh_data->count = info.count;

    backend->mesh_init(mesh_data, info);
}

void mesh_discard(mesh_t mesh) {
    mesh_data_t* mesh_data = gfx_respool_data(gfx_ctx.mesh_pool, mesh.id);
    backend->mesh_destroy(mesh_data);
    gfx_respool_free_internal(gfx_ctx.mesh_pool, mesh_data->internal);
}

void mesh_destroy(mesh_t mesh) {
    mesh_discard(mesh);
    gfx_respool_free_data(gfx_ctx.mesh_pool, mesh.id);
}

mesh_t mesh_new(mesh_info_t info) {
    mesh_t mesh = mesh_alloc();
    mesh_init(mesh, info);
    return mesh;
}

mesh_data_t* mesh_query_data(mesh_t mesh) {
    return gfx_respool_data(gfx_ctx.mesh_pool, mesh.id);
}

static void* mesh_internal(mesh_t mesh) {
    mesh_data_t* mesh_data = mesh_query_data(mesh);
    return gfx_respool_internal(gfx_ctx.mesh_pool, mesh_data->internal);
}

texture_t texture_alloc() {
    texture_t texture = {0};
    texture_data_t* texture_data = gfx_respool_push_data(gfx_ctx.texture_pool, &texture.id);
    mem_clear(texture_data, sizeof(texture_data_t));
    return texture;
}

void texture_init(texture_t texture, texture_info_t info) {
    texture_data_t* texture_data = gfx_respool_data(gfx_ctx.texture_pool, texture.id);

    if(info.type == TEXTURE_TYPE_UNDEFINED) info.type = TEXTURE_TYPE_2D;
    if(info.mipmaps == 0) info.mipmaps = 1;

    texture_data->type = info.type;
    texture_data->format = info.format;
    texture_data->width = info.width;
    texture_data->height = info.height;
    texture_data->mipmaps = info.mipmaps;

    backend->texture_init(texture_data, info);
}

void texture_discard(texture_t texture) {
    texture_data_t* texture_data = gfx_respool_data(gfx_ctx.texture_pool, texture.id);
    backend->texture_destroy(texture_data);
    gfx_respool_free_internal(gfx_ctx.texture_pool, texture_data->internal);
}

void texture_destroy(texture_t texture) {
    texture_discard(texture);
    gfx_respool_free_data(gfx_ctx.texture_pool, texture.id);
}

texture_t texture_new(texture_info_t info) {
    texture_t texture = texture_alloc();
    texture_init(texture, info);
    return texture;
}

texture_data_t* texture_query_data(texture_t texture) {
    return gfx_respool_data(gfx_ctx.texture_pool, texture.id);
}

static void* texture_internal(texture_t texture) {
    texture_data_t* texture_data = texture_query_data(texture);
    return gfx_respool_internal(gfx_ctx.texture_pool, texture_data->internal);
}

sampler_t sampler_alloc() {
    sampler_t sampler = {0};
    sampler_data_t* sampler_data = gfx_respool_push_data(gfx_ctx.sampler_pool, &sampler.id);
    mem_clear(sampler_data, sizeof(sampler_data_t));
    return sampler;
}

void sampler_init(sampler_t sampler, sampler_info_t info) {
    sampler_data_t* sampler_data = gfx_respool_data(gfx_ctx.sampler_pool, sampler.id);

    if(info.wrap != TEXTURE_WRAP_UNDEFINED) {
        info.u_wrap = info.wrap;
        info.v_wrap = info.wrap;
    }

    if(info.filter != TEXTURE_FILTER_UNDEFINED) {
        info.min_filter = info.filter;
        info.mag_filter = info.filter;
    }

    sampler_data->u_wrap = info.u_wrap;
    sampler_data->v_wrap = info.v_wrap;
    sampler_data->min_filter = info.min_filter;
    sampler_data->mag_filter = info.mag_filter;

    backend->sampler_init(sampler_data, info);
}

void sampler_discard(sampler_t sampler) {
    sampler_data_t* sampler_data = gfx_respool_data(gfx_ctx.sampler_pool, sampler.id);
    backend->sampler_destroy(sampler_data);
    gfx_respool_free_internal(gfx_ctx.sampler_pool, sampler_data->internal);
}

void sampler_destroy(sampler_t sampler) {
    sampler_discard(sampler);
    gfx_respool_free_data(gfx_ctx.sampler_pool, sampler.id);
}

sampler_t sampler_new(sampler_info_t info) {
    sampler_t sampler = sampler_alloc();
    sampler_init(sampler, info);
    return sampler;
}

sampler_data_t* sampler_query_data(sampler_t sampler) {
    return gfx_respool_data(gfx_ctx.sampler_pool, sampler.id);
}

static void* sampler_internal(sampler_t sampler) {
    sampler_data_t* sampler_data = sampler_query_data(sampler);
    return gfx_respool_internal(gfx_ctx.sampler_pool, sampler_data->internal);
}

attachments_t attachments_alloc() {
    attachments_t attachments = {0};
    attachments_data_t* attachments_data = gfx_respool_push_data(gfx_ctx.attachments_pool, &attachments.id);
    mem_clear(attachments_data, sizeof(attachments_data_t));
    return attachments;
}

void attachments_init(attachments_t attachments, attachments_info_t info) {
    attachments_data_t* data = gfx_respool_data(gfx_ctx.attachments_pool, attachments.id);

    memcpy(data->colours, info.colours, sizeof(data->colours));
    data->depth_stencil = info.depth_stencil;

    for(u32 i = 0; i < GFX_MAX_COLOUR_ATTACHMENTS; i ++) {
        printf("%u, ", data->colours[i].id);
    }
    printf("\n");

    backend->attachments_init(data, info);
}

void attachments_discard(attachments_t attachments) {
    attachments_data_t* data = gfx_respool_data(gfx_ctx.attachments_pool, attachments.id);
    backend->attachments_destroy(data);
    gfx_respool_free_internal(gfx_ctx.attachments_pool, data->internal);
}

void attachments_destroy(attachments_t attachments) {
    attachments_discard(attachments);
    gfx_respool_free_data(gfx_ctx.attachments_pool, attachments.id);
}

attachments_t attachments_new(attachments_info_t info) {
    attachments_t attachments = attachments_alloc();
    attachments_init(attachments, info);
    return attachments;
}

attachments_data_t* attachments_query_data(attachments_t attachments) {
    return gfx_respool_data(gfx_ctx.attachments_pool, attachments.id);
}

static void* attachments_internal(attachments_t attachments) {
    attachments_data_t* data = attachments_query_data(attachments);
    return gfx_respool_internal(gfx_ctx.attachments_pool, data->internal);
}

static u32 uniform_type_get_bytes(uniform_type_t type) {
    // TODO(nix3l): padding??
    switch (type) {
        case UNIFORM_TYPE_i32: return 4;
        case UNIFORM_TYPE_u32: return 4;
        case UNIFORM_TYPE_f32: return 4;
        case UNIFORM_TYPE_v2f: return 8;
        case UNIFORM_TYPE_v2i: return 8;
        case UNIFORM_TYPE_v3f: return 12;
        case UNIFORM_TYPE_v3i: return 12;
        case UNIFORM_TYPE_v4f: return 16;
        case UNIFORM_TYPE_v4i: return 16;
        case UNIFORM_TYPE_mat4: return 64;
        default: UNREACHABLE; return 0;
    }
}

static char* shader_type_name(shader_pass_type_t type) {
    switch(type) {
        case SHADER_PASS_VERTEX: return "vertex";
        case SHADER_PASS_FRAGMENT: return "fragment";
        case SHADER_PASS_COMPUTE: return "compute";
        default: return "unknown";
    }
}

shader_t shader_alloc() {
    shader_t shader = {0};
    shader_data_t* shader_data = gfx_respool_push_data(gfx_ctx.shader_pool, &shader.id);
    mem_clear(shader_data, sizeof(shader_data_t));
    return shader;
}

void shader_init(shader_t shader, shader_info_t info) {
    shader_data_t* shader_data = gfx_respool_data(gfx_ctx.shader_pool, shader.id);

    memcpy(shader_data->name, info.name, sizeof(shader_data->name));
    memcpy(shader_data->pretty_name, info.pretty_name, sizeof(shader_data->pretty_name));
    memcpy(shader_data->attribs, info.attribs, sizeof(shader_data->attribs));

    shader_data->vertex_pass = (shader_pass_t) {
        .src = info.vertex_src,
        .type = SHADER_PASS_VERTEX
    };

    shader_data->fragment_pass = (shader_pass_t) {
        .src = info.fragment_src,
        .type = SHADER_PASS_FRAGMENT
    };

    backend->shader_init(shader_data, info);
}

void shader_discard(shader_t shader) {
    shader_data_t* shader_data = gfx_respool_data(gfx_ctx.shader_pool, shader.id);
    backend->shader_destroy(shader_data);
    gfx_respool_free_internal(gfx_ctx.shader_pool, shader_data->internal);
}

void shader_destroy(shader_t shader) {
    shader_discard(shader);
    gfx_respool_free_data(gfx_ctx.shader_pool, shader.id);
}

shader_t shader_new(shader_info_t info) {
    shader_t shader = shader_alloc();
    shader_init(shader, info);
    return shader;
}

shader_data_t* shader_query_data(shader_t shader) {
    return gfx_respool_data(gfx_ctx.shader_pool, shader.id);
}

static void* shader_internal(shader_t shader) {
    shader_data_t* shader_data = shader_query_data(shader);
    return gfx_respool_internal(gfx_ctx.shader_pool, shader_data->internal);
}

void shader_update_uniforms(shader_t shader, range_t data) {
    shader_data_t* shader_data = gfx_respool_data(gfx_ctx.shader_pool, shader.id);
    backend->shader_update_uniforms(shader_data, data);
}

void gfx_activate_pipeline(render_pipeline_t pip) {
    if(pip.depth.func == DEPTH_FUNC_UNDEFINED) pip.depth.func = DEPTH_FUNC_LESS;
    if(pip.cull.face == CULL_FACE_UNDEFINED) pip.cull.face = CULL_FACE_BACK;
    gfx_ctx.active_pipeline = pip;
    backend->activate_pipeline(gfx_ctx.active_pipeline);
}

void gfx_supply_bindings(render_bindings_t bindings) {
    gfx_ctx.active_bindings = bindings;
    backend->activate_bindings(gfx_ctx.active_bindings);
}

void gfx_draw() {
    backend->draw(gfx_ctx.active_bindings.mesh);
}

void gfx_viewport(u32 x, u32 y, u32 w, u32 h) {
    backend->viewport(x, y, w, h);
}

// OPENGL-SPECIFIC
// MESH
static u32 gl_mesh_primitive(mesh_primitive_t primitive) {
    switch(primitive) {
        case MESH_PRIMITIVE_TRIANGLES: return GL_TRIANGLES;
        case MESH_PRIMITIVE_LINES: return GL_LINES;
        case MESH_PRIMITIVE_POINTS: return GL_POINTS;
        default: UNREACHABLE; return 0;
    }
}

static u32 gl_vbo_create(u32 attribute, u32 dimensions, void* data, u32 bytes) {
    u32 vbo;
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, bytes, data, GL_STATIC_DRAW);
    glVertexAttribPointer(attribute, dimensions, GL_FLOAT, GL_FALSE, 0, NULL);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return vbo;
}

static u32 gl_indices_vbo_create(u32* indices, u32 bytes) {
    u32 vbo;
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, bytes, indices, GL_STATIC_DRAW);

    return vbo;
}

static void gl_mesh_init(mesh_data_t* mesh, mesh_info_t info) {
    gl_mesh_internal_t* glmesh = gfx_respool_push_internal(gfx_ctx.mesh_pool, &mesh->internal);
    mem_clear(glmesh, sizeof(gl_mesh_internal_t));

    glGenVertexArrays(1, &glmesh->vao);
    glBindVertexArray(glmesh->vao);

    for(u32 i = 0; i < mesh_format_num_attributes(info.format); i ++) {
        mesh_attribute_t attribute = info.attributes[i];
        glmesh->vbos[i] = gl_vbo_create(i, attribute.dimensions, attribute.data.ptr, attribute.data.size);
    }

    if(info.index_type != MESH_INDEX_NONE)
        glmesh->index_vbo = gl_indices_vbo_create(info.indices.ptr, info.indices.size);

    glBindVertexArray(0);
}

static void gl_mesh_destroy(mesh_data_t* mesh) {
    gl_mesh_internal_t* glmesh = gfx_respool_internal(gfx_ctx.mesh_pool, mesh->internal);

    // glDeleteBuffers simply ignores any 0's or invalid ids
    // so this is perfectly fine
    glDeleteBuffers(GFX_MAX_VERTEX_ATTRIBS, glmesh->vbos);
    glDeleteBuffers(1, &glmesh->index_vbo);
    glDeleteVertexArrays(1, &glmesh->vao);
}

static void gl_mesh_bind_attributes(mesh_data_t* mesh) {
    for(u32 i = 0; i < mesh_format_num_attributes(mesh->format); i ++) {
        glEnableVertexAttribArray(i);
    }
}

// TEXTURE
static u32 gl_texture_bind_target(texture_type_t type) {
    switch(type) {
        case TEXTURE_TYPE_2D: return GL_TEXTURE_2D;
        default: UNREACHABLE; return 0;
    }
}

static u32 gl_texture_format(texture_format_t format) {
    switch(format) {
        case TEXTURE_FORMAT_R8:
        case TEXTURE_FORMAT_R8I:
        case TEXTURE_FORMAT_R8UI:
        case TEXTURE_FORMAT_R16:
        case TEXTURE_FORMAT_R16F:
        case TEXTURE_FORMAT_R16I:
        case TEXTURE_FORMAT_R16UI:
        case TEXTURE_FORMAT_R32F:
        case TEXTURE_FORMAT_R32I:
        case TEXTURE_FORMAT_R32UI:
            return GL_RED;
        case TEXTURE_FORMAT_RG8:
        case TEXTURE_FORMAT_RG8I:
        case TEXTURE_FORMAT_RG8UI:
        case TEXTURE_FORMAT_RG16:
        case TEXTURE_FORMAT_RG16F:
        case TEXTURE_FORMAT_RG16I:
        case TEXTURE_FORMAT_RG16UI:
        case TEXTURE_FORMAT_RG32F:
        case TEXTURE_FORMAT_RG32I:
        case TEXTURE_FORMAT_RG32UI:
            return GL_RG;
        case TEXTURE_FORMAT_RGB8:
        case TEXTURE_FORMAT_RGB8I:
        case TEXTURE_FORMAT_RGB8UI:
        case TEXTURE_FORMAT_RGB16F:
        case TEXTURE_FORMAT_RGB16I:
        case TEXTURE_FORMAT_RGB16UI:
        case TEXTURE_FORMAT_RGB32F:
        case TEXTURE_FORMAT_RGB32I:
        case TEXTURE_FORMAT_RGB32UI:
            return GL_RGB;
        case TEXTURE_FORMAT_RGBA8:
        case TEXTURE_FORMAT_RGBA8I:
        case TEXTURE_FORMAT_RGBA8UI:
        case TEXTURE_FORMAT_RGBA16F:
        case TEXTURE_FORMAT_RGBA16I:
        case TEXTURE_FORMAT_RGBA16UI:
        case TEXTURE_FORMAT_RGBA32F:
        case TEXTURE_FORMAT_RGBA32I:
        case TEXTURE_FORMAT_RGBA32UI:
            return GL_RGBA;
        case TEXTURE_FORMAT_DEPTH:
            return GL_DEPTH_COMPONENT;
        case TEXTURE_FORMAT_DEPTH_STENCIL:
            return GL_DEPTH_STENCIL;
        default:
            UNREACHABLE;
            return 0;
    }
}

static u32 gl_texture_internalformat(texture_format_t format) {
    switch(format) {
        // one channel
        case TEXTURE_FORMAT_R8: return GL_R8;
        case TEXTURE_FORMAT_R8I: return GL_R8I;
        case TEXTURE_FORMAT_R8UI: return GL_R8UI;

        case TEXTURE_FORMAT_R16: return GL_R16;
        case TEXTURE_FORMAT_R16F: return GL_R16F;
        case TEXTURE_FORMAT_R16I: return GL_R16I;
        case TEXTURE_FORMAT_R16UI: return GL_R16UI;

        case TEXTURE_FORMAT_R32F: return GL_R32F;
        case TEXTURE_FORMAT_R32I: return GL_R32I;
        case TEXTURE_FORMAT_R32UI: return GL_R32UI;

        // 2 channels
        case TEXTURE_FORMAT_RG8: return GL_RG8;
        case TEXTURE_FORMAT_RG8I: return GL_RG8I;
        case TEXTURE_FORMAT_RG8UI: return GL_RG8UI;

        case TEXTURE_FORMAT_RG16: return GL_RG16;
        case TEXTURE_FORMAT_RG16F: return GL_RG16F;
        case TEXTURE_FORMAT_RG16I: return GL_RG16I;
        case TEXTURE_FORMAT_RG16UI: return GL_RG16UI;

        case TEXTURE_FORMAT_RG32F: return GL_RG32F;
        case TEXTURE_FORMAT_RG32I: return GL_RG32I;
        case TEXTURE_FORMAT_RG32UI: return GL_R32UI;

        // 3 channels
        case TEXTURE_FORMAT_RGB8: return GL_RGB8;
        case TEXTURE_FORMAT_RGB8I: return GL_RGB8I;
        case TEXTURE_FORMAT_RGB8UI: return GL_RGB8UI;

        case TEXTURE_FORMAT_RGB16F: return GL_RGB16F;
        case TEXTURE_FORMAT_RGB16I: return GL_RGB16I;
        case TEXTURE_FORMAT_RGB16UI: return GL_RGB16UI;

        case TEXTURE_FORMAT_RGB32F: return GL_RGB32F;
        case TEXTURE_FORMAT_RGB32I: return GL_RGB32I;
        case TEXTURE_FORMAT_RGB32UI: return GL_RGB32UI;

        // 4 channels
        case TEXTURE_FORMAT_RGBA8: return GL_RGBA8;
        case TEXTURE_FORMAT_RGBA8I: return GL_RGBA8I;
        case TEXTURE_FORMAT_RGBA8UI: return GL_RGBA8UI;

        case TEXTURE_FORMAT_RGBA16F: return GL_RGBA16F;
        case TEXTURE_FORMAT_RGBA16I: return GL_RGBA16I;
        case TEXTURE_FORMAT_RGBA16UI: return GL_RGBA16UI;

        case TEXTURE_FORMAT_RGBA32F: return GL_RGBA32F;
        case TEXTURE_FORMAT_RGBA32I: return GL_RGBA32I;
        case TEXTURE_FORMAT_RGBA32UI: return GL_RGBA32UI;

        // depth/stencil
        case TEXTURE_FORMAT_DEPTH: GL_DEPTH_COMPONENT32F;
        case TEXTURE_FORMAT_DEPTH_STENCIL: return GL_DEPTH32F_STENCIL8;

        default: UNREACHABLE; return 0;
    }
}

static u32 gl_texture_data_type(texture_format_t format) {
    switch(format) {
        case TEXTURE_FORMAT_R8I:
        case TEXTURE_FORMAT_RG8I:
        case TEXTURE_FORMAT_RGB8I:
        case TEXTURE_FORMAT_RGBA8I:
            return GL_BYTE;
        case TEXTURE_FORMAT_R8:
        case TEXTURE_FORMAT_R8UI:
        case TEXTURE_FORMAT_RG8:
        case TEXTURE_FORMAT_RG8UI:
        case TEXTURE_FORMAT_RGB8:
        case TEXTURE_FORMAT_RGB8UI:
        case TEXTURE_FORMAT_RGBA8:
        case TEXTURE_FORMAT_RGBA8UI:
            return GL_UNSIGNED_BYTE;
        case TEXTURE_FORMAT_R16I:
        case TEXTURE_FORMAT_RG16I:
        case TEXTURE_FORMAT_RGB16I:
        case TEXTURE_FORMAT_RGBA16I:
            return GL_SHORT;
        case TEXTURE_FORMAT_R16:
        case TEXTURE_FORMAT_R16UI:
        case TEXTURE_FORMAT_RG16:
        case TEXTURE_FORMAT_RG16UI:
        case TEXTURE_FORMAT_RGB16UI:
        case TEXTURE_FORMAT_RGBA16UI:
            return GL_UNSIGNED_SHORT;
        case TEXTURE_FORMAT_R32UI:
        case TEXTURE_FORMAT_RG32UI:
        case TEXTURE_FORMAT_RGB32UI:
        case TEXTURE_FORMAT_RGBA32UI:
            return GL_UNSIGNED_INT;
        case TEXTURE_FORMAT_R32I:
        case TEXTURE_FORMAT_RG32I:
        case TEXTURE_FORMAT_RGB32I:
        case TEXTURE_FORMAT_RGBA32I:
            return GL_INT;
        case TEXTURE_FORMAT_R16F:
        case TEXTURE_FORMAT_RG16F:
        case TEXTURE_FORMAT_RGB16F:
        case TEXTURE_FORMAT_RGBA16F:
            return GL_HALF_FLOAT;
        case TEXTURE_FORMAT_R32F:
        case TEXTURE_FORMAT_RG32F:
        case TEXTURE_FORMAT_RGB32F:
        case TEXTURE_FORMAT_RGBA32F:
            return GL_FLOAT;
        case TEXTURE_FORMAT_DEPTH:
            return GL_FLOAT;
        case TEXTURE_FORMAT_DEPTH_STENCIL:
            return GL_UNSIGNED_INT_24_8;
        default:
            UNREACHABLE;
            return 0;
    }
}

static void gl_texture_init(texture_data_t* texture, texture_info_t info) {
    gl_texture_internal_t* gltex = gfx_respool_push_internal(gfx_ctx.texture_pool, &texture->internal);
    mem_clear(gltex, sizeof(gl_texture_internal_t));

    u32 target = gl_texture_bind_target(info.type);

    glGenTextures(1, &gltex->id);
    glBindTexture(target, gltex->id);

    glTexImage2D(
        target,
        0,
        gl_texture_internalformat(info.format),
        info.width,
        info.height,
        0, // TODO(nix3l): border?
        gl_texture_format(info.format),
        gl_texture_data_type(info.format),
        info.data.ptr
    );

    glBindTexture(target, 0);
}

static void gl_texture_destroy(texture_data_t* texture) {
    gl_texture_internal_t* gltex = gfx_respool_internal(gfx_ctx.texture_pool, texture->internal);
    glDeleteTextures(1, &gltex->id);
}

// SAMPLER
static u32 gl_texture_filter(texture_filter_t filter) {
    switch(filter) {
        case TEXTURE_FILTER_NEAREST: return GL_NEAREST;
        case TEXTURE_FILTER_LINEAR: return GL_LINEAR;
        default: UNREACHABLE; return 0;
    }
}

static u32 gl_texture_wrap(texture_wrap_t wrap) {
    switch(wrap) {
        case TEXTURE_WRAP_REPEAT: return GL_REPEAT;
        case TEXTURE_WRAP_MIRRORED_REPEAT: return GL_MIRRORED_REPEAT;
        case TEXTURE_WRAP_CLAMP_TO_EDGE: return GL_CLAMP_TO_EDGE;
        default: UNREACHABLE; return 0;
    }
}

static void gl_sampler_init(sampler_data_t* sampler, sampler_info_t info) {
    gl_sampler_internal_t* glsampler = gfx_respool_push_internal(gfx_ctx.sampler_pool, &sampler->internal);
    mem_clear(glsampler, sizeof(gl_sampler_internal_t));

    glGenSamplers(1, &glsampler->id);
    glSamplerParameteri(glsampler->id, GL_TEXTURE_MIN_FILTER, gl_texture_filter(info.min_filter));
    glSamplerParameteri(glsampler->id, GL_TEXTURE_MAG_FILTER, gl_texture_filter(info.mag_filter));
    glSamplerParameteri(glsampler->id, GL_TEXTURE_WRAP_R, gl_texture_wrap(info.u_wrap));
    glSamplerParameteri(glsampler->id, GL_TEXTURE_WRAP_S, gl_texture_wrap(info.v_wrap));
    // TODO(nix3l): update for GL_TEXTURE_WRAP_T
}

static void gl_sampler_destroy(sampler_data_t* sampler) {
    gl_sampler_internal_t* glsampler = gfx_respool_internal(gfx_ctx.sampler_pool, sampler->internal);
    if(!glsampler) return;
    glDeleteSamplers(1, &glsampler->id);
}

// ATTACHMENTS
static void gl_attachments_init(attachments_data_t* att, attachments_info_t info) {
    gl_attachments_internal_t* glatt = gfx_respool_push_internal(gfx_ctx.attachments_pool, &att->internal);
    mem_clear(glatt, sizeof(gl_attachments_internal_t));

    glGenFramebuffers(1, &glatt->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, glatt->fbo);

    for(u32 i = 0; i < GFX_MAX_COLOUR_ATTACHMENTS; i ++) {
        texture_t tex = info.colours[i];
        if(tex.id == GFX_INVALID_ID) {
            att->num_colours = i;
            break;
        }

        texture_data_t* tex_data = texture_query_data(tex);
        gl_texture_internal_t* gltex = texture_internal(tex);

        u32 target = gl_texture_bind_target(tex_data->type);
        glBindTexture(target, gltex->id);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, target, gltex->id, 0);
        glBindTexture(target, 0);
    }

    if(info.depth_stencil.id != GFX_INVALID_ID) {
        texture_t tex = info.depth_stencil;
        texture_data_t* tex_data = texture_query_data(tex);
        gl_texture_internal_t* gltex = texture_internal(tex);
        u32 target = gl_texture_bind_target(tex_data->type);

        glBindTexture(target, gltex->id);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, target, gltex->id, 0);
        glBindTexture(target, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void gl_attachments_destroy(attachments_data_t* att) {
    gl_attachments_internal_t* glatt = gfx_respool_internal(gfx_ctx.attachments_pool, att->internal);
    if(!glatt) return;
    glDeleteFramebuffers(1, &glatt->fbo);
}

// SHADER
static u32 gl_shader_type(shader_pass_type_t type) {
    switch(type) {
        case SHADER_PASS_VERTEX: return GL_VERTEX_SHADER;
        case SHADER_PASS_FRAGMENT: return GL_FRAGMENT_SHADER;
        case SHADER_PASS_COMPUTE: return GL_COMPUTE_SHADER;
        default: UNREACHABLE; return 0;
    }
}

static u32 gl_compile_shader(range_t src, shader_pass_type_t type) {
    u32 id = glCreateShader(gl_shader_type(type));

    glShaderSource(id, 1, (const char* const*)&src.ptr, NULL);
    glCompileShader(id);

    i32 success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if(!success) {
        char log[512];
        glGetShaderInfoLog(id, sizeof(log), NULL, log);
        LOG_ERR("error compiling %s shader:\n", shader_type_name(type));
        LOG_ERR("%s\n", log);
    }

    return id;
}

static void gl_shader_init(shader_data_t* shader, shader_info_t info) {
    gl_shader_internal_t* glshader = gfx_respool_push_internal(gfx_ctx.shader_pool, &shader->internal);
    mem_clear(glshader, sizeof(gl_shader_internal_t));

    glshader->program = glCreateProgram();

    u32 vs_id = gl_compile_shader(info.vertex_src, SHADER_PASS_VERTEX);
    u32 fs_id = gl_compile_shader(info.fragment_src, SHADER_PASS_FRAGMENT);

    glAttachShader(glshader->program, vs_id);
    glAttachShader(glshader->program, fs_id);

    for(u32 i = 0; i < GFX_MAX_VERTEX_ATTRIBS; i ++) {
        shader_vertex_attribute_t attrib = info.attribs[i];
        if(!attrib.name) break;
        glBindAttribLocation(glshader->program, i, attrib.name);
    }

    glLinkProgram(glshader->program);
    glValidateProgram(glshader->program);

    i32 success;
    glGetProgramiv(glshader->program, GL_LINK_STATUS, &success);

    if(!success) {
        char log[512];
        glGetProgramInfoLog(glshader->program, 512, NULL, log);
        LOG_ERR("failed to link shader [%s]:\n%s\n", info.name, log);
    }

    for(u32 i = 0; i < GFX_MAX_UNIFORMS; i ++) {
        uniform_t uniform_info = info.uniforms[i];
        if(!uniform_info.name || uniform_info.type == UNIFORM_TYPE_INVALID) {
            shader->uniform_block.num = i;
            break;
        }

        uniform_t* shader_uniform = &shader->uniform_block.uniforms[i];
        shader_uniform->glid = glGetUniformLocation(glshader->program, uniform_info.name);
        if(shader_uniform->glid == (u32)-1) LOG_ERR("couldnt find uniform [%s] in shader [%s]\n", uniform_info.name, info.pretty_name);
        shader_uniform->name = uniform_info.name;
        shader_uniform->type = uniform_info.type;
    }

    glDetachShader(glshader->program, vs_id);
    glDetachShader(glshader->program, fs_id);
    glDeleteShader(vs_id);
    glDeleteShader(fs_id);
}

static void gl_shader_destroy(shader_data_t* shader) {
    gl_shader_internal_t* glshader = gfx_respool_internal(gfx_ctx.shader_pool, shader->internal);
    if(!glshader) return;
    glDeleteProgram(glshader->program);
}

static void gl_shader_update_uniforms(shader_data_t* shader, range_t uniforms) {
    gl_shader_internal_t* glshader = gfx_respool_internal(gfx_ctx.shader_pool, shader->internal);
    if(!glshader) return;
    u32 read_size = 0;
    for(u32 i = 0; i < shader->uniform_block.num; i ++) {
        uniform_t uniform = shader->uniform_block.uniforms[i];
        u32 size = uniform_type_get_bytes(uniform.type);
        if(size == 0) continue;

        void* curr = uniforms.ptr + read_size;
        switch(uniform.type) {
            case UNIFORM_TYPE_i32:
                glUniform1iv(uniform.glid, 1, (i32*)curr);
                break;
            case UNIFORM_TYPE_u32:
                glUniform1uiv(uniform.glid, 1, (u32*)curr);
                break;
            case UNIFORM_TYPE_f32:
                glUniform1fv(uniform.glid, 1, (f32*)curr);
                break;
            case UNIFORM_TYPE_v2i:
                glUniform2iv(uniform.glid, 1, (i32*)curr);
                break;
            case UNIFORM_TYPE_v2f:
                glUniform2fv(uniform.glid, 1, (f32*)curr);
                break;
            case UNIFORM_TYPE_v3i:
                glUniform3iv(uniform.glid, 1, (i32*)curr);
                break;
            case UNIFORM_TYPE_v3f:
                glUniform3fv(uniform.glid, 1, (f32*)curr);
                break;
            case UNIFORM_TYPE_v4i:
                glUniform4iv(uniform.glid, 1, (i32*)curr);
                break;
            case UNIFORM_TYPE_v4f:
                glUniform4fv(uniform.glid, 1, (f32*)curr);
                break;
            case UNIFORM_TYPE_mat4:
                glUniformMatrix4fv(uniform.glid, 1, false, (f32*)curr);
                break;
            default: UNREACHABLE; break;
        }

        read_size += size;
        if(read_size > uniforms.size) {
            LOG_ERR("tried to write more data than was provided. ignoring.\n");
            return;
        }
    }
}

static u32 gl_depth_func(depth_func_t func) {
    switch(func) {
        case DEPTH_FUNC_NEVER: return GL_NEVER;
        case DEPTH_FUNC_ALWAYS: return GL_ALWAYS;
        case DEPTH_FUNC_LESS: return GL_LESS;
        case DEPTH_FUNC_GREATER: return GL_GREATER;
        case DEPTH_FUNC_LESS_EQUAL: return GL_LEQUAL;
        case DEPTH_FUNC_GREATER_EQUAL: return GL_GEQUAL;
        case DEPTH_FUNC_EQUAL: return GL_EQUAL;
        case DEPTH_FUNC_NOT_EQUAL: return GL_NOTEQUAL;
        default: UNREACHABLE; return 0;
    }
}

static u32 gl_cull_face(cull_face_t face) {
    switch(face) {
        case CULL_FACE_FRONT: return GL_FRONT;
        case CULL_FACE_BACK: return GL_BACK;
        case CULL_FACE_FRONT_AND_BACK: return GL_FRONT_AND_BACK;
        default: UNREACHABLE; return 0;
    }
}

static u32 gl_blend_func(blend_func_t func) {
    switch(func) {
        case BLEND_FUNC_ZERO: return GL_ZERO;
        case BLEND_FUNC_ONE: return GL_ONE;
        case BLEND_FUNC_SRC_COLOUR: return GL_SRC_COLOR;
        case BLEND_FUNC_SRC_ALPHA: return GL_SRC_ALPHA;
        case BLEND_FUNC_SRC_ONE_MINUS_COLOUR: return GL_ONE_MINUS_SRC_COLOR;
        case BLEND_FUNC_SRC_ONE_MINUS_ALPHA: return GL_ONE_MINUS_SRC_ALPHA;
        case BLEND_FUNC_DST_COLOUR: return GL_DST_COLOR;
        case BLEND_FUNC_DST_ALPHA: return GL_DST_ALPHA;
        case BLEND_FUNC_DST_ONE_MINUS_COLOUR: return GL_ONE_MINUS_DST_COLOR;
        case BLEND_FUNC_DST_ONE_MINUS_ALPHA: return GL_ONE_MINUS_DST_ALPHA;
        default: UNREACHABLE; return 0;
    }
}

static void gl_activate_pipeline(render_pipeline_t pip) {
    if(pip.depth.enable) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(gl_depth_func(pip.depth.func));
    }

    if(pip.cull.enable) {
        glEnable(GL_CULL_FACE);
        glCullFace(gl_cull_face(pip.cull.face));
    }

    if(pip.blend.enable) {
        glEnable(GL_BLEND);
        glBlendFunc(gl_blend_func(pip.blend.src_func), gl_blend_func(pip.blend.dst_func));
    }

    if(pip.draw_attachments.id != GFX_INVALID_ID) {
        gl_attachments_internal_t* glatt = attachments_internal(pip.draw_attachments);
        glBindFramebuffer(GL_FRAMEBUFFER, glatt->fbo);

        for(u32 i = 0; i < GFX_MAX_COLOUR_ATTACHMENTS; i ++) {
            render_target_t target = pip.colour_targets[i];
            if(!target.enable) continue;
            glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);

            if(pip.clear.colour && !target.disable_clear)
                glClearBufferfv(GL_COLOR, i, target.override_clear_col ? target.clear_col.raw : pip.clear.clear_col.raw);
        }
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDrawBuffer(GL_BACK);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(v4f_expand(pip.clear.clear_col));
    }

    GLbitfield depth_stencil_clear = 0;
    if(pip.clear.depth) depth_stencil_clear |= GL_DEPTH_BUFFER_BIT;
    if(pip.clear.stencil) depth_stencil_clear |= GL_STENCIL_BUFFER_BIT;
    glClear(depth_stencil_clear);

    gl_shader_internal_t* glshader = shader_internal(pip.shader);
    glUseProgram(glshader->program);
}

static void gl_activate_bindings(render_bindings_t bindings) {
    mesh_data_t* mesh = mesh_query_data(bindings.mesh);
    gl_mesh_internal_t* glmesh = mesh_internal(bindings.mesh);
    glBindVertexArray(glmesh->vao);
    gl_mesh_bind_attributes(mesh);

    if(bindings.read_attachments.id != GFX_INVALID_ID) {
        gl_attachments_internal_t* glatt = attachments_internal(bindings.read_attachments);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, glatt->fbo);
        attachments_data_t* att_data = attachments_query_data(bindings.read_attachments);
        for(u32 i = 0; i < GFX_MAX_COLOUR_ATTACHMENTS; i ++) {
            if(att_data->colours[i].id != GFX_INVALID_ID)
                glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
        }
    } else {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    }

    for(u32 i = 0; i < GFX_MAX_SAMPLER_SLOTS; i ++) {
        texture_t texture = bindings.texture_samplers[i].texture;
        sampler_t sampler = bindings.texture_samplers[i].sampler;
        if(texture.id == GFX_INVALID_ID) continue;

        texture_data_t* texture_data = texture_query_data(texture);
        gl_texture_internal_t* gltex = texture_internal(texture);

        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(gl_texture_bind_target(texture_data->type), gltex->id);

        if(sampler.id != GFX_INVALID_ID) {
            gl_sampler_internal_t* glsampler = sampler_internal(sampler);
            glBindSampler(i, glsampler->id);
        }
    }
}

static void gl_draw(mesh_t mesh) {
    mesh_data_t* mesh_data = mesh_query_data(mesh);

    if(mesh_data->index_type != MESH_INDEX_NONE) {
        glDrawElements(gl_mesh_primitive(mesh_data->primitive), mesh_data->count, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(gl_mesh_primitive(mesh_data->primitive), 0, mesh_data->count);
    }
}

static void gl_viewport(u32 x, u32 y, u32 w, u32 h) {
    glViewport(x, y, w, h);
}
