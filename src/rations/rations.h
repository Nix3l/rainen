#ifndef _RATIONS_H
#define _RATIONS_H

#include "base.h"
#include "memory/memory.h"

enum {
    RATIONS_EVENTS  = KILOBYTES(4),
    RATIONS_IO      = KILOBYTES(4),
    RATIONS_GFX     = MEGABYTES(1),
    RATIONS_RENDER  = MEGABYTES(1),
    RATIONS_PHYSICS = MEGABYTES(1),
    RATIONS_ENTITY  = MEGABYTES(1),
    RATIONS_GAME    = KILOBYTES(1),
};

typedef struct rations_t {
    range_t bank;

    range_t io;
    range_t gfx;
    range_t render;
    range_t physics;
    range_t entity;
    range_t events;
    range_t game;
} rations_t;

extern rations_t rations;

void rations_divide();
void rations_destroy();

#endif
