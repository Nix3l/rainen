#include "timer.h"

void timer_update(timer_t *timer) {
    timer->dt = platform_get_milli_diff_reset(&timer->tick);
}
