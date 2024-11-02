#define STB_IMAGE_IMPLEMENTATION
#define STB_RECT_PACK_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION

// NOTE(nix3l): YOU ARE NOT WORKING ON AN ENGINE
//              THIS IS A *GAME*

// ENGINE FEATURES ------------------------
// TODO(nix3l): basic physics
// => LOW PRIORITY ------------------------
// TODO(nix3l): add compact list data structure to utils
// TODO(nix3l): decide the scene layout and serialisation method
// TODO(nix3l): scene editor
// TODO(nix3l): GUI system

// FIXES ----------------------------------
// TODO(nix3l): cant include engine files in game dir
// => LOW PRIORITY ------------------------
// TODO(nix3l):

#include "engine.h"
#include "asset/asset.h"
#include "util/log.h"
#include "util/math.h"

// TODO(nix3l): this is a slightly older version of cimgui
// since i completely forget how i compiled the backends the first time around lol
// at some point probably try to compile the new backends. the new version of the main lib is still here though

#include "platform/platform.h"

engine_memory_s* engine_memory = NULL;
engine_state_s* engine_state = NULL;

asset_manager_s* asset_manager = NULL;

static void show_debug_stats_window() {
    if(is_key_pressed(GLFW_KEY_F1)) engine_state->show_debug_stats_window = !engine_state->show_debug_stats_window;
    if(!engine_state->show_debug_stats_window) return;
    igBegin("stats", &engine_state->show_debug_stats_window, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    // FRAME STATS
    igSeparator();
    igText("elapsed time: %lf\n", engine_state->curr_time);
    igText("delta time: %f\n", engine_state->delta_time);
    igText("frame count: %u\n", engine_state->frame_count);
    igText("fps: %u\n", engine_state->fps);

    igSeparator();

    // MEMORY
    igText("total transient memory: %u\n", engine_memory->transient_storage_size);
    igText("total permenant memory: %u\n", engine_memory->permenant_storage_size);

    igText("permenant memory in use: %u/%u\n",
            sizeof(engine_state_s) + 
            engine_state->fbo_arena.size +
            engine_state->draw_groups_arena.size + 
            engine_state->draw_calls_arena.size + 
            engine_state->assets_arena.size,
            engine_memory->permenant_storage_size);
    igIndent(12.0f);
    igText("of which state: %u\n", sizeof(engine_state_s));
    igText("of which framebuffers: %u/%u\n", engine_state->fbo_arena.size, engine_state->fbo_arena.capacity);
    igText("of which draw groups: %u/%u\n", engine_state->draw_groups_arena.size, engine_state->draw_groups_arena.capacity);
    igText("of which draw calls: %u/%u\n", engine_state->draw_calls_arena.size, engine_state->draw_calls_arena.capacity);
    igText("of which frame: %u/%u\n", engine_state->frame_arena.size, engine_state->frame_arena.capacity);
    igUnindent(12.0f);

    igEnd();
}

static void show_settings_window() {
    if(is_key_pressed(GLFW_KEY_F2)) engine_state->show_settings_window = !engine_state->show_settings_window;
    if(!engine_state->show_settings_window) return;

    igBegin("settings", &engine_state->show_settings_window, ImGuiWindowFlags_None);

    // SETTINGS

    if(igCollapsingHeader_TreeNodeFlags("camera", ImGuiTreeNodeFlags_None)) {
        igPushID_Str("camera_settings");
        igDragFloat("sensetivity", &engine_state->camera.sens, 10.0f, 0.0f, MAX_f32, "%.0f", ImGuiSliderFlags_None);
        igDragFloat("move speed", &engine_state->camera.speed, 1.0f, 0.0f, MAX_f32, "%.0f", ImGuiSliderFlags_None);

        igDragFloat3("position", engine_state->camera.position.raw, 0.1f, -MAX_f32, MAX_f32, "%.1f", ImGuiSliderFlags_None);
        igDragFloat3("rotation", engine_state->camera.rotation.raw, 0.1f, -MAX_f32, MAX_f32, "%.1f", ImGuiSliderFlags_None);

        igDragFloat("zoom", &engine_state->camera.zoom, 0.005f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);
        igPopID();
    }
  
    igEnd();
}

// NOTE(nix3l): this is a very rudimentary version of time profiling
// later on i should implement a stack based thing where i can push and pop
// before/after actions to more accurately profile, but that goes beyond
// the scope of this project
static void update_frame_stats() {
    engine_state->old_time = engine_state->curr_time;
    engine_state->curr_time = glfwGetTime() * engine_state->time_scale;
    engine_state->delta_time = engine_state->curr_time - engine_state->old_time;
    engine_state->frame_count ++;
    engine_state->fps_counter ++;
    engine_state->fps_timer += engine_state->delta_time;

    // update fps every 0.16s or so
    // makes it flicker less than updating it every frame
    if(engine_state->fps_timer >= (1/6.0f) * engine_state->time_scale) {
        engine_state->fps = engine_state->fps_counter / engine_state->fps_timer;
        engine_state->fps_counter = 0;
        engine_state->fps_timer = 0.0f;
    }
}

static arena_s partition_permenant_memory(void** mem, usize size, usize* remaining) {
    ASSERT(size <= *remaining);
    arena_s arena = arena_create_in_block(*mem, size);

    *remaining -= size;
    *mem += size;
    return arena;
}

static void init_engine_state(usize permenant_memory_to_allocate, usize transient_memory_to_allocate) {
    ASSERT(sizeof(engine_state_s) < permenant_memory_to_allocate);

    // INITIALISE MEMORY
    engine_memory = mem_alloc(sizeof(engine_memory_s));
    ASSERT(engine_memory);

    engine_memory->permenant_storage_size = permenant_memory_to_allocate;
    engine_memory->permenant_storage = mem_alloc(permenant_memory_to_allocate);
    ASSERT(engine_memory->permenant_storage);
    MEM_ZERO(engine_memory->permenant_storage, engine_memory->permenant_storage_size);

    engine_memory->transient_storage_size = transient_memory_to_allocate;
    engine_memory->transient_storage = mem_alloc(transient_memory_to_allocate);
    ASSERT(engine_memory->transient_storage);
    MEM_ZERO(engine_memory->transient_storage, engine_memory->transient_storage_size);

    // PARTITIONING MEMORY
    engine_state = engine_memory->permenant_storage;

    void* memory = engine_memory->permenant_storage + sizeof(engine_state_s);
    usize remaining_memory = permenant_memory_to_allocate - sizeof(engine_state_s);

    engine_state->fbo_arena             = partition_permenant_memory(&memory, KILOBYTES(1), &remaining_memory);
    engine_state->draw_groups_arena     = partition_permenant_memory(&memory, MAX_DRAW_GROUPS * sizeof(draw_group_s), &remaining_memory);
    engine_state->draw_calls_arena      = partition_permenant_memory(&memory, MAX_DRAW_CALLS * MAX_DRAW_GROUPS * sizeof(draw_call_s), &remaining_memory);
    engine_state->assets_arena          = partition_permenant_memory(&memory, 1024 * sizeof(asset_s), &remaining_memory);
    engine_state->entities_arena        = partition_permenant_memory(&memory, 1024 * sizeof(entity_s), &remaining_memory);
    engine_state->physics_objects_arena = partition_permenant_memory(&memory, 1024 * sizeof(rigidbody_s), &remaining_memory);
    engine_state->frame_arena           = partition_permenant_memory(&memory, remaining_memory, &remaining_memory);

    // IO
    create_window(1600, 900, "rainen");
    init_input();

    // INITIALISE ASSETS
    asset_manager = mem_alloc(sizeof(asset_manager_s));
    ASSERT(asset_manager);
    MEM_ZERO(asset_manager, sizeof(asset_manager_s));

    init_asset_manager(asset_manager,
            &engine_state->assets_arena, &engine_state->frame_arena,
            128, 128);

    // SHADERS
    init_default_shader(&engine_state->default_shader);
    init_text_shader(&engine_state->text_shader);

    // RENDERER
    engine_state->camera = (camera_s) {
        .position     = V3F(0.0f, 0.0f, 5.0f),
        .rotation     = V3F(0.0f, 0.0f, 0.0f),
        
        .near_plane   = 0.001f,
        .far_plane    = 2048.0f,
        .fov          = 70.0f,
        .ortho_width  = 1600.0f,
        .ortho_height = 900.0f,

        .zoom         = 1.0f,

        .speed        = 128.0f,
        .sens         = 7500.0f,
    };

    init_renderer(&engine_state->renderer, &engine_state->draw_groups_arena, &engine_state->screen_buffer);

    engine_state->screen_buffer = create_fbo(
            engine_state->window.width,
            engine_state->window.height,
            1,
            &engine_state->fbo_arena);

    fbo_create_texture(&engine_state->screen_buffer, GL_COLOR_ATTACHMENT0, TEXTURE_RGB, TEXTURE_16b);
    fbo_create_depth_texture(&engine_state->screen_buffer);

    engine_state->default_group = push_draw_group(&engine_state->renderer, &engine_state->default_shader.program, &engine_state->camera);
    engine_state->text_group = push_draw_group(&engine_state->renderer, &engine_state->text_shader.program, &engine_state->camera);

    // ENTITES
    init_entity_handler(&engine_state->entity_handler, &engine_state->entities_arena, 1024);

    // PHYSICS
    init_physics_ctx(&engine_state->physics_ctx, &engine_state->physics_objects_arena, 16);

    // GUI
    init_imgui();
    engine_state->time_scale = 1.0f;

    glfwSetInputMode(engine_state->window.glfw_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
}

static void terminate_engine() {
    shutdown_imgui();
    destroy_window();

    mem_free(engine_memory->transient_storage);
    mem_free(engine_memory->permenant_storage);
    mem_free(engine_memory);
}

int main(void) {
    init_engine_state(GIGABYTES(1), MEGABYTES(16));

    texture_s texture = load_texture("res/test.png");

    font_s font;
    init_font(&font, "res/font/tamzen.ttf", &engine_state->frame_arena);

    entity_s* entity = create_entity(&engine_state->entity_handler);
    entity->position = V2F(-250.0f, 0.0f);
    entity->sprite = (sprite_s) {
        .texture = &texture,

        .offset = V2F_ZERO(),
        .rotation = 0.0f,
        .scale = V2F(6.0f, 6.0f),

        .color = V4F_ZERO(),
    };

    rigidbody_s* rb = physics_register_entity(&engine_state->physics_ctx, entity->handle);

    while(!glfwWindowShouldClose(engine_state->window.glfw_window)) {
        // UPDATE
        update_frame_stats();

        update_camera(&engine_state->camera);
        
        apply_force(rb, engine_state->physics_ctx.gravity);
        integrate_physics(&engine_state->physics_ctx);

        // RENDER
        fbo_clear(&engine_state->screen_buffer, V3F_RGB(0.0f, 0.0f, 0.0f), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render_entity(entity);

        push_text_draw_call(engine_state->text_group, &font, 15, "hello world!", strlen("hello world!"), V2F(0.0f, 128.0f), &engine_state->frame_arena);
        push_text_draw_call(engine_state->text_group, &font, 2, "small???", strlen("small???"), V2F_ZERO(), &engine_state->frame_arena);

        render_draw_groups(&engine_state->renderer);

        fbo_copy_texture_to_screen(&engine_state->screen_buffer, GL_COLOR_ATTACHMENT0);

        update_imgui();
        show_debug_stats_window();
        show_settings_window();
        render_imgui();

        glfwSwapBuffers(engine_state->window.glfw_window);

        update_input();
        glfwPollEvents();

        // CLEANUP
        cleanup_assets();

        arena_clear(&engine_state->frame_arena);
    }

    terminate_engine();

    return 0;
}
