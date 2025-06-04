#include "gfx.h"
#include "base.h"
#include "base_macros.h"
#include "memory/memory.h"
#include <string.h>

//
//
//    ▄████  █████▒██   ██▒
//   ██▒ ▀█▓██   ▒▒▒ █ █ ▒░
//  ▒██░▄▄▄▒████ ░░░  █   ░
//  ░▓█  ██░▓█▒  ░ ░ █ █ ▒ 
//  ░▒▓███▀░▒█░   ▒██▒ ▒██▒
//   ░▒   ▒ ▒ ░   ▒▒ ░ ░▓ ░
//    ░   ░ ░     ░░   ░▒ ░
//  ░ ░   ░ ░ ░    ░    ░  
//        ░        ░    ░  
//
//

gfx_ctx_t gfx_ctx;

typedef void (*mesh_init_func) (mesh_t*, mesh_info_t);
typedef void (*mesh_destroy_func) (mesh_t*);

typedef struct backend_jumptable_t {
    mesh_init_func mesh_init;
    mesh_destroy_func mesh_destroy;
} backend_jumptable_t;

static backend_jumptable_t backend_jumptables[GFX_BACKEND_NUM] = {0};
static backend_jumptable_t* jumptable = NULL;

static void gl_mesh_init(mesh_t* mesh, mesh_info_t info);
static void gl_mesh_destroy(mesh_t* mesh);

static void init_jumptables() {
    backend_jumptables[GFX_BACKEND_GL] = (backend_jumptable_t) {
        .mesh_init = gl_mesh_init,
        .mesh_destroy = gl_mesh_destroy,
    };

    jumptable = &backend_jumptables[gfx_ctx.backend];
}

static mempool_t mesh_pool;

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
            opengl_core_info,
        },
    };

    if(!gfx_ctx.backend_info[backend].supported)
        PANIC("chosen backend is not supported on current OS\n");

    // for now, keep immutable
    mesh_pool = mempool_alloc_new(GFX_MAX_MESHES, gfx_ctx.backend_info[backend].mesh_id_size, EXPAND_TYPE_IMMUTABLE);
    gfx_ctx.mesh_pool = &mesh_pool;

    init_jumptables();
}

void gfx_terminate() {
    mempool_destroy(gfx_ctx.mesh_pool);
}

gfx_backend_t gfx_backend() {
    return gfx_ctx.backend;
}

gfx_backend_info_t gfx_backend_info() {
    return gfx_ctx.backend_info[gfx_ctx.backend];
}

//
//
//   ▄▄▄      ██▓███  ██▓
//  ▒████▄   ▓██░  ██▓██▒
//  ▒██  ▀█▄ ▓██░ ██▓▒██▒
//  ░██▄▄▄▄██▒██▄█▓▒ ░██░
//   ▓█   ▓██▒██▒ ░  ░██░
//   ▒▒   ▓▒█▒▓▒░ ░  ░▓  
//    ▒   ▒▒ ░▒ ░     ▒ ░
//    ░   ▒  ░░       ▒ ░
//        ░  ░        ░  
//
//

static u32 mesh_format_num_attributes(mesh_format_t format) {
    switch(format) {
        case MESH_FORMAT_X3T2N3: return 3;
        case MESH_FORMAT_DEFAULT: return 3;
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
    mesh_init_func mesh_init = jumptable->mesh_init;
    if(!mesh_init) PANIC("unsupported backend [%s] for %s\n", gfx_backend_info().name_pretty, __FUNCTION__);
    mesh_init(&mesh, info);

    mesh.format = info.format == MESH_FORMAT_DEFAULT ? MESH_FORMAT_X3T2N3 : info.format;
    mesh.index_type = info.index_type == MESH_INDEX_TYPE_DEFAULT ? MESH_INDEX_TYPE_NONE : info.index_type;
    mesh.primitive = info.primitive == MESH_PRIMITIVE_DEFAULT ? MESH_PRIMITIVE_TRIANGLES : info.primitive;
    mesh.winding = info.winding == MESH_WINDING_DEFAULT ? MESH_WINDING_CCW : info.winding;

    return mesh;
}

//
//
//   ▒█████  ██▓███ ▓█████ ███▄    █  ▄████ ██▓    
//  ▒██▒  ██▓██░  ██▓█   ▀ ██ ▀█   █ ██▒ ▀█▓██▒    
//  ▒██░  ██▓██░ ██▓▒███  ▓██  ▀█ ██▒██░▄▄▄▒██░    
//  ▒██   ██▒██▄█▓▒ ▒▓█  ▄▓██▒  ▐▌██░▓█  ██▒██░    
//  ░ ████▓▒▒██▒ ░  ░▒████▒██░   ▓██░▒▓███▀░██████▒
//  ░ ▒░▒░▒░▒▓▒░ ░  ░░ ▒░ ░ ▒░   ▒ ▒ ░▒   ▒░ ▒░▓  ░
//    ░ ▒ ▒░░▒ ░     ░ ░  ░ ░░   ░ ▒░ ░   ░░ ░ ▒  ░
//  ░ ░ ░ ▒ ░░         ░     ░   ░ ░░ ░   ░  ░ ░   
//      ░ ░            ░  ░        ░      ░    ░  ░
//
//

static u32 gl_vbo_create(u32 attribute, u32 dimensions, f32* data, u32 bytes) {
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
    gl_mesh_info_t* glmesh = mempool_push(gfx_ctx.mesh_pool, &mesh->handle);
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
    gl_mesh_info_t* glmesh = mempool_get(gfx_ctx.mesh_pool, mesh->handle);
    if(!glmesh) return;

    // glDeleteBuffers simply ignores any 0's or invalid ids
    // so this is perfectly fine
    glDeleteBuffers(GFX_MAX_VERTEX_ATTRIBS, glmesh->vbos);
    glDeleteBuffers(1, &glmesh->index_vbo);
    glDeleteVertexArrays(1, &glmesh->vao);
}
