#ifndef _BOUNDS_H
#define _BOUNDS_H

#include "base.h"

typedef struct aabb_t {
    v2f min;
    v2f max;
} aabb_t;

aabb_t aabb_new(v2f min, v2f max);
aabb_t aabb_new_rect(v2f center, v2f half_extents);

aabb_t aabb_translate(aabb_t box, v2f offset);

v2f aabb_center(aabb_t box);
v2f aabb_size(aabb_t box);
v2f aabb_half_extents(aabb_t box);

aabb_t aabb_minkowski_diff(aabb_t box1, aabb_t box2);

bool aabb_point_check(aabb_t box, v2f point);
bool aabb_aabb_check(aabb_t box1, aabb_t box2);

typedef struct intersection_t {
    bool inersect;
    f32 penetration;
    v2f dir;
} intersection_t;

// NOTE(nix3l): these can be optimised a bit further, but no need right now

intersection_t aabb_point_intersect(aabb_t box, v2f point);
intersection_t aabb_aabb_intersect(aabb_t box1, aabb_t box2);

#endif
