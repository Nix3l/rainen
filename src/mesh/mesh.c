#include "mesh.h"
#include "game.h"
#include "util/log.h"
#include "platform/platform.h"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

static GLuint create_vbo(GLuint attribute, GLuint dimensions, GLfloat* data, u32 data_size) {
    GLuint vbo;
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, data_size * dimensions * sizeof(GLfloat), data, GL_STATIC_DRAW);
    glVertexAttribPointer(attribute, dimensions, GL_FLOAT, GL_FALSE, 0, NULL);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return vbo;
}

static GLuint create_indices_vbo(GLuint* indices, u32 num_indices) {
    GLuint vbo;
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(GLuint), indices, GL_STATIC_DRAW);

    return vbo;
}

mesh_s create_mesh(
        f32* vertex_data, f32* uvs_data, f32* normals_data, f32* colors_data, 
        GLuint* indices,
        u32 num_indices, u32 num_vertices) {
    mesh_s mesh;

    if(!vertex_data || num_vertices == 0) {
        LOG_ERR("tried to load mesh without vertex data!\n");
        return (mesh_s) {};
    }

    mesh.data = MESH_VERTICES;
    if(uvs_data) mesh.data |= MESH_UVS;
    if(normals_data) mesh.data |= MESH_NORMALS;
    if(colors_data) mesh.data |= MESH_COLORS;

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    mesh.vertices_vbo = create_vbo(MESH_ATTRIBUTE_VERTICES, 3, vertex_data, num_vertices);
    mesh.uvs_vbo = uvs_data ? create_vbo(MESH_ATTRIBUTE_UVS, 2, uvs_data, num_vertices) : 0;
    mesh.normals_vbo = normals_data ? create_vbo(MESH_ATTRIBUTE_NORMALS, 3, normals_data, num_vertices) : 0;
    mesh.colors_vbo = colors_data ? create_vbo(MESH_ATTRIBUTE_COLORS, 3, colors_data, num_vertices) : 0;
    mesh.indices_vbo = create_indices_vbo(indices, num_indices);

    mesh.index_count = num_indices;
    mesh.vertex_count = num_vertices;

    mesh.name = NULL;
    mesh.full_path = NULL;

    glBindVertexArray(0);

    return mesh;
}

mesh_s create_mesh_arrays(
        f32* vertex_data, f32* uvs_data, f32* normals_data, f32* colors_data, 
        u32 num_vertices) {
    mesh_s mesh;

    if(!vertex_data || num_vertices == 0) {
        LOG_ERR("tried to load mesh without vertex data!\n");
        return (mesh_s) {};
    }

    mesh.data = MESH_VERTICES;
    if(uvs_data) mesh.data |= MESH_UVS;
    if(normals_data) mesh.data |= MESH_NORMALS;
    if(colors_data) mesh.data |= MESH_COLORS;

    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    mesh.vertices_vbo = create_vbo(MESH_ATTRIBUTE_VERTICES, 3, vertex_data, num_vertices);
    mesh.uvs_vbo = uvs_data ? create_vbo(MESH_ATTRIBUTE_UVS, 2, uvs_data, num_vertices) : 0;
    mesh.normals_vbo = normals_data ? create_vbo(MESH_ATTRIBUTE_NORMALS, 3, normals_data, num_vertices) : 0;
    mesh.colors_vbo = colors_data ? create_vbo(MESH_ATTRIBUTE_COLORS, 3, colors_data, num_vertices) : 0;
    mesh.indices_vbo = 0;

    mesh.index_count = 0;
    mesh.vertex_count = num_vertices;

    mesh.name = NULL;
    mesh.full_path = NULL;

    glBindVertexArray(0);

    return mesh;
}

void mesh_enable_attributes(mesh_s* mesh) {
    if(mesh->data & MESH_VERTICES)
        glEnableVertexAttribArray(MESH_ATTRIBUTE_VERTICES);
    if(mesh->data & MESH_UVS)
        glEnableVertexAttribArray(MESH_ATTRIBUTE_UVS);
    if(mesh->data & MESH_NORMALS)
        glEnableVertexAttribArray(MESH_ATTRIBUTE_NORMALS);
    if(mesh->data & MESH_COLORS)
        glEnableVertexAttribArray(MESH_ATTRIBUTE_COLORS);
}

void mesh_disable_attributes(mesh_s* mesh) {
    if(mesh->data & MESH_VERTICES)
        glDisableVertexAttribArray(MESH_ATTRIBUTE_VERTICES);
    if(mesh->data & MESH_UVS)
        glDisableVertexAttribArray(MESH_ATTRIBUTE_UVS);
    if(mesh->data & MESH_NORMALS)
        glDisableVertexAttribArray(MESH_ATTRIBUTE_NORMALS);
    if(mesh->data & MESH_COLORS)
        glDisableVertexAttribArray(MESH_ATTRIBUTE_COLORS);
}

void destroy_mesh(mesh_s* mesh) {
    glDeleteBuffers(1, &mesh->vertices_vbo);
    glDeleteBuffers(1, &mesh->uvs_vbo);
    glDeleteBuffers(1, &mesh->normals_vbo);
    glDeleteBuffers(1, &mesh->colors_vbo);
    glDeleteBuffers(1, &mesh->indices_vbo);

    glDeleteVertexArrays(1, &mesh->vao);
}

// LOADING FROM DISK
static mesh_s importer_process_mesh(const struct aiMesh* ai_mesh, arena_s* arena) {
    if(!ai_mesh->mVertices) {
        LOG_ERR("tried to load mesh with no vertex data");
        return (mesh_s) {};
    }

    // TODO(nix3l): this function is quite bad currently
    // check if the mesh has the data before loading it
    // add it to the mesh's data flags
    // and use those to allocate the memory (i.e. if there arent any uvs, uvs = NULL)

    f32* vertices   = arena_push(arena, sizeof(f32) * ai_mesh->mNumVertices * 3);
    f32* uvs        = arena_push(arena, sizeof(f32) * ai_mesh->mNumVertices * 2);
    f32* normals    = arena_push(arena, sizeof(f32) * ai_mesh->mNumVertices * 3);
    f32* colors     = arena_push(arena, sizeof(f32) * ai_mesh->mNumVertices * 3);
    // assuming 3 vertices per face at all times
    u32* indices    = arena_push(arena, sizeof(u32) * ai_mesh->mNumFaces * 3);

    u32 curr_vertex_pos = 0;
    u32 curr_uvs = 0;
    u32 curr_normals = 0;
    u32 curr_colors = 0;

    for(usize i = 0; i < ai_mesh->mNumVertices; i ++) {
        vertices[curr_vertex_pos++] = ai_mesh->mVertices[i].x;
        vertices[curr_vertex_pos++] = ai_mesh->mVertices[i].y;
        vertices[curr_vertex_pos++] = ai_mesh->mVertices[i].z;

        if(ai_mesh->mTextureCoords[0]) {
            uvs[curr_uvs++] = ai_mesh->mTextureCoords[0][i].x;
            uvs[curr_uvs++] = ai_mesh->mTextureCoords[0][i].y;
        }

        if(ai_mesh->mNormals) {
            normals[curr_normals++] = ai_mesh->mNormals[i].x;
            normals[curr_normals++] = ai_mesh->mNormals[i].y;
            normals[curr_normals++] = ai_mesh->mNormals[i].z;
        }

        if(ai_mesh->mColors[0]) {
            colors[curr_colors++] = 0.0f;
            colors[curr_colors++] = 0.0f;
            colors[curr_colors++] = 0.0f;
        }
    }

    u32 curr_index = 0;

    for(usize f = 0; f < ai_mesh->mNumFaces; f ++) {
        struct aiFace face = ai_mesh->mFaces[f];

        for(usize i = 0; i < face.mNumIndices; i ++)
            indices[curr_index++] = face.mIndices[i];
    }

    mesh_s mesh = create_mesh(
            vertices, uvs, normals, colors,
            indices,
            ai_mesh->mNumFaces * 3, ai_mesh->mNumVertices
        );
    
    arena_pop(arena, sizeof(GLuint) * ai_mesh->mNumFaces * 3); // indices
    arena_pop(arena, sizeof(f32) * ai_mesh->mNumVertices * 3); // colors
    arena_pop(arena, sizeof(f32) * ai_mesh->mNumVertices * 3); // normals
    arena_pop(arena, sizeof(f32) * ai_mesh->mNumVertices * 2); // uvs
    arena_pop(arena, sizeof(f32) * ai_mesh->mNumVertices * 3); // vertices

    return mesh;
}

mesh_s load_mesh_from_file(char* filename, arena_s* arena) {
    mesh_s mesh;

    char* filepath = platform_get_res_path(filename, arena);
    const u32 import_flags =
        aiProcess_JoinIdenticalVertices |
        aiProcess_Triangulate |
        aiProcess_FixInfacingNormals |
        aiProcess_FlipUVs;

    // NOTE(nix3l): for now, load the file data into transient memory
    // until we are done with it.
    // not the best solution, since if files go a bit larger, then
    // transient memory will have to scale accordingly
    // and we should try to keep the transient memory requirement
    // as little as possible
    arena_s temp_mem = arena_create_in_block(game_memory->transient_storage, game_memory->transient_storage_size);

    usize file_length;
    void* file_data = platform_load_file(filepath, &file_length, &temp_mem);

    const struct aiScene* scene = aiImportFileFromMemory(file_data, file_length, import_flags, platform_get_file_extension(filepath));
    if(!scene) {
        LOG_ERR("couldnt import mesh from file [%s]\n", filepath);
        return (mesh_s) {};
    }

    mesh = importer_process_mesh(scene->mMeshes[0], arena);
    mesh.full_path = filepath;
    // NOTE(nix3l): assuming that the file name is in the arena already
    // so not copying it for now
    mesh.name = filename;

    arena_clear(&temp_mem);
    aiReleaseImport(scene);
    return mesh;
}

// MESH GENERATION
mesh_s primitive_plane_mesh(v3f bottom_left, v2i num_vertices, v2f world_size, arena_s* arena) {
    // NOTE(nix3l): colors are not accounted for in this function
    // however i can always create another one or pass in a color for all the vertices
    // as a parameter to this function if need be
    u32 total_vertices = num_vertices.x * num_vertices.y;

    // -1 to account for the shared vertices
    // i.e. the edge vertices only belong to one face, whereas
    // all the other vertices belong to 2-4
    u32 total_faces = (num_vertices.x - 1) * (num_vertices.y - 1);

    // 2 traingles per quad, 3 indices per triangle
    // therefore 6 indices per quad
    u32 total_indices = total_faces * 6;

    f32* vertices = arena_push(arena, sizeof(f32) * total_vertices * 3);
    f32* uvs      = arena_push(arena, sizeof(f32) * total_vertices * 2);
    f32* normals  = arena_push(arena, sizeof(f32) * total_vertices * 3);
    u32* indices  = arena_push(arena, sizeof(u32) * total_indices);

    f32 x_step = world_size.x / (num_vertices.x - 1);
    f32 z_step = world_size.y / (num_vertices.y - 1);

    f32* curr_pos_value = vertices;
    f32* curr_uvs_value = uvs;
    f32* curr_normals_value = normals;

    for(u32 y = 0; y < num_vertices.y; y ++) {
        for(u32 x = 0; x < num_vertices.x; x ++) {
            *(curr_pos_value ++) = bottom_left.x + (x_step * x);
            *(curr_pos_value ++) = bottom_left.y;
            *(curr_pos_value ++) = bottom_left.z + (z_step * y);

            *(curr_uvs_value ++) = (bottom_left.x + (x_step * x)) / world_size.x;
            *(curr_uvs_value ++) = (bottom_left.z + (z_step * y)) / world_size.y;

            *(curr_normals_value ++) = 0.0f;
            *(curr_normals_value ++) = 1.0f;
            *(curr_normals_value ++) = 0.0f;
        }
    }

    // this offset is kind of hard to explain 
    // but essentially whenever we hit an edge, the vertex at the very edge
    // will belong to less faces than faces in the middle
    // essentially this offset lets us account for that
    // as we are iterating through the vertices
    // from bottom left to top right, row by row
    u32 offset = 0;
    for(u32 i = 0; i < total_indices;) {
        // bottom left vertex in the current face
        u32 face_bottom_left = i / 6 + offset;

        // if we are on an edge
        if((face_bottom_left + 1) % num_vertices.x == 0) {
            offset ++;
            face_bottom_left ++;
        }

        // keep in mind that we are going from bottom left to top right 

        // first triangle
        indices[i++] = face_bottom_left; // bottom left
        indices[i++] = face_bottom_left + num_vertices.x; // top left
        indices[i++] = face_bottom_left + num_vertices.x + 1; // top right

        // second triangle
        indices[i++] = face_bottom_left; // bottom left
        indices[i++] = face_bottom_left + num_vertices.x + 1; // top right
        indices[i++] = face_bottom_left + 1; // bottom right
    }

    mesh_s mesh = create_mesh(
            vertices, uvs, normals,
            NULL, // no colors
            indices,
            total_indices,
            total_vertices
        );

    arena_pop(arena, sizeof(GLuint) * total_indices);   // indices
    arena_pop(arena, sizeof(f32) * total_vertices * 3); // normals
    arena_pop(arena, sizeof(f32) * total_vertices * 2); // uvs
    arena_pop(arena, sizeof(f32) * total_vertices * 3); // vertices

    return mesh;
}

mesh_s primitive_cube_mesh() {
    f32 vertices[] = {
        0.0f, -1.0f, 0.0f, // lower left
        0.0f, -1.0f, 1.0f, // upper left
        0.0f,  1.0f, 0.0f, // lower right
        0.0f,  1.0f, 1.0f, // upper right
        1.0f, -1.0f, 0.0f, // lower front left
        1.0f, -1.0f, 1.0f, // upper front left
        1.0f,  1.0f, 1.0f, // lower front right
        1.0f,  1.0f, 0.0f  // upper front right
    };

    u32 indices[] = {
        0, 2, 3,
        3, 1, 0,
        0, 1, 4,
        4, 1, 5,
        4, 5, 7,
        7, 5, 6,
        7, 6, 3,
        3, 2, 7,
        1, 3, 5,
        6, 5, 3,
        2, 0, 4,
        4, 7, 2
    };

    return create_mesh(
            vertices,
            NULL,
            NULL,
            NULL,
            indices,
            ARRAY_SIZE(indices),
            ARRAY_SIZE(vertices)
        );
}
