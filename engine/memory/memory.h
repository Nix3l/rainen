#ifndef MEMORY_MANAGEMENT_H
#define MEMORY_MANAGEMENT_H

#include <stdlib.h>
#include "base.h"

// NOTE(nix3l): for now, use the std provided heap allocation functions,
// in case we want to replace with our own allocation interface
// at some point for whatever reason
#define mem_alloc(_bytes) malloc((_bytes))
#define mem_calloc(_bytes) calloc(1, (_bytes))
#define mem_realloc(_buffer, _bytes) realloc((_buffer), (_bytes))
#define mem_free(_ptr) free((_ptr))

typedef struct {
    usize capacity; // amount of data reserved
    usize size; // amount of data committed, serves as the memory position

    void* data;

    bool expandable;
} arena_s;

// create an arena with an immutable fixed size
arena_s arena_create(usize capacity);
// create an arena that expands as its contents do
arena_s arena_create_expandable(usize capacity);

// create a subarena with an immutable fixed size
arena_s subarena_create(arena_s* arena, usize capacity);

// create an arena with an immutable fixed size within a given memory chunk
arena_s arena_create_in_block(void* data, usize capacity);
// create an arena that expands as its contents do within a given memory chunk
arena_s arena_create_expandable_in_block(void* data, usize capacity);

void* arena_push(arena_s* arena, usize bytes);
void arena_pop(arena_s* arena, usize bytes);

// pushes a pointer with as much memory as the arena has left
void* arena_push_to_capacity(arena_s* arena);

// returns whether the given number of bytes can fit in the arena or not
bool arena_fits(arena_s* arena, usize bytes);

// returns the maximum amount of memory that can be pushed on to the arena
usize arena_remaining_capacity(arena_s* arena);

// clear all the used memory in the arena. does not destroy the arena itself
void arena_clear(arena_s* arena);
// frees and destroys the arena. arena can not be used afterwards
void arena_free(arena_s* arena);

typedef enum {
    COMPACT_LIST_EMPTY = 0,
    COMPACT_LIST_TAKEN,
} compact_list_element_t;

// TODO(nix3l): doesnt really handle filling up

typedef struct {
    arena_s* arena; // MUST be zero'd

    u32 element_size; // size of each stored element
    u32 capacity; // max amount of stored elements

    u32 count; // amount of stored elements
               // does NOT equal the index of the last element
    u32 first_free_index; // index of first free element
    // TODO(nix3l)
    u32 last_used_index; // index of first free element

    void* contents; // data
    compact_list_element_t* elements; // empty slot or taken slot
} compact_list_s;

// returns a compact list with the specified element size and capacity
// where capacity is the maximum number of elements
compact_list_s create_compact_list(arena_s* arena, u32 element_size, u32 capacity);

// returns the memory in the first free memory slot in the list
void* compact_list_push(compact_list_s* list, u32* index);
// returns the memory of the slot at the given index
// returns NULL if the slot is empty at index
void* compact_list_get(compact_list_s* list, u32 index);
// sets the slot at index to empty
void compact_list_remove(compact_list_s* list, u32 index);

// TODO(nix3l): this is quite horrible and needs tons of work
//              no way of handling removing elements
//              doesnt really handle filling up

typedef struct {
    void* contents;
    void* next;
} linked_list_element_s;

typedef struct {
    arena_s* arena; // doesnt HAVE to be zero'd but preferably should be

    u32 count; // amount of stored elements

    linked_list_element_s* first;
    linked_list_element_s* last;
} linked_list_s;

// create a linked list that occupies the given arena
linked_list_s create_linked_list(arena_s* arena);

// puts the data at the head of the linked list
void linked_list_push(linked_list_s* list, void* data);
// inserts the data at the given index of the linked list
// all succeeding elements are pushed down one slot
void linked_list_insert(linked_list_s* list, void* data, u32 index);

// removes all elements from a linked list
// does not free any data
void linked_list_clear(linked_list_s* list);

#endif
