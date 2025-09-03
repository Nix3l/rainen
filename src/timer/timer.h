#ifndef _TIMER_H
#define _TIMER_H

#include "base.h"
#include "platform/platform.h"

typedef struct {
    u64 tick;
    f32 dt;
} timer_t;

void timer_update(timer_t* timer);

#endif
