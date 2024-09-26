#ifndef GAME_H
#define GAME_H

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

typedef struct {
    usize permenant_storage_size;
    void* permenant_storage; // must be cleared to zero on startup

    usize transient_storage_size;
    void* transient_storage; // must be cleared to zero on startup
} game_memory_s;

typedef struct {
    // ARENAS
    arena_s frame_arena; // for any and all memory that needs to be allocated and used for one frame only
    arena_s fbo_arena; // for storing data about fbo textures
    arena_s draw_groups_arena; // for storing draw groups for the renderer
    arena_s draw_calls_arena; // for storing draw calls for each group

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

    // RENDERER
    camera_s camera;
    renderer_s renderer;

    mesh_s unit_square;
    fbo_s screen_buffer;

    draw_group_s* default_group;

    // IMGUI
    struct ImGuiContext* imgui_ctx;
    struct ImGuiIO* imgui_io;

    // OTHER
    bool show_debug_stats_window;
    bool show_settings_window;
} game_state_s;

extern game_memory_s* game_memory;
extern game_state_s* game_state;

// not a big fan of having this macro in this header but i do not care honestly
// much more important things to take care of than proper code placement
// especially when its this insignificant
#define delta_time() (game_state->delta_time)

#endif
