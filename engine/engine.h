#ifndef ENGINE_H
#define ENGINE_H

#include "base.h"

#include "im_gui/im_gui.h"

#include "memory/memory.h"
#include "io/window.h"
#include "io/input.h"
#include "mesh/mesh.h"
#include "framebuffer/fbo.h"
#include "shader/shader.h"
#include "camera/camera.h"
#include "render/renderer.h"
#include "shader/default_shader.h"
#include "shader/text_shader.h"
#include "entity/entity.h"
#include "physics/physics.h"
#include "font/font.h"

// GOALS FOR V0.1 ---------
// => change up entity manager
// => clean up older code a bit
// => basic physics
// ------------------------

#define ENGINE_VERSION_MAJOR 0
#define ENGINE_VERSION_MINOR 0

typedef struct {
    usize permenant_storage_size;
    void* permenant_storage; // must be cleared to zero on startup

    // TODO(nix3l): maybe keep this, maybe dont
    usize transient_storage_size;
    void* transient_storage; // must be cleared to zero on startup
} engine_memory_s;

typedef struct {
    // ARENAS
    arena_s fbo_arena; // for storing data about fbo textures
    arena_s draw_groups_arena; // for storing draw groups for the renderer
    arena_s draw_calls_arena; // for storing draw calls for each group
    arena_s assets_arena; // for storing the asset managers data
    arena_s entities_arena; // for storing the asset managers data
    arena_s physics_objects_arena; // for storing the rigidbody data
    arena_s frame_arena; // for any and all memory that needs to be allocated and used for one frame only

    // IO
    window_s window;
    input_state_s input_state;

    // FRAME STATS
    // TODO(nix3l): scaled + unscaled time
    f64 old_time;
    f64 curr_time;
    f32 delta_time;
    u32 frame_count;
    f32 fps_timer;
    u32 fps_counter;
    u32 fps;

    f32 time_scale;

    // SHADERS
    default_shader_s default_shader;
    text_shader_s text_shader;

    // RENDERER
    camera_s camera;
    renderer_s renderer;

    fbo_s screen_buffer;

    draw_group_s* default_group;
    draw_group_s* text_group;

    // ENTITIES
    entity_handler_s entity_handler;

    // PHYSICS
    physics_ctx_s physics_ctx;

    // IMGUI
    struct ImGuiContext* imgui_ctx;
    struct ImGuiIO* imgui_io;

    // OTHER
    bool show_debug_stats_window;
    bool show_settings_window;
} engine_state_s;

extern engine_memory_s* engine_memory;
extern engine_state_s* engine_state;

// not a big fan of having this macro in this header but i do not care honestly
// much more important things to take care of than proper code placement
// especially when its this insignificant
#define delta_time() (engine_state->delta_time)

#endif
