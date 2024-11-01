#include "vector.h"
#include "log.h"
#include "memory/memory.h"
#include "base.h"

/* 
 * implementation of c++/rust like vectors in c
 * using a double void pointer as an array that can vary in size as fits
 * each element is a void pointer to a variable in heap memory (or some arena)
 *
 *          size => length of the vector
 *      capacity => maximum available memory of the vector
 *  
 */

#define VECTOR_INIT_CAPACITY 2
// according to facebook 1.5 is the optimum number
#define GROWTH_FACTOR (1.5f)

vector_s vector_create() {
    vector_s vec;
    vec.size = 0;
    vec.capacity = VECTOR_INIT_CAPACITY;
    usize bytes = sizeof(void*) * vec.capacity;
    vec.data = MEM_ZERO(mem_alloc(bytes), bytes);
    ASSERT(vec.data);

    return vec;
}

static void vector_resize(vector_s* vec, usize new_capacity) {
    if(new_capacity == 0) {
        LOG_ERR("cant resize vector to a capacity of zero\n");
        return;
    }

    void** new_data = mem_realloc(vec->data, sizeof(void*) * new_capacity);

    vec->capacity = new_capacity;
    vec->data = new_data;
}

void* vector_get(vector_s* vec, usize index) {
    ASSERT(vec);

    if(index >= vec->size) {
        LOG_ERR("index out of bounds!\n");
        return NULL;
    }
    
    return vec->data[index];
}

void vector_add(vector_s* vec, void* new_data) {
    ASSERT(vec);

    if((vec->size + 1) >= vec->capacity)
        vector_resize(vec, (usize)ceil(vec->capacity * GROWTH_FACTOR));

    if(new_data == NULL) LOG_WARN("adding null element to vector\n");

    vec->data[vec->size++] = new_data;
}

void vector_set(vector_s* vec, usize index, void* new_data) {
    ASSERT(vec);

    if(index >= vec->size) {
        LOG_ERR("index out of bounds\n");
        return;
    }

    // if(new_data == NULL) LOG_WARN("setting vector element to null\n");

    vec->data[index] = new_data;
}

void vector_insert(vector_s* vec, usize index, void* new_data) {
    ASSERT(vec);

    if(index >= vec->size) {
        LOG_ERR("index out of bounds!\n");
        return;
    }

    if(++vec->size >= vec->capacity)
        vector_resize(vec, (usize)ceil(vec->capacity * GROWTH_FACTOR));

    for(usize i = vec->size; i > index; i --)
        vec->data[i] = vec->data[i - 1];

    vec->data[index] = new_data;
}

void vector_remove(vector_s* vec, usize index) {
    ASSERT(vec);

    if(index >= vec->size) {
        LOG_ERR("index out of bounds!\n");
        return;
    }

    if(vec->size == 0) return;

    vec->data[index] = NULL;

    for(usize i = index; i < vec->size; i ++) {
        vec->data[i] = vec->data[i + 1];
        vec->data[i + 1] = NULL;
    }

    vec->size--;

    // better to overestimate than underestimate
    // might be dumb to round in the first place but
    // lets be real this wont impact performance
    // at any noticeable degree
    usize cap_threshold = (usize)ceil(vec->capacity / GROWTH_FACTOR);
    if(vec->size == 0) {
        vector_resize(vec, VECTOR_INIT_CAPACITY);
    } else if(vec->size < cap_threshold) {
        vector_resize(vec, cap_threshold);
    }
}

void vector_clear(vector_s* vec) {
    ASSERT(vec);

    vector_resize(vec, VECTOR_INIT_CAPACITY);
    vec->size = 0;
}

void vector_free(vector_s* vec) {
    ASSERT(vec);

    mem_free(vec->data);
    vec->data = NULL;
    vec->size = 0;
}

void vector_free_elements(vector_s* vec) {
    ASSERT(vec);

    for(usize i = 0; i < vec->size; i ++)
        mem_free(vector_get(vec, i));

    vector_clear(vec);
}
