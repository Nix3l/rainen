#include "im_gui.h"

#include "engine.h"

void init_imgui() {
    engine->imgui_ctx = igCreateContext(NULL);
    engine->imgui_io = igGetIO();

    ImGui_ImplGlfw_InitForOpenGL(engine->window.glfw_window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    igStyleColorsDark(NULL);
}

void shutdown_imgui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    igDestroyContext(engine->imgui_ctx);
}

void update_imgui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    igNewFrame();
}

void render_imgui() {
    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}
