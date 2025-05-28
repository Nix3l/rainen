#include "memory.h"
#include <string.h>

// ARRAYS/RANGES
range_t range_new(void* data, usize size) {
    return (range_t) {
        .size = size,
        .data = data,
    };
}

range_t range_alloc(usize size) {
    void* data = mem_alloc(size);
    if(!data) PANIC("couldnt allocate range\n");

    return (range_t) {
        .size = size,
        .data = data,
    };
}

void range_destroy(range_t* range) {
    range->size = 0;
    free(range->data);
    range->data = NULL;
}

vector_t vector_new(void* data, u32 capacity, u32 element_size) {
    return (vector_t) {
        .size = 0,
        .capacity = capacity,
        .element_size = element_size,
        .data = data,
    };
}

vector_t vector_alloc(u32 capacity, u32 element_size) {
    void* data = mem_alloc(capacity * element_size);
    if(!data) PANIC("couldnt allocate range\n");

    return (vector_t) {
        .size = 0,
        .capacity = capacity,
        .element_size = element_size,
        .data = data,
    };
}

void* vector_push(vector_t* vector) {
    if(vector->size == vector->capacity) return NULL;
    vector->size ++;
    return vector->data + (vector->size - 1) * vector->element_size;;
}

void* vector_push_data(vector_t* vector, void* data) {
    if(vector->size == vector->capacity) return NULL;
    void* dest = vector->data + vector->size * vector->element_size;
    vector->size ++;
    memcpy(dest, data, vector->element_size);
    return dest;
}

void* vector_get(vector_t* vector, u32 index) {
    if(index > vector->size) return NULL;
    return vector->data + index * vector->element_size;
}

void vector_fetch(vector_t* vector, u32 index, void* dest) {
    if(index > vector->size)
        memset(dest, 0, vector->element_size);
    else
        memcpy(dest, vector->data + index * vector->element_size, vector->element_size);
}

void vector_remove(vector_t* vector, u32 index) {
    if(index > vector->size) return;

    // move all the elements in front of index back
    for(u32 i = index; i < vector->size - 1; i ++) {
        void* curr_start = vector_get(vector, i);
        void* next_start = vector_get(vector, i + 1);
        memcpy(curr_start, next_start, vector->element_size);
    }

    vector->size --;
}

void vector_clear(vector_t* vector) {
    vector->size = 0;
}

void vector_destroy(vector_t* vector) {
    vector->size = 0;
    vector->capacity = 0;
    vector->element_size = 0;
    free(vector->data);
    vector->data = NULL;
}

// ARENAS
arena_t arena_new(usize capacity, expand_type_t expand_type) {
    void* data = mem_alloc(capacity);
    if(!data) PANIC("couldnt allocate memory for arena\n");

    return (arena_t) {
        .size = 0,
        .capacity = capacity,
        .data = data,
        .type = expand_type,
    };
}

void arena_resize(arena_t* arena, usize new_capacity) {
    if(arena->type == EXPAND_TYPE_IMMUTABLE) return;

    arena->capacity = new_capacity;
    arena->data = mem_realloc(arena->data, new_capacity);

    if(!arena->data) PANIC("couldnt resize for arena\n");
}

void arena_prepare(arena_t* arena, usize bytes) {
    arena_resize(arena, arena->capacity + bytes);
}

// TODO(nix3l): should these really be separate functions?
static void grow_arena(arena_t* arena) {
    arena->capacity *= 1.5f;
    arena->data = mem_realloc(arena->data, arena->capacity);

    if(!arena->data) PANIC("couldnt expand arena\n");
}

static void shrink_arena(arena_t* arena) {
    arena->capacity *= 0.5f;
    arena->data = mem_realloc(arena->data, arena->capacity);

    if(!arena->data) PANIC("couldnt shrink arena\n");
}

void* arena_push(arena_t* arena, u32 bytes) {
    if(arena->size + bytes > arena->capacity) {
        if(arena->type == EXPAND_TYPE_AUTOEXPAND) grow_arena(arena);
        else return NULL;
    }

    void* data = arena->data + arena->size;
    arena->size += bytes;

    return data;
}

void* arena_push_to_capacity(arena_t* arena) {
    return arena_push(arena, arena->capacity - arena->size);
}

void arena_pop(arena_t* arena, u32 bytes) {
    if(arena->type == EXPAND_TYPE_AUTOEXPAND && arena->size - bytes < arena->capacity * 0.5f)
        shrink_arena(arena);

    if(arena->size <= bytes) arena->size = 0;
    else arena->size -= bytes;
}

bool arena_fits(arena_t* arena, u32 bytes) {
    return arena->capacity - arena->size >= bytes;
}

usize arena_remaining(arena_t* arena) {
    return arena->capacity - arena->size;
}

range_t arena_range(arena_t* arena, usize start, usize size) {
    return (range_t) {
        .size = size,
        .data = arena->data + start,
    };
}

range_t arena_range_full(arena_t* arena) {
    return (range_t) {
        .size = arena->size,
        .data = arena->data,
    };
}

void arena_clear(arena_t* arena) {
    arena->size = 0;
}

void arena_destroy(arena_t* arena) {
    arena->size = 0;
    arena->capacity = 0;
    mem_free(arena->data);
    arena->data = NULL;
}

// POOLS
bool handle_equals(handle_t h1, handle_t h2) {
    return h1.index == h2.index && h1.gen == h2.gen;
}

pool_t pool_new(u32 capacity, u32 element_size, expand_type_t type) {
    void* data = mem_alloc(capacity * element_size);
    pool_element_t* elements = mem_alloc(capacity * sizeof(pool_element_t));

    if(!data || !elements) PANIC("couldnt allocate memory for pool\n");

    for(u32 i = 0; i < capacity; i ++) {
        elements[i] = (pool_element_t) {
            .handle = (handle_t) { .index = i, .gen = 0 },
            .in_use = false,
        };
    }

    return (pool_t) {
        .element_size = element_size,

        .num_in_use = 0,
        .capacity = capacity,
        .first_free_element = 0,
        .first_used_element = 0,
        .last_used_element = 0,

        .type = type,

        .data = data,
        .elements = elements,
    };
}

void pool_resize(pool_t* pool, usize new_capacity) {
    if(pool->type == EXPAND_TYPE_IMMUTABLE) return;
    if(pool->capacity == new_capacity) return;

    void* data = mem_realloc(pool->data, new_capacity * pool->element_size);
    pool_element_t* elements = mem_realloc(pool->elements, new_capacity * sizeof(pool_element_t));

    if(!data || !elements) PANIC("couldnt prepare memory for pool\n");

    if(new_capacity > pool->capacity) {
        for(u32 i = pool->capacity + 1; i < new_capacity; i ++) {
            elements[i] = (pool_element_t) {
                .handle = (handle_t) { .index = i, .gen = 0 },
                .in_use = false,
            };
        }
    }

    pool->capacity = new_capacity;
    pool->data = data;
    pool->elements = elements;
}

void pool_prepare(pool_t* pool, u32 num_new_elements) {
    pool_resize(pool, pool->capacity + num_new_elements);
}

void* pool_push(pool_t* pool) {
    if(pool->num_in_use == pool->capacity) {
        // if the first free element is used,
        // then the pool is at capacity, and should be expanded if necessary
        if(pool->type != EXPAND_TYPE_AUTOEXPAND) return NULL;

        // TODO(nix3l): figure out a good number for this.
        pool_prepare(pool, 32);

        pool->first_free_element ++;
    }

    pool_element_t elem = pool->elements[pool->first_free_element];
    elem.in_use = true;
    elem.handle.gen ++;
    pool->elements[elem.handle.index] = elem;

    for(u32 i = pool->first_free_element; i < pool->capacity; i ++) {
        if(!pool->elements[i].in_use) {
            pool->first_free_element = i;
            break;
        }
    }

    if(elem.handle.index < pool->first_used_element) pool->first_used_element = elem.handle.index;
    if(elem.handle.index > pool->last_used_element) pool->last_used_element = elem.handle.index;
    pool->num_in_use ++;

    return pool->data + elem.handle.index * pool->element_size;
}

void* pool_push_at_index(pool_t* pool, u32 index) {
    if(index > pool->capacity) return NULL;
    
    pool_element_t elem = pool->elements[pool->first_free_element];
    elem.in_use = true;
    elem.handle.gen ++;
    pool->elements[elem.handle.index] = elem;

    for(u32 i = pool->first_free_element; i < pool->capacity; i ++) {
        if(!pool->elements[i].in_use) {
            pool->first_free_element = i;
            break;
        }
    }

    if(index < pool->first_used_element) pool->first_used_element = index;
    if(index > pool->last_used_element) pool->last_used_element = index;

    pool->num_in_use ++;

    return pool->data + elem.handle.index * pool->element_size;
}

void* pool_get(pool_t* pool, handle_t handle) {
    if(handle.index > pool->capacity) return NULL;

    // bit of an optimisation
    // probably doesnt speed it up all that much but hey its there
    if(handle.index < pool->first_used_element || handle.index > pool->num_in_use)
        return NULL;

    for(u32 i = pool->first_used_element; i <= pool->last_used_element; i ++) {
        pool_element_t curr_elem = pool->elements[i];
        if(!curr_elem.in_use) continue;

        if(handle_equals(handle, curr_elem.handle))
            return pool->data + i * pool->element_size;
    }

    return NULL;
}

void* pool_at_index(pool_t* pool, u32 index) {
    if(index > pool->num_in_use) return NULL;
    else return pool->data + index * pool->element_size;
}

void pool_free(pool_t* pool, handle_t handle) {
    if(handle.index > pool->capacity) return;

    pool_element_t elem = pool->elements[handle.index];
    elem.in_use = false;
    elem.handle.gen ++;
    pool->elements[elem.handle.index] = elem;

    pool->num_in_use --;

    // NOTE(nix3l): while these look scary for performance,
    // if you think about it, these loops will probably never iterate over 2-3 times in the WORST cases
    // since if the pools are working as they should, the used elements should be more closely packed
    // and not sparse

    if(pool->num_in_use == 0) {
        pool->first_used_element = 0;
        pool->last_used_element = 0;
        pool->first_free_element = 0;
    } else {
        // check if the first free element changed
        if(handle.index < pool->first_free_element) pool->first_free_element = handle.index;

        // if we freed the first used element, find the next used element
        if(handle.index == pool->first_used_element) {
            for(u32 i = handle.index; i <= pool->last_used_element; i ++) {
                if(pool->elements[i].in_use) {
                    pool->first_used_element = i;
                    break;
                }
            }
        // if we freed the last used element, find the previous used element
        } else if(handle.index == pool->last_used_element) {
            for(u32 i = handle.index; i >= pool->first_used_element; i --) {
                if(pool->elements[i].in_use) {
                    pool->last_used_element = i;
                    break;
                }
            }
        }
    }

    // // TODO(nix3l): finish
    /*
    if(pool->type == EXPAND_TYPE_AUTOEXPAND && pool->last_used_element < pool->capacity - 32) {
        
    }
    */
}

void pool_free_at_index(pool_t* pool, u32 index) {
    pool_free(pool, (handle_t) { .index = index });
}

void pool_clear(pool_t* pool) {
    for(u32 i = 0; i < pool->capacity; i ++) {
        pool_element_t elem = pool->elements[i];
        elem.in_use = false;
        elem.handle.gen = 0;
        pool->elements[elem.handle.index] = elem;
    }

    pool->first_free_element = 0;
    pool->first_used_element = 0;
    pool->num_in_use = 0;
}

void pool_destroy(pool_t* pool) {
    pool->num_in_use = 0;
    pool->element_size = 0;
    pool->capacity = 0;
    pool->first_free_element = 0;
    pool->first_used_element = 0;

    mem_free(pool->data);
    pool->data = NULL;
    mem_free(pool->elements);
    pool->elements = NULL;
}
