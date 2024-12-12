#include "memory.h"

compact_list_s create_compact_list(arena_s* arena, u32 element_size, u32 capacity) {
    return (compact_list_s) {
        .arena = arena,

        .element_size = element_size,
        .capacity = capacity,

        .count = 0,
        .first_free_index = 0,
        .contents = arena_push(arena, element_size * capacity),
        .elements = arena_push(arena, sizeof(compact_list_element_t) * capacity),
    };
}

void* compact_list_push(compact_list_s* list, u32* index) {
    if(list->count == list->capacity) return NULL;

    list->elements[list->first_free_index] = COMPACT_LIST_TAKEN;
    void* data = list->contents + list->element_size * list->first_free_index;
    if(index) *index = list->first_free_index;
    if(list->first_free_index > list->last_used_index) list->last_used_index = list->first_free_index;

    for(u32 i = list->first_free_index + 1; i < list->capacity; i ++) {
        if(list->elements[i] == COMPACT_LIST_EMPTY) {
            list->first_free_index = i;
            break;
        }
    }

    list->count ++;
    return data;
}

void* compact_list_get(compact_list_s* list, u32 index) {
    return list->elements[index] == COMPACT_LIST_EMPTY ? NULL : list->contents + list->element_size * index;
}

// TODO(nix3l): fix trying to remove an already free element
void compact_list_remove(compact_list_s* list, u32 index) {
    if(index > list->capacity) return;

    list->elements[index] = COMPACT_LIST_EMPTY;

    if(list->first_free_index > index)
        list->first_free_index = index;

    if(index == list->last_used_index) {
        for(u32 i = index; i >= 0; i --) {
            if(list->elements[i] == COMPACT_LIST_TAKEN) {
                list->last_used_index = i;
                break;
            }
        }
    }

    list->count --;
}
