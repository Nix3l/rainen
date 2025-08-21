#include "editor.h"
#include "imgui/imgui_manager.h"
#include "io/io.h"
#include "memory/memory.h"
#include "util/util.h"
#include "gfx/gfx.h"

// temporary because my cmp keeps auto including this stupid file and its causing errors
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

editor_ctx_t editor_ctx = {0};

// STATE
void editor_init() {
    editor_ctx = (editor_ctx_t) {
        .open = true,

        .editor = {
            .open = true,
        },

        .resviewer = {
            .open = true,
            .selected_texture = 0,
            .texture = { 0 },
        },
    };
}

void editor_terminate() {
    // do stuff
}

void editor_set_open(bool open) {
    editor_ctx.open = open;
}

void editor_toggle() {
    editor_ctx.open = !editor_ctx.open;
}

bool editor_is_open() {
    return editor_ctx.open;
}

// EDITOR UI
static void editor_main() {
    if(!igBegin("editor", &editor_ctx.editor.open, ImGuiWindowFlags_None)) {
        igEnd();
        return;
    }

    igEnd();
}

static void editor_resviewer() {
    if(!igBegin("resviewer", &editor_ctx.resviewer.open, ImGuiWindowFlags_None)) {
        igEnd();
        return;
    }

    igSetNextItemOpen(true, ImGuiCond_Once);
    if(igCollapsingHeader_BoolPtr("textures", NULL, ImGuiTreeNodeFlags_None)) {
        igBeginChild_Str("texturelist", imv2f(120.0f, 0.0f), ImGuiChildFlags_Borders, ImGuiWindowFlags_None);
        pool_iter_t iter = { .absolute_index = 1, };
        while(pool_iter(&gfx_ctx.texture_pool->res_pool, &iter)) {
            char label[32];
            snprintf(label, 32, "texture %u", iter.absolute_index);
            if(igSelectable_Bool(label, iter.absolute_index == editor_ctx.resviewer.selected_texture, ImGuiSelectableFlags_SelectOnClick, imv2f_ZERO)) {
                if(editor_ctx.resviewer.selected_texture == iter.absolute_index) {
                    editor_ctx.resviewer.selected_texture = 0;
                    editor_ctx.resviewer.texture = (texture_t) { 0 };
                } else {
                    editor_ctx.resviewer.selected_texture = iter.absolute_index;
                    editor_ctx.resviewer.texture = (texture_t) { iter.handle };
                }
            }
        }
        igEndChild();

        igSameLine(120.0f, 12.0f);

        igBeginChild_Str("textureviewer", imv2f_ZERO, ImGuiChildFlags_Borders, ImGuiWindowFlags_None);

        if(editor_ctx.resviewer.selected_texture > 0) {
            texture_data_t* texture_data = texture_get_data(editor_ctx.resviewer.texture);
            imgui_texture_image(editor_ctx.resviewer.texture, v2f_new(256.0f, 256.0f));
            igText("width [%u] height [%u]", texture_data->width, texture_data->height);

            // lol
            const char* format_names[] = {
                STRINGIFY(TEXTURE_FORMAT_UNDEFINED),
                STRINGIFY(TEXTURE_FORMAT_R8),
                STRINGIFY(TEXTURE_FORMAT_R8I),
                STRINGIFY(TEXTURE_FORMAT_R8UI),
                STRINGIFY(TEXTURE_FORMAT_R16),
                STRINGIFY(TEXTURE_FORMAT_R16F),
                STRINGIFY(TEXTURE_FORMAT_R16I),
                STRINGIFY(TEXTURE_FORMAT_R16UI),
                STRINGIFY(TEXTURE_FORMAT_R32F),
                STRINGIFY(TEXTURE_FORMAT_R32I),
                STRINGIFY(TEXTURE_FORMAT_R32UI),
                STRINGIFY(TEXTURE_FORMAT_RG8),
                STRINGIFY(TEXTURE_FORMAT_RG8I),
                STRINGIFY(TEXTURE_FORMAT_RG8UI),
                STRINGIFY(TEXTURE_FORMAT_RG16),
                STRINGIFY(TEXTURE_FORMAT_RG16F),
                STRINGIFY(TEXTURE_FORMAT_RG16I),
                STRINGIFY(TEXTURE_FORMAT_RG16UI),
                STRINGIFY(TEXTURE_FORMAT_RG32F),
                STRINGIFY(TEXTURE_FORMAT_RG32I),
                STRINGIFY(TEXTURE_FORMAT_RG32UI),
                STRINGIFY(TEXTURE_FORMAT_RGB8),
                STRINGIFY(TEXTURE_FORMAT_RGB8I),
                STRINGIFY(TEXTURE_FORMAT_RGB8UI),
                STRINGIFY(TEXTURE_FORMAT_RGB16F),
                STRINGIFY(TEXTURE_FORMAT_RGB16I),
                STRINGIFY(TEXTURE_FORMAT_RGB16UI),
                STRINGIFY(TEXTURE_FORMAT_RGB32F),
                STRINGIFY(TEXTURE_FORMAT_RGB32I),
                STRINGIFY(TEXTURE_FORMAT_RGB32UI),
                STRINGIFY(TEXTURE_FORMAT_RGBA8),
                STRINGIFY(TEXTURE_FORMAT_RGBA8I),
                STRINGIFY(TEXTURE_FORMAT_RGBA8UI),
                STRINGIFY(TEXTURE_FORMAT_RGBA16F),
                STRINGIFY(TEXTURE_FORMAT_RGBA16I),
                STRINGIFY(TEXTURE_FORMAT_RGBA16UI),
                STRINGIFY(TEXTURE_FORMAT_RGBA32F),
                STRINGIFY(TEXTURE_FORMAT_RGBA32I),
                STRINGIFY(TEXTURE_FORMAT_RGBA32UI),
                STRINGIFY(TEXTURE_FORMAT_DEPTH),
                STRINGIFY(TEXTURE_FORMAT_DEPTH_STENCIL),
            };

            igText("format [%s]", format_names[texture_data->format]);
            igText("mipmaps [%u]", texture_data->mipmaps);
        } else {
            igText("no texture selected");
        }

        igEndChild();
    }

    igEnd();
}

void editor_update() {
    editor_main();
    editor_resviewer();
}
