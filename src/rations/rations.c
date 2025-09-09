#include "rations.h"

rations_t rations = {0};

void rations_divide() {
    const u32 total =
        RATIONS_EVENTS  +
        RATIONS_IO      +
        RATIONS_GFX     +
        RATIONS_RENDER  + 
        RATIONS_PHYSICS +
        RATIONS_ENTITY  +
        RATIONS_GAME;

    rations.bank = range_alloc_new(total);
    mem_clear(rations.bank.ptr, total);
    arena_t bank = arena_new(rations.bank);

    rations.entity  = arena_range_push(&bank, RATIONS_ENTITY);
    rations.io      = arena_range_push(&bank, RATIONS_IO);
    rations.gfx     = arena_range_push(&bank, RATIONS_GFX);
    rations.render  = arena_range_push(&bank, RATIONS_RENDER);
    rations.physics = arena_range_push(&bank, RATIONS_PHYSICS);
    rations.events  = arena_range_push(&bank, RATIONS_EVENTS);
    rations.game    = arena_range_push(&bank, RATIONS_GAME);
}

void rations_destroy() {
    range_destroy(&rations.bank);
}
