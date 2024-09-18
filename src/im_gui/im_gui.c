#include "im_gui.h"

#include "game.h"

void init_imgui() {
    game_state->imgui_ctx = igCreateContext(NULL);
    game_state->imgui_io = igGetIO();

    ImGui_ImplGlfw_InitForOpenGL(game_state->window.glfw_window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    igStyleColorsDark(NULL);
}

void shutdown_imgui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    igDestroyContext(game_state->imgui_ctx);
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
