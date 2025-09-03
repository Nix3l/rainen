#ifndef _PLATFORM_H
#define _PLATFORM_H

#include "base.h"
#include "memory/memory.h"

// TODO(nix3l):
//  => functions for loading files into byte buffers
//  => multithreading support? (later)

// reads a file from the filepath given
// pushes the data into the arena
// returns the number of bytes read into *out_size
DEVONLY range_t platform_load_file(arena_t* arena, const char* filename);

// time things
u64 platform_get_ticks();
f32 platform_get_milli_diff(u64 last_ticks);
f32 platform_get_milli_diff_reset(u64* last_ticks);

f32 platform_ticks_to_milli(u64 t);
f32 platform_ticks_to_sec(u64 t);

#endif /* ifndef _PLATFORM_H */
