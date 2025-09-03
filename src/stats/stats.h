#ifndef _STATS_H
#define _STATS_H

#include "base.h"
#include "timer/timer.h"

typedef struct {
    u64 frame_ticks;
    u64 elapsed_ticks;

    f32 elapsed_time;
    f32 dt;
} profiler_t;

void stats_init();
void stats_start_frame();

f32 stats_dt();
f32 stats_elapsed_time();

#endif
