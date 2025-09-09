#include "stats.h"
#include "platform/platform.h"

static profiler_t stats = {0};

void stats_init() {
    stats.frame_ticks = platform_get_ticks();
}

void stats_start_frame() {
    u64 new_ticks = platform_get_ticks();
    stats.elapsed_ticks += new_ticks - stats.frame_ticks;
    stats.dt = platform_ticks_to_sec(new_ticks - stats.frame_ticks);
    stats.elapsed_time += stats.dt;

    stats.ticks ++;

    stats.fps_timer += stats.dt;
    if(stats.fps_timer > 1.00f) {
        stats.fps = stats.fps_counter;
        stats.fps_counter = 0;
        stats.fps_timer = 0.0f;
    } else {
        stats.fps_counter ++;
    }

    stats.frame_ticks = new_ticks;
}

f32 stats_dt() {
    return stats.dt;
}

f32 stats_elapsed_time() {
    return stats.elapsed_time;
}

u32 stats_fps() {
    return stats.fps;
}

u32 stats_ticks() {
    return stats.ticks;
}
