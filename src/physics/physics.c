#include "physics.h"
#include "base_macros.h"
#include "memory/memory.h"
#include "physics/bounds.h"
#include "rations/rations.h"
#include "util/util.h"

// RESOURCES:
//  => https://perso.liris.cnrs.fr/nicolas.pronost/UUCourses/GamePhysics/lectures/lecture%204%20Rigid%20Body%20Physics.pdf
//  => https://perso.liris.cnrs.fr/nicolas.pronost/UUCourses/GamePhysics/lectures/lecture%205%20Numerical%20Integration.pdf
//  => https://perso.liris.cnrs.fr/nicolas.pronost/UUCourses/GamePhysics/lectures/lecture%206%20Collision%20Detection.pdf
//  => https://perso.liris.cnrs.fr/nicolas.pronost/UUCourses/GamePhysics/lectures/lecture%207%20Collision%20Resolution.pdf
//  => https://perso.liris.cnrs.fr/nicolas.pronost/UUCourses/GamePhysics/lectures/lecture%208%20Soft%20Body%20Physics.pdf
//  => https://perso.liris.cnrs.fr/nicolas.pronost/UUCourses/GamePhysics/lectures/lecture%209%20Physics%20Engine.pdf
//
//  => https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/physicstutorials/1raycasting/Physics%20-%20Raycasting.pdf
//  => https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/physicstutorials/2linearmotion/Physics%20-%20Linear%20Motion.pdf
//  => https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/physicstutorials/3angularmotion/Physics%20-%20Angular%20Motion.pdf
//  => https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/physicstutorials/4collisiondetection/Physics%20-%20Collision%20Detection.pdf
//  => https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/physicstutorials/5collisionresponse/Physics%20-%20Collision%20Response.pdf
//  => https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/physicstutorials/6accelerationstructures/Physics%20-%20Spatial%20Acceleration%20Structures.pdf
//  => https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/physicstutorials/8constraintsandsolvers/Physics%20-%20Constraints%20and%20Solvers.pdf

//  => https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/previousinformation/physics1introductiontonewtoniandynamics/2017%20Tutorial%201%20-%20Introduction%20to%20Newtonian%20Dynamics.pdf
//  => https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/previousinformation/physics2numericalintegrationmethods/2017%20Tutorial%202%20-%20Numerical%20Integration%20Methods.pdf
//  => https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/previousinformation/physics3constraints/2017%20Tutorial%203%20-%20Constraints.pdf
//  => https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/previousinformation/physics4collisiondetection/2017%20Tutorial%204%20-%20Collision%20Detection.pdf
//  => https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/previousinformation/physics5collisionmanifolds/2017%20Tutorial%205%20-%20Collision%20Manifolds.pdf
//  => https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/previousinformation/physics6collisionresponse/2017%20Tutorial%206%20-%20Collision%20Response.pdf
//  => https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/previousinformation/physics7solvers/2017%20Tutorial%207%20-%20Solvers.pdf

physics_ctx_t physics_ctx = {0};

void physics_init() {
    arena_t physics_rations = arena_new(rations.physics);
    pool_t obj_pool = arena_pool_push(&physics_rations, PHYS_MAX_OBJS, sizeof(physobj_t));
    arena_t frame_arena = arena_new(arena_range_remaining(&physics_rations));

    physics_ctx = (physics_ctx_t) {
        .rations = physics_rations,
        .obj_pool = obj_pool,
        .frame_arena = frame_arena,
        .substeps = 8,
        .global_gravity = v2f_new(0.0f, -981.0f),
    };
}

void physics_terminate() {
    arena_clear(&physics_ctx.rations);
}

collider_t collider_new(collider_info_t info) {
    collider_t collider = {0};
    physobj_t* obj = pool_push(&physics_ctx.obj_pool, &collider.id);

    obj->tags = info.tags;
    obj->bounds = info.bounds;
    obj->pos = info.pos;
    obj->vel = v2f_ZERO;
    obj->acc = v2f_ZERO;
    obj->force = v2f_ZERO;
    obj->restitution = info.restitution;
    obj->friction = info.friction;

    if(info.tags & COLLIDER_TAGS_STATIC) {
        obj->mass = 0.0f;
        obj->inv_m = 0.0f;
    } else {
        obj->mass = info.mass;
        obj->inv_m = 1.0f / info.mass;
    }

    return collider;
}

void collider_destroy(collider_t collider) {
    pool_free(&physics_ctx.obj_pool, collider.id);
}

physobj_t* collider_get_data(collider_t collider) {
    return pool_get(&physics_ctx.obj_pool, collider.id);
}

void collider_apply_force(collider_t collider, v2f force) {
    physobj_t* obj = collider_get_data(collider);   
    if(obj->tags & COLLIDER_TAGS_STATIC) return;
    obj->force = v2f_add(obj->force, force);
}

static void physics_apply_gravity() {
    pool_iter_t iter = {0};
    while(pool_iter(&physics_ctx.obj_pool, &iter)) {
        physobj_t* obj = iter.data;
        if(obj->tags & COLLIDER_TAGS_STATIC) continue;
        if(!(obj->tags & COLLIDER_TAGS_NO_GRAV)) obj->force = v2f_add(obj->force, v2f_scale(physics_ctx.global_gravity, obj->mass));
    }
}

static void physics_integrate_force(physobj_t* obj, f32 dt) {
    // velocity-verlet integration
    v2f new_acc = v2f_scale(obj->force, obj->inv_m);
    v2f new_vel = v2f_add(obj->vel, v2f_scale(v2f_add(obj->acc, new_acc), 0.5f * dt));

    obj->vel = new_vel;
    obj->acc = new_acc;
    obj->force = v2f_ZERO;
}

static void physics_integrate_position(physobj_t* obj, f32 dt) {
    obj->pos = v2f_add(v2f_add(obj->pos, v2f_scale(obj->vel, dt)), v2f_scale(obj->acc, 0.5f * dt * dt));
}

static void physics_integrate_forces(f32 dt) {
    pool_iter_t iter = {0};
    while(pool_iter(&physics_ctx.obj_pool, &iter)) {
        physobj_t* obj = iter.data;
        if(obj->tags & COLLIDER_TAGS_STATIC) continue;
        physics_integrate_force(obj, dt);
    }
}

static void physics_integrate_positions(f32 dt) {
    pool_iter_t iter = {0};
    while(pool_iter(&physics_ctx.obj_pool, &iter)) {
        physobj_t* obj = iter.data;
        if(obj->tags & COLLIDER_TAGS_STATIC) continue;
        physics_integrate_position(obj, dt);
    }
}

static intersection_t physics_collide_objs(physobj_t* obj1, physobj_t* obj2) {
    aabb_t b1 = aabb_translate(obj1->bounds.box, obj1->pos);
    aabb_t b2 = aabb_translate(obj2->bounds.box, obj2->pos);
    return aabb_aabb_intersect(b1, b2);
}

static void physics_collisions_compute_manifolds() {
    physics_ctx.narrow.pairs = arena_vector_push(&physics_ctx.frame_arena, 128, sizeof(manifold_t));
    pool_iter_t major_iter = {0};
    while(pool_iter(&physics_ctx.obj_pool, &major_iter)) {
        physobj_t* obj1 = major_iter.data;
        if(obj1->tags & COLLIDER_TAGS_STATIC) continue;
        pool_iter_t minor_iter = {0};
        while(pool_iter(&physics_ctx.obj_pool, &minor_iter)) {
            if(major_iter.handle == minor_iter.handle) continue;
            physobj_t* obj2 = minor_iter.data;
            intersection_t inter = physics_collide_objs(obj1, obj2);
            if(inter.inersect) {
                manifold_t* pair = vector_push(&physics_ctx.narrow.pairs);
                pair->coll1 = (collider_t) { major_iter.handle };
                pair->coll2 = (collider_t) { minor_iter.handle };
                pair->obj1 = obj1;
                pair->obj2 = obj2;
                pair->inter = inter;
            }
        }
    }
}

static void physics_manifold_resolve(manifold_t* pair) {
    // position correction
    const f32 correction_amount = 0.7f;
    f32 r1 = pair->obj1->inv_m / (pair->obj1->inv_m + pair->obj2->inv_m);
    f32 r2 = pair->obj2->inv_m / (pair->obj1->inv_m + pair->obj2->inv_m);
    v2f dx1 = v2f_scale(pair->inter.normal, -correction_amount * r1 * pair->inter.penetration);
    v2f dx2 = v2f_scale(pair->inter.normal,  correction_amount * r2 * pair->inter.penetration);
    pair->obj1->pos = v2f_add(pair->obj1->pos, dx1);
    pair->obj2->pos = v2f_add(pair->obj2->pos, dx2);

    // impulse resolution
    v2f vr = v2f_sub(pair->obj1->vel, pair->obj2->vel);
    v2f n = pair->inter.normal;
    f32 ndvr = v2f_dot(n, vr);
    // weird stuff happens when restitution is 1.0
    // like objects keep accumulating velocity if they are resting on one another,
    // so they kind of "snap" when they are no longer colliding
    // but for some reason keeping it at 0.999 makes it fine so
    // TODO(nix3l): figure out a better fix (or why that is happening in the first place)
    f32 e = CLAMP_MAX(MIN(pair->obj1->restitution, pair->obj2->restitution), 0.999f);
    f32 vj = -(1.0f + e) * ndvr;
    f32 J = vj / (pair->obj1->inv_m + pair->obj2->inv_m);

    // friction
    v2f fr = v2f_ZERO;
    v2f t = v2f_scale(v2f_new(n.y, -n.x), SIGN(ndvr));
    f32 mu = MAX(pair->obj1->friction, pair->obj2->friction);
    f32 tdvr = v2f_dot(t, vr);
    f32 fr_scale = CLAMP(mu * tdvr / (pair->obj1->inv_m + pair->obj2->inv_m), -mu, mu);
    fr = v2f_scale(t, fr_scale);

    pair->obj1->vel = v2f_add(pair->obj1->vel, v2f_scale(v2f_add(n, fr), J * pair->obj1->inv_m));
    pair->obj2->vel = v2f_sub(pair->obj2->vel, v2f_scale(v2f_add(n, fr), J * pair->obj2->inv_m));
}

static void physics_collisions_resolve() {
    for(u32 i = 0; i < physics_ctx.narrow.pairs.size; i ++) {
        manifold_t* pair = vector_get(&physics_ctx.narrow.pairs, i);
        physics_manifold_resolve(pair);
    }
}

void physics_update(f32 dt) {
    physics_apply_gravity();

    dt /= physics_ctx.substeps;
    for(u32 i = 0; i < physics_ctx.substeps; i ++) {
        physics_integrate_positions(dt);
        physics_integrate_forces(dt);
        physics_collisions_compute_manifolds();
        physics_collisions_resolve();
        arena_clear(&physics_ctx.frame_arena);
    }
}
