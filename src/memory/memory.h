#ifndef _MEMORY_H
#define _MEMORY_H

#include "base.h"

// in case we want to change these later
#define mem_alloc(_bytes) malloc((_bytes))
#define mem_calloc(_bytes) calloc(1, (_bytes))
#define mem_realloc(_buffer, _bytes) realloc((_buffer), (_bytes))
#define mem_free(_ptr) free((_ptr))

void mem_clear(void* ptr, usize size);

typedef enum expand_type_t {
    EXPAND_TYPE_IMMUTABLE = 0,
    EXPAND_TYPE_EXPANDABLE,
    EXPAND_TYPE_AUTOEXPAND,
} expand_type_t;

// RANGES/VECTORS
// a block of memory with a size
typedef struct range_t {
    usize size; // in bytes
    void* ptr;
} range_t;

#define RANGE_EMPTY ((range_t) { .size = 0, .ptr = NULL })

range_t range_new(void* data, usize bytes);
range_t range_alloc_new(usize bytes);

// frees the range
void range_destroy(range_t* range);

// a copy-on-write range that you can dynamically add and remove elements from
// currently, no expanding/shrinking
typedef struct vector_t {
    u32 size; // in elements
    u32 capacity; // in elements
    u32 element_size;
    void* data;
} vector_t;

vector_t vector_new(void* data, u32 capacity, u32 element_size);
vector_t vector_alloc_new(u32 capacity, u32 element_size);

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

// POOLS
typedef u32 handle_t;

handle_t handle_new(u32 index, u32 generation);

u32 handle_index(handle_t handle);
u32 handle_gen(handle_t handle);

handle_t handle_set_index(handle_t handle, u32 index);
handle_t handle_set_gen(handle_t handle, u32 gen);

handle_t handle_inc_index(handle_t handle);
handle_t handle_inc_gen(handle_t handle);

// TODO(nix3l): what the fuck am i doing here?
typedef enum pool_element_state_t {
    POOL_ELEMENT_FREE = 0,
    POOL_ELEMENT_IN_USE,
    POOL_ELEMENT_RESERVED,
} pool_element_state_t;

typedef struct pool_element_t {
    handle_t handle;
    pool_element_state_t state;
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
    // as this points to a used element
    u32 last_used_element;
    expand_type_t type;

    void* data;
    pool_element_t* elements;
} pool_t;

// immutable pool creation
pool_t pool_new(void* block, u32 capacity, u32 element_size);
pool_t pool_alloc_new(u32 capacity, u32 element_size);

// expandable pool creation
pool_t pool_new_expand(void* block, u32 capacity, u32 element_size, expand_type_t expand_type);
pool_t pool_alloc_new_expand(u32 capacity, u32 element_size, expand_type_t expand_type);

void pool_resize(pool_t* pool, u32 new_capacity);
void pool_prepare(pool_t* pool, u32 num_new_elements);

// returns the memory slot at the first free element in the pool
void* pool_push(pool_t* pool, handle_t* out_handle);
// forcefully sets element at index
// removes any data that was there before pushing
// will not expand the arena even if it is auto-expandable
void* pool_set(pool_t* pool, u32 index, handle_t* out_handle);

void* pool_get(pool_t* pool, handle_t handle);
void* pool_at_index(pool_t* pool, u32 index);

void pool_free(pool_t* pool, handle_t handle);

// resets all the elements to 0 (including generations)
void pool_clear(pool_t* pool);
void pool_destroy(pool_t* pool);

typedef struct pool_iter_t {
    void* data;
    u32 iteration;
    u32 absolute_index;
    handle_t handle;
} pool_iter_t;

bool pool_iter(pool_t* pool, pool_iter_t* iter);

// ARENAS
typedef struct arena_t {
    usize size;
    usize capacity;
    expand_type_t type;
    void* data;
} arena_t;

// TODO(nix3l): subarena_new

// immutable arena creation
arena_t arena_new(range_t block);
arena_t arena_alloc_new(usize capacity);

// expandable array creation
arena_t arena_new_expand(range_t block, expand_type_t expand_type);
arena_t arena_alloc_new_expand(usize capacity, expand_type_t expand_type);

// only works on non-immutable arenas
void arena_resize(arena_t* arena, usize new_capacity);
// only works on non-immutable arenas
void arena_prepare(arena_t* arena, usize bytes);

void* arena_push(arena_t* arena, u32 bytes);
void* arena_push_to_capacity(arena_t* arena);

void arena_pop(arena_t* arena, u32 bytes);

bool arena_fits(arena_t* arena, u32 bytes);
usize arena_remaining(arena_t* arena);

// returns a range within the arena
range_t arena_range(arena_t* arena, usize start, usize size);
// returns a range with the unallocated memory in the arena
range_t arena_range_remaining(arena_t* arena);
// returns a range containing the entire arena
range_t arena_range_full(arena_t* arena);
// allocates a range in an arena
range_t arena_range_push(arena_t* arena, u32 bytes);

// allocates an immutable vector in the arena
vector_t arena_vector_push(arena_t* arena, u32 num_elements, u32 element_size);

// allocates an immutable pool in the arena
pool_t arena_pool_push(arena_t* arena, u32 capacity, u32 element_size);

// resets the arena head to zero
void arena_clear(arena_t* arena);
// frees the arena
void arena_destroy(arena_t* arena);

typedef struct llist_node_t {
    void* data;
    struct llist_node_t* next;
    struct llist_node_t* prev;
} llist_node_t;

typedef struct llist_t {
    u32 size;
    llist_node_t* start;
    llist_node_t* end;
} llist_t;

llist_t llist_new();

llist_node_t* llist_push(llist_t* list, arena_t* arena, void* data);

// does not deallocate data
void llist_remove(llist_t* list, llist_node_t* node);
// does not deallocate data
void llist_clear(llist_t* list);

typedef struct llist_iter_t {
    u32 index;
    void* data;
    llist_node_t* node;
} llist_iter_t;

bool llist_iter(llist_t* list, llist_iter_t* iter);

#endif
