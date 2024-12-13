#define STB_IMAGE_IMPLEMENTATION
#define STB_RECT_PACK_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION

// NOTE(nix3l): YOU ARE NOT WORKING ON AN ENGINE
//              THIS IS A *GAME*

// NOTE(nix3l): ANY METHOD THAT IS NAMED <DEVfoo()>
//              IS ***NOT*** READY FOR PRODUCTION
//              DO NOT SHIP UNDER ANY CIRCUMSTANCES

// ENGINE FEATURES ------------------------
// TODO(nix3l): 
// => LOW PRIORITY ------------------------
// TODO(nix3l): change the memory management up
// TODO(nix3l): basic collision resolution 
// TODO(nix3l): decide the scene layout and dev serialisation method
// TODO(nix3l): scene editor
// TODO(nix3l): GUI system

// FIXES ----------------------------------
// TODO(nix3l): cant include engine files in game dir
// => LOW PRIORITY ------------------------
// TODO(nix3l):

#include "engine.h"
#include "asset/asset.h"
#include "util/util.h"

// TODO(nix3l): this is a slightly older version of cimgui
// since i completely forget how i compiled the backends the first time around lol
// at some point probably try to compile the new backends. the new version of the main lib is still here though

#include "platform/platform.h"

engine_memory_s* engine_memory = NULL;
engine_state_s* engine = NULL;

asset_manager_s* asset_manager = NULL;

static void show_debug_stats_window() {
    if(is_key_pressed(GLFW_KEY_F1)) engine->show_debug_stats_window = !engine->show_debug_stats_window;
    if(!engine->show_debug_stats_window) return;
    igBegin("stats", &engine->show_debug_stats_window, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    // FRAME STATS
    igSeparator();
    igText("elapsed time: %lf\n", engine->curr_time);
    igText("delta time: %f\n", engine->delta_time);
    igText("frame count: %u\n", engine->frame_count);
    igText("fps: %u\n", engine->fps);

    igSeparator();

    // MEMORY
    igText("total transient memory: %u\n", engine_memory->transient_storage_size);
    igText("total permenant memory: %u\n", engine_memory->permenant_storage_size);

    igText("permenant memory in use: %u/%u\n",
            sizeof(engine_state_s) + 
            engine->fbo_arena.size +
            engine->draw_groups_arena.size + 
            engine->draw_calls_arena.size + 
            engine->assets_arena.size,
            engine_memory->permenant_storage_size);
    igIndent(12.0f);
    igText("of which state: %u\n", sizeof(engine_state_s));
    igText("of which framebuffers: %u/%u\n", engine->fbo_arena.size, engine->fbo_arena.capacity);
    igText("of which draw groups: %u/%u\n", engine->draw_groups_arena.size, engine->draw_groups_arena.capacity);
    igText("of which draw calls: %u/%u\n", engine->draw_calls_arena.size, engine->draw_calls_arena.capacity);
    igText("of which frame: %u/%u\n", engine->frame_arena.size, engine->frame_arena.capacity);
    igUnindent(12.0f);

    igEnd();
}

static void show_settings_window() {
    if(is_key_pressed(GLFW_KEY_F2)) engine->show_settings_window = !engine->show_settings_window;
    if(!engine->show_settings_window) return;

    igBegin("settings", &engine->show_settings_window, ImGuiWindowFlags_None);

    // SETTINGS

    if(igCollapsingHeader_TreeNodeFlags("camera", ImGuiTreeNodeFlags_None)) {
        igPushID_Str("camera_settings");
        igDragFloat("sensetivity", &engine->camera.sens, 10.0f, 0.0f, MAX_f32, "%.0f", ImGuiSliderFlags_None);
        igDragFloat("move speed", &engine->camera.speed, 1.0f, 0.0f, MAX_f32, "%.0f", ImGuiSliderFlags_None);

        igDragFloat3("position", engine->camera.position.raw, 0.1f, -MAX_f32, MAX_f32, "%.1f", ImGuiSliderFlags_None);
        igDragFloat3("rotation", engine->camera.rotation.raw, 0.1f, -MAX_f32, MAX_f32, "%.1f", ImGuiSliderFlags_None);

        igDragFloat("zoom", &engine->camera.zoom, 0.005f, 0.0f, MAX_f32, "%.3f", ImGuiSliderFlags_None);
        igPopID();
    }
  
    igEnd();
}

// NOTE(nix3l): this is a very rudimentary version of time profiling
// later on i should implement a stack based thing where i can push and pop
// before/after actions to more accurately profile, but that goes beyond
// the scope of this project
static void update_frame_stats() {
    engine->old_time = engine->curr_time;
    engine->curr_time = glfwGetTime() * engine->time_scale;
    engine->delta_time = engine->curr_time - engine->old_time;
    engine->frame_count ++;
    engine->fps_counter ++;
    engine->fps_timer += engine->delta_time;

    // update fps every 0.16s or so
    // makes it flicker less than updating it every frame
    if(engine->fps_timer >= (1/6.0f) * engine->time_scale) {
        engine->fps = engine->fps_counter / engine->fps_timer;
        engine->fps_counter = 0;
        engine->fps_timer = 0.0f;
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
    engine = engine_memory->permenant_storage;

    void* memory = engine_memory->permenant_storage + sizeof(engine_state_s);
    usize remaining_memory = permenant_memory_to_allocate - sizeof(engine_state_s);

    engine->fbo_arena             = partition_permenant_memory(&memory, KILOBYTES(1), &remaining_memory);
    engine->draw_groups_arena     = partition_permenant_memory(&memory, MAX_DRAW_GROUPS * sizeof(draw_group_s), &remaining_memory);
    engine->draw_calls_arena      = partition_permenant_memory(&memory, MAX_DRAW_CALLS * MAX_DRAW_GROUPS * sizeof(draw_call_s), &remaining_memory);
    engine->assets_arena          = partition_permenant_memory(&memory, 1024 * sizeof(asset_s), &remaining_memory);
    engine->entities_arena        = partition_permenant_memory(&memory, 1024 * sizeof(entity_s), &remaining_memory);
    engine->physics_objects_arena = partition_permenant_memory(&memory, 1024 * sizeof(rigidbody_s), &remaining_memory);
    engine->static_objects_arena  = partition_permenant_memory(&memory, 1024 * sizeof(static_collider_s), &remaining_memory);
    engine->frame_arena           = partition_permenant_memory(&memory, remaining_memory, &remaining_memory);

    // IO
    create_window(1600, 900, "rainen");
    init_input();

    // INITIALISE ASSETS
    asset_manager = mem_alloc(sizeof(asset_manager_s));
    ASSERT(asset_manager);
    MEM_ZERO(asset_manager, sizeof(asset_manager_s));

    init_asset_manager(asset_manager,
            &engine->assets_arena, &engine->frame_arena,
            128, 128);

    // SHADERS
    init_default_shader(&engine->default_shader);
    init_text_shader(&engine->text_shader);
    init_debug_shader(&engine->debug_shader);

    // PRIMITIVES
    engine->primitive_square = primitive_unit_square();
    engine->primitive_line   = primitive_line();
    engine->primitive_point  = primitive_point();

    // RENDERER
    engine->camera = (camera_s) {
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

    init_renderer(&engine->renderer, &engine->draw_groups_arena, &engine->screen_buffer);

    engine->screen_buffer = create_fbo(
            engine->window.width,
            engine->window.height,
            1,
            &engine->fbo_arena);

    fbo_create_texture(&engine->screen_buffer, GL_COLOR_ATTACHMENT0, TEXTURE_RGB, TEXTURE_16b);
    fbo_create_depth_texture(&engine->screen_buffer);

    engine->default_group = push_draw_group(&engine->renderer, &engine->default_shader.program, &engine->camera);
    engine->text_group = push_draw_group(&engine->renderer, &engine->text_shader.program, &engine->camera);
    engine->debug_group = push_draw_group(&engine->renderer, &engine->debug_shader.program, &engine->camera);

    // ENTITES
    init_entity_handler(&engine->entity_handler, &engine->entities_arena, 12);

    // PHYSICS
    init_physics_ctx(&engine->physics_ctx, &engine->physics_objects_arena, 16, &engine->static_objects_arena, 16);

    // GUI
    init_imgui();
    engine->time_scale = 1.0f;

    glfwSetInputMode(engine->window.glfw_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
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
    init_font(&font, "res/font/tamzen.ttf", &engine->frame_arena);

    entity_s* ent = create_entity(&engine->entity_handler);
    ent->position = V2F(0.0f, 800.0f);
    ent->sprite = (sprite_s) {
        .texture = NULL,

        .offset = V2F_ZERO,
        .rotation = 0.0f,
        .scale = V2F(6.0f, 6.0f),

        .color = V4F(0.05f, 0.5f, 0.2f, 1.0f),
    };

    rigidbody_s* rb = physics_register_entity(&engine->physics_ctx, ent->handle);
    rb->mass = 10.0f;
    // rb->box = sprite_bounding_box(&ent->sprite);
    rb->box = aabb_create_dimensions(100.0f, 100.0f);

    aabb_s static_collider = aabb_create_dimensions(100.0f, 100.0f);
    physics_register_static_collider(&engine->physics_ctx, static_collider);

    while(!glfwWindowShouldClose(engine->window.glfw_window)) {
        // UPDATE
        update_frame_stats();

        update_entities(&engine->entity_handler);
        update_camera(&engine->camera);

        v2f pos = get_mouse_pos();
        pos.y = 900.0f - pos.y;

        pos.x += engine->camera.position.x;
        pos.y += engine->camera.position.y;

        process_physics(&engine->physics_ctx);

        // RENDER
        fbo_clear(&engine->screen_buffer, V3F_RGB(0.0f, 0.0f, 0.0f), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        render_entities(&engine->entity_handler);
        render_draw_groups(&engine->renderer);
        fbo_copy_texture_to_screen(&engine->screen_buffer, GL_COLOR_ATTACHMENT0);

        update_imgui();
        show_debug_stats_window();
        show_settings_window();
        render_imgui();

        glfwSwapBuffers(engine->window.glfw_window);

        update_input();
        glfwPollEvents();

        // CLEANUP
        cleanup_assets();

        arena_clear(&engine->frame_arena);
    }

    terminate_engine();

    return 0;
}
