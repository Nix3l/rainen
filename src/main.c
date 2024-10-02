#define STB_IMAGE_IMPLEMENTATION

// TODO(nix3l): fix orthographic projection

#include "game.h"
#include "util/log.h"
#include "util/math.h"

// TODO(nix3l): this is a slightly older version of cimgui
// since i completely forget how i compiled the backends the first time around lol
// at some point probably try to compile the new backends. the new version of the main lib is still here though

#include "platform/platform.h"

game_memory_s* game_memory = NULL;
game_state_s* game_state = NULL;

static void show_debug_stats_window() {
    if(is_key_pressed(GLFW_KEY_F1)) game_state->show_debug_stats_window = !game_state->show_debug_stats_window;
    if(!game_state->show_debug_stats_window) return;
    igBegin("stats", &game_state->show_debug_stats_window, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    // FRAME STATS
    igSeparator();
    igText("elapsed time: %lf\n", game_state->curr_time);
    igText("delta time: %f\n", game_state->delta_time);
    igText("frame count: %u\n", game_state->frame_count);
    igText("fps: %u\n", game_state->fps);

    igSeparator();

    // MEMORY
    igText("total transient memory: %u\n", game_memory->transient_storage_size);
    igText("total permenant memory: %u\n", game_memory->permenant_storage_size);

    igText("permenant memory in use: %u/%u\n",
            sizeof(game_state_s) + 
            game_state->fbo_arena.size +
            game_state->draw_groups_arena.size + 
            game_state->draw_calls_arena.size,
            game_memory->permenant_storage_size);
    igIndent(12.0f);
    igText("of which state: %u\n", sizeof(game_state_s));
    igText("of which framebuffers: %u/%u\n", game_state->fbo_arena.size, game_state->fbo_arena.capacity);
    igText("of which draw groups: %u/%u\n", game_state->draw_groups_arena.size, game_state->draw_groups_arena.capacity);
    igText("of which draw calls: %u/%u\n", game_state->draw_calls_arena.size, game_state->draw_calls_arena.capacity);
    igText("of which frame: %u/%u\n", game_state->frame_arena.size, game_state->frame_arena.capacity);
    igUnindent(12.0f);

    igEnd();
}

static void show_settings_window() {
    if(is_key_pressed(GLFW_KEY_F2)) game_state->show_settings_window = !game_state->show_settings_window;
    if(!game_state->show_settings_window) return;

    igBegin("settings", &game_state->show_settings_window, ImGuiWindowFlags_None);

    // SETTINGS

    if(igCollapsingHeader_TreeNodeFlags("camera", ImGuiTreeNodeFlags_None)) {
        igPushID_Str("camera_settings");
        igDragFloat("sensetivity", &game_state->camera.sens, 10.0f, 0.0f, MAX_f32, "%.0f", ImGuiSliderFlags_None);
        igDragFloat("move speed", &game_state->camera.speed, 1.0f, 0.0f, MAX_f32, "%.0f", ImGuiSliderFlags_None);

        igDragFloat3("position", game_state->camera.position.raw, 0.1f, -MAX_f32, MAX_f32, "%.1f", ImGuiSliderFlags_None);
        igDragFloat3("rotation", game_state->camera.rotation.raw, 0.1f, -MAX_f32, MAX_f32, "%.1f", ImGuiSliderFlags_None);
        igPopID();
    }
  
    igEnd();
}

// NOTE(nix3l): this is a very rudimentary version of time profiling
// later on i should implement a stack based thing where i can push and pop
// before/after actions to more accurately profile, but that goes beyond
// the scope of this project
static void update_frame_stats() {
    game_state->old_time = game_state->curr_time;
    game_state->curr_time = glfwGetTime() * game_state->time_scale;
    game_state->delta_time = game_state->curr_time - game_state->old_time;
    game_state->frame_count ++;
    game_state->fps_counter ++;
    game_state->fps_timer += game_state->delta_time;

    // update fps every 0.16s or so
    // makes it flicker less than updating it every frame
    if(game_state->fps_timer >= (1/6.0f) * game_state->time_scale) {
        game_state->fps = game_state->fps_counter / game_state->fps_timer;
        game_state->fps_counter = 0;
        game_state->fps_timer = 0.0f;
    }
}

static arena_s partition_permenant_memory(void** mem, usize size, usize* remaining) {
    ASSERT(size <= *remaining);
    arena_s arena = arena_create_in_block(*mem, size);

    *remaining -= size;
    *mem += size;
    return arena;
}

static void init_game_state(usize permenant_memory_to_allocate, usize transient_memory_to_allocate) {
    ASSERT(sizeof(game_state_s) < permenant_memory_to_allocate);

    // INITIALISE MEMORY
    game_memory = mem_alloc(sizeof(game_memory_s));
    ASSERT(game_memory);

    game_memory->permenant_storage_size = permenant_memory_to_allocate;
    game_memory->permenant_storage = mem_alloc(permenant_memory_to_allocate);
    ASSERT(game_memory->permenant_storage);
    MEM_ZERO(game_memory->permenant_storage, game_memory->permenant_storage_size);
    
    game_memory->transient_storage_size = transient_memory_to_allocate;
    game_memory->transient_storage = mem_alloc(transient_memory_to_allocate);
    ASSERT(game_memory->transient_storage);
    MEM_ZERO(game_memory->transient_storage, game_memory->transient_storage_size);

    // PARTITIONING MEMORY
    game_state = game_memory->permenant_storage;

    void* memory = game_memory->permenant_storage + sizeof(game_state_s);
    usize remaining_memory = permenant_memory_to_allocate - sizeof(game_state_s);

    game_state->fbo_arena         = partition_permenant_memory(&memory, KILOBYTES(1), &remaining_memory);
    game_state->draw_groups_arena = partition_permenant_memory(&memory, MAX_DRAW_GROUPS * sizeof(draw_group_s), &remaining_memory);
    game_state->draw_calls_arena  = partition_permenant_memory(&memory, MAX_DRAW_CALLS * MAX_DRAW_GROUPS * sizeof(draw_call_s), &remaining_memory);
    game_state->frame_arena       = partition_permenant_memory(&memory, remaining_memory, &remaining_memory);

    // IO
    create_window(1600, 900, "rainen");
    init_input();

    // SHADERS
    init_default_shader(&game_state->default_shader);

    // RENDERER
    game_state->camera = (camera_s) {
        .position     = V3F(0.0f, 0.0f, 5.0f),
        .rotation     = V3F(0.0f, 0.0f, 0.0f),
        
        .near_plane   = 0.001f,
        .far_plane    = 2048.0f,
        .fov          = 70.0f,
        .ortho_width  = 1600.0f,
        .ortho_height = 900.0f,

        .speed        = 8.0f,
        .sens         = 7500.0f
    };

    game_state->unit_square = primitive_unit_square();

    init_renderer(&game_state->renderer, &game_state->draw_groups_arena);

    game_state->screen_buffer = create_fbo(
            game_state->window.width,
            game_state->window.height,
            1,
            &game_state->fbo_arena);

    fbo_create_texture(&game_state->screen_buffer, GL_COLOR_ATTACHMENT0, GL_RGB16F, GL_RGB);
    fbo_create_depth_texture(&game_state->screen_buffer);

    game_state->default_group = push_draw_group(&game_state->renderer, &game_state->default_shader.program, &game_state->camera);

    // ENTITES
    init_entity_handler(&game_state->entity_handler);

    // GUI
    init_imgui();
    game_state->time_scale = 1.0f;

    glfwSetInputMode(game_state->window.glfw_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
}

static void terminate_game() {
    shutdown_imgui();
    destroy_window();

    mem_free(game_memory->transient_storage);
    mem_free(game_memory->permenant_storage);
    mem_free(game_memory);
}

int main(void) {
    init_game_state(GIGABYTES(1), MEGABYTES(16));

    entity_s* entity = create_entity(&game_state->entity_handler);
    entity->position = V2F_ZERO();
    entity->sprite = (sprite_s) {
        .texture = NULL,

        .offset = V2F_ZERO(),
        .rotation = 0.0f,
        .scale = V2F_ONE(),

        .color = V4F_ONE(),
    };

    while(!glfwWindowShouldClose(game_state->window.glfw_window)) {
        // UPDATE
        update_frame_stats();

        update_camera(&game_state->camera);

        // RENDER
        fbo_clear(&game_state->screen_buffer, V3F_RGB(0.0f, 0.0f, 0.0f), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render_entity(entity);

        render_draw_groups(&game_state->renderer);

        fbo_copy_texture_to_screen(&game_state->screen_buffer, GL_COLOR_ATTACHMENT0);

        update_imgui();
        show_debug_stats_window();
        show_settings_window();
        render_imgui();

        arena_clear(&game_state->frame_arena);

        glfwSwapBuffers(game_state->window.glfw_window);

        update_input();
        glfwPollEvents();
    }

    terminate_game();

    return 0;
}
