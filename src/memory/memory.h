#ifndef _MEMORY_H
#define _MEMORY_H

#include "base.h"

// in case we want to change these later
#define mem_alloc(_bytes) malloc((_bytes))
#define mem_calloc(_bytes) calloc(1, (_bytes))
#define mem_realloc(_buffer, _bytes) realloc((_buffer), (_bytes))
#define mem_free(_ptr) free((_ptr))

typedef enum expand_type_t {
    EXPAND_TYPE_IMMUTABLE = 0,
    EXPAND_TYPE_EXPANDABLE,
    EXPAND_TYPE_AUTOEXPAND,
} expand_type_t;

// RANGES/VECTORS
// a block of memory with a size
typedef struct range_t {
    usize size; // in bytes
    void* data;
} range_t;

range_t range_new(void* data, usize bytes);
range_t range_alloc(usize size);

// frees the range
void range_destroy(range_t* range);

// a range that you can dynamically add and remove elements from
// currently, no expanding/shrinking
typedef struct vector_t {
    u32 size; // in elements
    u32 capacity; // in elements
    u32 element_size;
    void* data;
} vector_t;

vector_t vector_new(void* data, u32 capacity, u32 element_size);
vector_t vector_alloc(u32 capacity, u32 element_size);

void* vector_push(vector_t* vector);
// add the element to the end of the vector
// copies the data put into it, does NOT store a pointer to the original data
// returns a pointer to the copied data within the vector
void* vector_push_data(vector_t* vector, void* data);
// return a pointer to the data located in the vector
void* vector_get(vector_t* vector, u32 index);
// copies the data at the index in the vector to the destination provided
// destination must have the same size (in bytes) as the element_size of the vector
void vector_fetch(vector_t* vector, u32 index, void* dest);
// removes the element and moves all the proceeding elements back 
void vector_remove(vector_t* vector, u32 index);

// removes all the vectors elements
void vector_clear(vector_t* vector);
// frees the data in the vector
void vector_destroy(vector_t* vector);

// ARENAS
typedef struct arena_t {
    usize size;
    usize capacity;
    expand_type_t type;
    void* data;
} arena_t;

// TODO(nix3l): split into arena_new and arena_new_alloc
arena_t arena_new(usize capacity, expand_type_t expand_type);

// only works on non-immutable arenas
void arena_resize(arena_t* arena, usize new_capacity);
// only works on non-immutable arenas
void arena_prepare(arena_t* arena, usize bytes);

void* arena_push(arena_t* arena, u32 bytes);
void* arena_push_to_capacity(arena_t* arena);

void arena_pop(arena_t* arena, u32 bytes);

bool arena_fits(arena_t* arena, u32 bytes);
usize arena_remaining(arena_t* arena);

range_t arena_range(arena_t* arena, usize start, usize size);
range_t arena_range_full(arena_t* arena);

// resets the arena head to zero
void arena_clear(arena_t* arena);
// frees the arena
void arena_destroy(arena_t* arena);

// POOLS
typedef struct handle_t {
    u32 index;
    u8 gen; // generation
} handle_t;

bool handle_equals(handle_t h1, handle_t h2);

typedef struct pool_element_t {
    handle_t handle;
    bool in_use;
} pool_element_t;

// i still feel like the name "compact list" is more fitting
// but i guess people call this a pool so who cares
typedef struct pool_t {
    u32 element_size;

    u32 num_in_use;
    u32 capacity; // in number of elements
    u32 first_free_element;
    u32 first_used_element;

    // NOTE(nix3l): make sure to use <= in loop conditions,
    // as this points to the a used element
    u32 last_used_element;

    expand_type_t type;

    void* data;
    pool_element_t* elements;
} pool_t;

pool_t pool_new(u32 capacity, u32 element_size, expand_type_t expand_type);
pool_t pool_new_unexpandable(u32 capacity, u32 element_size);

void pool_resize(pool_t* pool, usize new_capacity);
void pool_prepare(pool_t* pool, u32 num_new_elements);

// returns the memory slot at the first free element in the pool
void* pool_push(pool_t* pool);
// forcefully pushes to element at the index
// removes any data that was there before pushing
// will not expand the arena even if it is auto-expandable
void* pool_push_at_index(pool_t* pool, u32 index);

void* pool_get(pool_t* pool, handle_t handle);
void* pool_at_index(pool_t* pool, u32 index);

void pool_free(pool_t* pool, handle_t handle);
void pool_free_at_index(pool_t* pool, u32 index);

// resets all the elements to 0 (including generations)
void pool_clear(pool_t* pool);
void pool_destroy(pool_t* pool);

// TODO(nix3l): linked lists

#endif /* ifndef _MEMORY_H */
