#include "gfx.h"
#include "util/util.h"

gfx_ctx_t gfx_ctx;

typedef void (*mesh_init_func) (mesh_t*, mesh_info_t);
typedef void (*mesh_destroy_func) (mesh_t*);
typedef void (*shader_init_func) (shader_t*, shader_info_t);
typedef void (*shader_destroy_func) (shader_t*);
typedef void (*shader_update_uniforms_func) (shader_t*, range_t);

typedef struct backend_jumptable_t {
    mesh_init_func              mesh_init;
    mesh_destroy_func           mesh_destroy;
    shader_init_func            shader_init;
    shader_destroy_func         shader_destroy;
    shader_update_uniforms_func shader_update_uniforms;
} backend_jumptable_t;

static backend_jumptable_t backend_jumptables[GFX_BACKEND_NUM] = {0};
static backend_jumptable_t* jumptable = NULL;

static void gl_mesh_init(mesh_t* mesh, mesh_info_t info);
static void gl_mesh_destroy(mesh_t* mesh);
static void gl_shader_init(shader_t* shader, shader_info_t info);
static void gl_shader_destroy(shader_t* shader);
static void gl_shader_update_uniforms(shader_t* shader, range_t data);

static void init_jumptables() {
    backend_jumptables[GFX_BACKEND_GL] = (backend_jumptable_t) {
        .mesh_init              = gl_mesh_init,
        .mesh_destroy           = gl_mesh_destroy,
        .shader_init            = gl_shader_init,
        .shader_destroy         = gl_shader_destroy,
        .shader_update_uniforms = gl_shader_update_uniforms,
    };

    jumptable = &backend_jumptables[gfx_ctx.backend];
}

static mempool_t mesh_pool;
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
        .mesh_id_size = sizeof(gl_mesh_info_t),
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
    mesh_pool = mempool_alloc_new(GFX_MAX_MESHES, gfx_ctx.backend_info[backend].mesh_id_size, EXPAND_TYPE_IMMUTABLE);
    gfx_ctx.mesh_pool = &mesh_pool;

    shader_pool = mempool_alloc_new(GFX_MAX_SHADERS, gfx_ctx.backend_info[backend].mesh_id_size, EXPAND_TYPE_IMMUTABLE);
    gfx_ctx.shader_pool = &shader_pool;

    init_jumptables();
}

void gfx_terminate() {
    mempool_destroy(gfx_ctx.mesh_pool);
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
        case MESH_FORMAT_INVALID: return 0;
        case MESH_FORMAT_X3T2N3: return 3;

        default:
            PANIC("unreachable branch\n");
            return -1;
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

    mesh_init_gfx(&mesh, info);

    mesh.format = info.format;
    mesh.index_type = info.index_type;
    mesh.primitive = info.primitive == MESH_PRIMITIVE_DEFAULT ? MESH_PRIMITIVE_TRIANGLES : info.primitive;
    mesh.winding = info.winding == MESH_WINDING_DEFAULT ? MESH_WINDING_CCW : info.winding;

    return mesh;
}

void mesh_destroy(mesh_t* mesh) {
    mesh_destroy_func mesh_destroy_gfx = jumptable->mesh_destroy;
    if(!mesh_destroy_gfx) PANIC("unsupported backend [%s] for %s\n", gfx_backend_info().name_pretty, __FUNCTION__);
    mesh_destroy_gfx(mesh);
}

static u32 uniform_type_get_bytes(uniform_type_t type) {
    return
        type == UNIFORM_TYPE_i32 ? sizeof(i32) :
        type == UNIFORM_TYPE_u32 ? sizeof(u32) :
        type == UNIFORM_TYPE_f32 ? sizeof(f32) :
        type == UNIFORM_TYPE_v2f ? 2 * sizeof(f32) :
        type == UNIFORM_TYPE_v2i ? 2 * sizeof(i32) :
        type == UNIFORM_TYPE_v3f ? 3 * sizeof(f32) :
        type == UNIFORM_TYPE_v3i ? 3 * sizeof(i32) :
        type == UNIFORM_TYPE_v4f ? 4 * sizeof(f32) :
        type == UNIFORM_TYPE_v4i ? 4 * sizeof(i32) :
        type == UNIFORM_TYPE_mat4 ? 16 * sizeof(f32) :
    0;
}

static char* shader_type_name(shader_pass_type_t type) {
    return
        type == SHADER_PASS_VERTEX ? "vertex" :
        type == SHADER_PASS_FRAGMENT ? "fragment" :
        type == SHADER_PASS_COMPUTE ? "compute" :
    "unknown";
}
shader_t shader_new(shader_info_t info) {
    shader_t shader = {0};
    shader_init_func shader_init = jumptable->shader_init;
    if(!shader_init) PANIC("unsupported backend [%s] for %s\n", gfx_backend_info().name_pretty, __FUNCTION__);

    shader_init(&shader, info);

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

    return shader;
}

void shader_destroy(shader_t* shader) {
    shader_destroy_func shader_destroy_gfx = jumptable->shader_destroy;
    if(!shader_destroy_gfx) PANIC("unsupported backend [%s] for %s\n", gfx_backend_info().name_pretty, __FUNCTION__);
    shader_destroy_gfx(shader);
}

void shader_update_uniforms(shader_t* shader, range_t data) {
    shader_update_uniforms_func shader_update_uniforms_gfx = jumptable->shader_update_uniforms;
    if(!shader_update_uniforms_gfx) PANIC("unsupported backend [%s] for %s\n", gfx_backend_info().name_pretty, __FUNCTION__);
    shader_update_uniforms_gfx(shader, data);
}

// OPENGL-SPECIFIC
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

static u32 gl_shader_type(shader_pass_type_t type) {
    return
        type == SHADER_PASS_VERTEX ? GL_VERTEX_SHADER :
        type == SHADER_PASS_FRAGMENT ? GL_FRAGMENT_SHADER :
        type == SHADER_PASS_COMPUTE ? GL_COMPUTE_SHADER :
    0;
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
            default: break;
        }

        read_size += size;
        if(read_size > uniforms.size) {
            LOG_ERR("tried to write more data than was provided. ignoring.\n");
            return;
        }
    }
}
