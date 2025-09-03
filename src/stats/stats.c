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
    stats.frame_ticks = new_ticks;
}

f32 stats_dt() {
    return stats.dt;
}

f32 stats_elapsed_time() {
    return stats.elapsed_time;
}
