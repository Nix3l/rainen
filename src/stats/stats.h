#ifndef _STATS_H
#define _STATS_H

#include "base.h"
#include "timer/timer.h"

typedef struct {
    u64 frame_ticks;
    u64 elapsed_ticks;

    f32 elapsed_time;
    f32 dt;

    // game ticks (frames)
    u32 ticks;

    f32 fps_timer;
    u32 fps_counter;
    u32 fps;
} profiler_t;

void stats_init();
void stats_start_frame();

f32 stats_dt();
f32 stats_elapsed_time();
u32 stats_fps();
u32 stats_ticks();

#endif
