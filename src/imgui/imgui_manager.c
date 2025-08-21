#include "imgui_manager.h"
#include "io/io.h"

// NOTE(nix3l): for my future idiot self who will no doubt have forgotten how to compile cimgui backends
// and is desperately looking at old repos trying to find the solution:
//  => you have to edit the main Makefile in cimgui
//  => add the cimgui_impl.cpp file to the OBJS
//  => add the backends you want from the imgui folder "./imgui/backends/xxx"
//  => add the libraries needed for those backends to link using pkg-config --static --libs (glfw3, GL etc)
//  => add the compiler definitions CIMGUI_USE_XXX
//  => set IMGUI_IMPL_API="extern \"C\""
//  => only then can you link against the resulting so file

static struct {
    struct ImGuiContext* ctx;
    struct ImGuiIO* io;
} state;

void imgui_init() {
    ASSERT(gfx_backend() == GFX_BACKEND_GL);

    state.ctx = igCreateContext(NULL);
    state.io = igGetIO_Nil();
    state.io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    state.io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    static const char* glsl_version = "#version 330 core";
    ImGui_ImplGlfw_InitForOpenGL(io_ctx.window.gl_window.id, true);
    ASSERT(ImGui_ImplOpenGL3_Init(glsl_version));

    igStyleColorsDark(NULL);
}

void imgui_terminate() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    igDestroyContext(NULL);
}

void imgui_start_frame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();
}

void imgui_show() {
    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}

void imgui_texture_image(texture_t tex, v2f size) {
    if(tex.id == 0) return;
    gfx_res_slot_t* slot = pool_get(&gfx_ctx.texture_pool->res_pool, tex.id);
    struct { u32 id; } *internal = pool_get(&gfx_ctx.texture_pool->internal_pool, slot->internal_handle);
    igImage((ImTextureRef) { ._TexData = NULL, ._TexID = internal->id, }, imv2f(size.x, size.y), imv2f(0.0f, 0.0f), imv2f(1.0f, 1.0f));
}
