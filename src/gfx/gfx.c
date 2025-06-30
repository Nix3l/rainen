#include "gfx.h"
#include "base.h"
#include "base_macros.h"
#include "memory/memory.h"
#include "util/util.h"

gfx_ctx_t gfx_ctx;

typedef void (*mesh_init_func) (mesh_t*, mesh_info_t);
typedef void (*mesh_destroy_func) (mesh_t*);
typedef void (*texture_init_func) (texture_t*, texture_info_t);
typedef void (*texture_destroy_func) (texture_t*);
typedef void (*sampler_init_func) (sampler_t*, sampler_info_t);
typedef void (*sampler_destroy_func) (sampler_t*);
typedef void (*shader_init_func) (shader_t*, shader_info_t);
typedef void (*shader_destroy_func) (shader_t*);
typedef void (*shader_update_uniforms_func) (shader_t*, range_t);

typedef struct backend_jumptable_t {
    mesh_init_func              mesh_init;
    mesh_destroy_func           mesh_destroy;
    texture_init_func           texture_init;
    texture_destroy_func        texture_destroy;
    sampler_init_func           sampler_init;
    sampler_destroy_func        sampler_destroy;
    shader_init_func            shader_init;
    shader_destroy_func         shader_destroy;
    shader_update_uniforms_func shader_update_uniforms;
} backend_jumptable_t;

static backend_jumptable_t backend_jumptables[GFX_BACKEND_NUM] = {0};
static backend_jumptable_t* jumptable = NULL;

static void gl_mesh_init(mesh_t* mesh, mesh_info_t info);
static void gl_mesh_destroy(mesh_t* mesh);
static void gl_texture_init(texture_t* tex, texture_info_t info);
static void gl_texture_destroy(texture_t* tex);
static void gl_texture_init(texture_t* tex, texture_info_t info);
static void gl_texture_destroy(texture_t* tex);
static void gl_sampler_init(sampler_t* sampler, sampler_info_t info);
static void gl_sampler_destroy(sampler_t* sampler);
static void gl_shader_init(shader_t* shader, shader_info_t info);
static void gl_shader_destroy(shader_t* shader);
static void gl_shader_update_uniforms(shader_t* shader, range_t data);

static void init_jumptables() {
    backend_jumptables[GFX_BACKEND_GL] = (backend_jumptable_t) {
        .mesh_init              = gl_mesh_init,
        .mesh_destroy           = gl_mesh_destroy,
        .texture_init           = gl_texture_init,
        .texture_destroy        = gl_texture_destroy,
        .sampler_init           = gl_sampler_init,
        .sampler_destroy        = gl_sampler_destroy,
        .shader_init            = gl_shader_init,
        .shader_destroy         = gl_shader_destroy,
        .shader_update_uniforms = gl_shader_update_uniforms,
    };

    jumptable = &backend_jumptables[gfx_ctx.backend];
}

static mempool_t mesh_pool;
static mempool_t texture_pool;
static mempool_t sampler_pool;
static mempool_t shader_pool;

void gfx_init(gfx_backend_t backend) {
    gfx_backend_info_t opengl_core_info = (gfx_backend_info_t) {
        .backend = GFX_BACKEND_GL,
        .version_major = 4,
        .version_minor = 3,
        .version_str = "4.3",
        .name = "glcore",
        .name_pretty = "OpenGL",
        .supported = GFX_SUPPORT_GL,
        .mesh_internal_size = sizeof(gl_mesh_info_t),
        .texture_internal_size = sizeof(gl_texture_info_t),
        .sampler_internal_size = sizeof(gl_sampler_info_t),
        .shader_internal_size = sizeof(gl_shader_info_t),
    };

    gfx_ctx = (gfx_ctx_t) {
        .backend = backend,
        .backend_info = {
            (gfx_backend_info_t) {0},
            opengl_core_info,
        },
    };

    if(!gfx_ctx.backend_info[backend].supported)
        PANIC("chosen backend is not supported on current OS\n");

    // for now, keep immutable
    mesh_pool = mempool_alloc_new(GFX_MAX_MESHES, gfx_ctx.backend_info[backend].mesh_internal_size, EXPAND_TYPE_IMMUTABLE);
    gfx_ctx.mesh_pool = &mesh_pool;

    texture_pool = mempool_alloc_new(GFX_MAX_TEXTURES, gfx_ctx.backend_info[backend].texture_internal_size, EXPAND_TYPE_IMMUTABLE);
    gfx_ctx.texture_pool = &texture_pool;

    shader_pool = mempool_alloc_new(GFX_MAX_SHADERS, gfx_ctx.backend_info[backend].shader_internal_size, EXPAND_TYPE_IMMUTABLE);
    gfx_ctx.shader_pool = &shader_pool;

    sampler_pool = mempool_alloc_new(GFX_MAX_SAMPLERS, gfx_ctx.backend_info[backend].sampler_internal_size, EXPAND_TYPE_IMMUTABLE);
    gfx_ctx.sampler_pool = &sampler_pool;

    init_jumptables();
}

void gfx_terminate() {
    mempool_destroy(gfx_ctx.mesh_pool);
    mempool_destroy(gfx_ctx.texture_pool);
    mempool_destroy(gfx_ctx.shader_pool);
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
        case MESH_FORMAT_X3T2N3: return 3;
        default: UNREACHABLE; return 0;
    }
}

mesh_attribute_t mesh_attribute(void *data, u32 size, u32 dimensions) {
    return (mesh_attribute_t) {
        .dimensions = dimensions,
        .data = range_new(data, size),
    };
}

mesh_t mesh_new(mesh_info_t info) {
    mesh_t mesh = {0};
    mesh_init_func mesh_init_gfx = jumptable->mesh_init;
    if(!mesh_init_gfx) PANIC("unsupported backend [%s] for %s\n", gfx_backend_info().name_pretty, __FUNCTION__);

    if(info.format == MESH_FORMAT_INVALID) {
        LOG_ERR("mesh format *must* be supplied\n");
        return mesh;
    }

    if(info.index_type == MESH_INDEX_TYPE_UNDEFINED)
        info.index_type = info.indices.size == 0 ? MESH_INDEX_TYPE_NONE : MESH_INDEX_TYPE_32b;

    if(info.primitive == MESH_PRIMITIVE_DEFAULT)
        info.primitive = MESH_PRIMITIVE_TRIANGLES;

    if(info.winding == MESH_WINDING_DEFAULT)
        info.winding = MESH_WINDING_CCW;

    mesh.format = info.format;
    mesh.index_type = info.index_type;
    mesh.primitive = info.primitive;
    mesh.winding = info.winding;

    mesh_init_gfx(&mesh, info);
    return mesh;
}

void mesh_destroy(mesh_t* mesh) {
    mesh_destroy_func mesh_destroy_gfx = jumptable->mesh_destroy;
    if(!mesh_destroy_gfx) PANIC("unsupported backend [%s] for %s\n", gfx_backend_info().name_pretty, __FUNCTION__);
    mesh_destroy_gfx(mesh);
    mempool_free(gfx_ctx.mesh_pool, mesh->gfx_handle);
}

texture_t texture_new(texture_info_t info) {
    texture_t texture = {0};
    texture_init_func texture_init_gfx = jumptable->texture_init;
    if(!texture_init_gfx) PANIC("unsupported backend[%s] for %s\n", gfx_backend_info().name_pretty, "__FUNCTION__");

    if(info.type == TEXTURE_TYPE_UNDEFINED) info.type = TEXTURE_TYPE_2D;
    if(info.mipmaps == 0) info.mipmaps = 1;

    texture.type = info.type;
    texture.format = info.format;
    texture.width = info.width;
    texture.height = info.height;
    texture.mipmaps = info.mipmaps;

    texture_init_gfx(&texture, info);
    return texture;
}

void texture_destroy(texture_t* texture) {
    texture_destroy_func texture_destroy_gfx = jumptable->texture_destroy;
    if(!texture_destroy_gfx) PANIC("unsupported backend[%s] for %s\n", gfx_backend_info().name_pretty, "__FUNCTION__");
    texture_destroy_gfx(texture);
    mempool_free(gfx_ctx.texture_pool, texture->gfx_handle);
}

sampler_t sampler_new(sampler_info_t info) {
    sampler_t sampler = {0};
    sampler_init_func sampler_init_gfx = jumptable->sampler_init;
    if(!sampler_init_gfx) PANIC("unsupported backend[%s] for %s\n", gfx_backend_info().name_pretty, "__FUNCTION__");

    if(info.wrap != TEXTURE_WRAP_UNDEFINED) {
        info.u_wrap = info.wrap;
        info.v_wrap = info.wrap;
    }

    if(info.filter != TEXTURE_FILTER_UNDEFINED) {
        info.min_filter = info.filter;
        info.mag_filter = info.filter;
    }

    sampler.u_wrap = info.u_wrap;
    sampler.v_wrap = info.v_wrap;
    sampler.min_filter = info.min_filter;
    sampler.mag_filter = info.mag_filter;

    sampler_init_gfx(&sampler, info);
    return sampler;
}

void sampler_destroy(sampler_t* sampler) {
    sampler_destroy_func sampler_destroy_gfx = jumptable->sampler_destroy;
    if(!sampler_destroy_gfx) PANIC("unsupported backend [%s] for %s\n", gfx_backend_info().name_pretty, __FUNCTION__);
    sampler_destroy_gfx(sampler);
    mempool_free(gfx_ctx.sampler_pool, sampler->gfx_handle);
}

static u32 uniform_type_get_bytes(uniform_type_t type) {
    // TODO(nix3l): padding??
    switch (type) {
        case UNIFORM_TYPE_i32: return sizeof(i32);
        case UNIFORM_TYPE_u32: return sizeof(u32);
        case UNIFORM_TYPE_f32: return sizeof(f32);
        case UNIFORM_TYPE_v2f: return 2 * sizeof(f32);
        case UNIFORM_TYPE_v2i: return 2 * sizeof(i32);
        case UNIFORM_TYPE_v3f: return 3 * sizeof(f32);
        case UNIFORM_TYPE_v3i: return 3 * sizeof(i32);
        case UNIFORM_TYPE_v4f: return 4 * sizeof(f32);
        case UNIFORM_TYPE_v4i: return 4 * sizeof(i32);
        case UNIFORM_TYPE_mat4: return 16 * sizeof(f32);
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

shader_t shader_new(shader_info_t info) {
    shader_t shader = {0};
    shader_init_func shader_init = jumptable->shader_init;
    if(!shader_init) PANIC("unsupported backend [%s] for %s\n", gfx_backend_info().name_pretty, __FUNCTION__);

    memcpy(shader.name, info.name, sizeof(shader.name));
    memcpy(shader.pretty_name, info.pretty_name, sizeof(shader.pretty_name));
    memcpy(shader.attribs, info.attribs, sizeof(shader.attribs));

    shader.vertex_pass = (shader_pass_t) {
        .src = info.vertex_src,
        .type = SHADER_PASS_VERTEX
    };

    shader.fragment_pass = (shader_pass_t) {
        .src = info.fragment_src,
        .type = SHADER_PASS_FRAGMENT
    };

    shader_init(&shader, info);
    return shader;
}

void shader_destroy(shader_t* shader) {
    shader_destroy_func shader_destroy_gfx = jumptable->shader_destroy;
    if(!shader_destroy_gfx) PANIC("unsupported backend [%s] for %s\n", gfx_backend_info().name_pretty, __FUNCTION__);
    shader_destroy_gfx(shader);
    mempool_free(gfx_ctx.shader_pool, shader->gfx_handle);
}

void shader_update_uniforms(shader_t* shader, range_t data) {
    shader_update_uniforms_func shader_update_uniforms_gfx = jumptable->shader_update_uniforms;
    if(!shader_update_uniforms_gfx) PANIC("unsupported backend [%s] for %s\n", gfx_backend_info().name_pretty, __FUNCTION__);
    shader_update_uniforms_gfx(shader, data);
}

// OPENGL-SPECIFIC
// MESH
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

static void gl_mesh_init(mesh_t* mesh, mesh_info_t info) {
    gl_mesh_info_t* glmesh = mempool_push(gfx_ctx.mesh_pool, &mesh->gfx_handle);
    memset(glmesh, 0, sizeof(gl_mesh_info_t));

    glGenVertexArrays(1, &glmesh->vao);
    glBindVertexArray(glmesh->vao);

    for(u32 i = 0; i < mesh_format_num_attributes(info.format); i ++) {
        mesh_attribute_t attribute = info.attributes[i];
        glmesh->vbos[i] = gl_vbo_create(i, attribute.dimensions, attribute.data.ptr, attribute.data.size);
    }

    if(info.index_type != MESH_INDEX_TYPE_NONE)
        glmesh->index_vbo = gl_indices_vbo_create(info.indices.ptr, info.indices.size);

    glBindVertexArray(0);
}

static void gl_mesh_destroy(mesh_t* mesh) {
    gl_mesh_info_t* glmesh = mempool_get(gfx_ctx.mesh_pool, mesh->gfx_handle);
    if(!glmesh) return;

    // glDeleteBuffers simply ignores any 0's or invalid ids
    // so this is perfectly fine
    glDeleteBuffers(GFX_MAX_VERTEX_ATTRIBS, glmesh->vbos);
    glDeleteBuffers(1, &glmesh->index_vbo);
    glDeleteVertexArrays(1, &glmesh->vao);

    mempool_free(gfx_ctx.mesh_pool, mesh->gfx_handle);
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

static void gl_texture_init(texture_t* texture, texture_info_t info) {
    gl_texture_info_t* gltex = mempool_push(gfx_ctx.texture_pool, &texture->gfx_handle);
    memset(gltex, 0, sizeof(gl_texture_info_t));

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

static void gl_texture_destroy(texture_t* texture) {
    gl_texture_info_t* gltex = mempool_get(gfx_ctx.texture_pool, texture->gfx_handle);
    if(!gltex) return;
    glDeleteTextures(1, &gltex->id);
    mempool_free(gfx_ctx.texture_pool, texture->gfx_handle);
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

static void gl_sampler_init(sampler_t* sampler, sampler_info_t info) {
    gl_sampler_info_t* glsampler = mempool_push(gfx_ctx.sampler_pool, &sampler->gfx_handle);
    memset(glsampler, 0, sizeof(gl_sampler_info_t));

    glGenSamplers(1, &glsampler->id);
    glSamplerParameteri(glsampler->id, GL_TEXTURE_MIN_FILTER, gl_texture_filter(info.min_filter));
    glSamplerParameteri(glsampler->id, GL_TEXTURE_MAG_FILTER, gl_texture_filter(info.mag_filter));
    glSamplerParameteri(glsampler->id, GL_TEXTURE_WRAP_R, gl_texture_wrap(info.u_wrap));
    glSamplerParameteri(glsampler->id, GL_TEXTURE_WRAP_S, gl_texture_wrap(info.v_wrap));
    // TODO(nix3l): update for GL_TEXTURE_WRAP_T
}

static void gl_sampler_destroy(sampler_t* sampler) {
    gl_sampler_info_t* glsampler = mempool_get(gfx_ctx.sampler_pool, sampler->gfx_handle);
    if(!glsampler) return;
    glDeleteSamplers(1, &glsampler->id);
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

static void gl_shader_init(shader_t* shader, shader_info_t info) {
    gl_shader_info_t* glshader = mempool_push(gfx_ctx.shader_pool, &shader->gfx_handle);
    memset(glshader, 0, sizeof(gl_shader_info_t));

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
        LOG_ERR("failed to link shader [%s]:\n%s\n", info.pretty_name, log);
    }

    for(u32 i = 0; i < GFX_MAX_UNIFORMS; i ++) {
        uniform_t uniform_info = info.uniforms[i];
        if(!uniform_info.name || uniform_info.type == UNIFORM_TYPE_INVALID) {
            shader->uniform_block.num = i + 1;
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

static void gl_shader_destroy(shader_t* shader) {
    gl_shader_info_t* glshader = mempool_get(gfx_ctx.shader_pool, shader->gfx_handle);
    if(!glshader) return;
    glDeleteProgram(glshader->program);
    mempool_free(gfx_ctx.shader_pool, shader->gfx_handle);
}

static void gl_shader_update_uniforms(shader_t* shader, range_t uniforms) {
    gl_shader_info_t* glshader = mempool_get(gfx_ctx.shader_pool, shader->gfx_handle);
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
