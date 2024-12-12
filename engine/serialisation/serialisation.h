#ifndef SERIALISATION_H
#define SERIALISATION_H

#include "base.h"

/*
 * => SCENE FILE FORMAT:
 *  -> entity_count [0]
 *  -> ent {
 *  ->     flags [0]
 *  ->     position [0, 0]
 *  ->     sprite.scale [0, 0]
 *  ->     sprite.rotation [0]
 *  ->     sprite.offset [0, 0]
 *  ->     sprite.layer [0]
 *  ->     sprite.color [0, 0, 0, 0]
 *  ->     rigidbody [0]
 *  ->     aabb [0, 0] [0, 0]
 *  -> }
 */

void DEVsave_game_state(char* filename);
void DEVload_game_state(char* filename);

#endif
