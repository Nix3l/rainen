#include "bounds.h"

aabb_t aabb_new(v2f min, v2f max) {
    return (aabb_t) {
        .min = min,
        .max = max,
    };
}

aabb_t aabb_new_rect(v2f center, v2f half_extents) {
    return (aabb_t) {
        .min = v2f_sub(center, half_extents),
        .max = v2f_add(center, half_extents),
    };
};

aabb_t aabb_translate(aabb_t box, v2f offset) {
    return (aabb_t) {
        .min = v2f_add(box.min, offset),
        .max = v2f_add(box.max, offset),
    };
}

v2f aabb_center(aabb_t box) {
    return v2f_scale(v2f_add(box.max, box.min), 0.5f);
}

v2f aabb_size(aabb_t box) {
    return v2f_sub(box.max, box.min);
}

v2f aabb_half_extents(aabb_t box) {
    return v2f_scale(v2f_sub(box.max, box.min), 0.5f);
}

// TODO(nix3l): redo this
aabb_t aabb_minkowski_diff(aabb_t box1, aabb_t box2) {
    v2f center1 = aabb_center(box1);
    v2f center2 = aabb_center(box2);
    v2f size1 = aabb_half_extents(box1);
    v2f size2 = aabb_half_extents(box2);

    v2f center = v2f_sub(center1, center2);
    v2f he = v2f_add(size1, size2);

    return (aabb_t) {
        .min = v2f_sub(center, he),
        .max = v2f_add(center, he),
    };
}

bool aabb_point_intersect(aabb_t box, v2f point) {
    return
        (box.min.x < point.x && box.max.x > point.x) &&
        (box.min.y < point.y && box.max.y > point.y);
}

intersection_t aabb_aabb_intersect(aabb_t box1, aabb_t box2) {
    intersection_t res = {0};

    aabb_t minkowski = aabb_minkowski_diff(box1, box2);
    if(!aabb_point_intersect(minkowski, v2f_ZERO)) return res;

    f32 min_dist = fabsf(minkowski.min.x);
    res.dir = v2f_new(-1.0f, 0.0f);

    if(fabsf(minkowski.max.x) < min_dist) {
        min_dist = fabsf(minkowski.max.x);
        res.dir = v2f_new(1.0f, 0.0f);
    }

    if(fabsf(minkowski.min.y) < min_dist) {
        min_dist = fabsf(minkowski.min.y);
        res.dir = v2f_new(0.0f, -1.0f);
    }

    if(fabsf(minkowski.max.y) < min_dist) {
        min_dist = fabsf(minkowski.max.y);
        res.dir = v2f_new(0.0f, 1.0f);
    }

    res.inersect = true;
    res.penetration = min_dist;
    return res;
}
