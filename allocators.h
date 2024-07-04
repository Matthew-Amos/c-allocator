#ifndef __LIB_ALLOCATORS
#define __LIB_ALLOCATORS

#include <stdlib.h>
#include <stdbool.h>

typedef enum {
    ALLOC_STATUS_INITIALIZED,
    ALLOC_STATUS_SUCCESS,
    ALLOC_STATUS_OUT_OF_MEMORY,
} AllocatorStatus;

typedef void* (*allocfn_alloc)(void* allocator, size_t bytes);
typedef void (*allocfn_deinit)(void* allocator);

typedef struct {
    allocfn_alloc fn_alloc;
    allocfn_deinit fn_deinit;
    AllocatorStatus status;
} AllocatorInterface;

void*
alloc(void* allocator, size_t bytes)
{
    AllocatorInterface* a = (AllocatorInterface*) allocator;
    return (a->fn_alloc)(allocator, bytes);
}

void
deinit(void* allocator)
{
    AllocatorInterface* a = (AllocatorInterface*) allocator;
    (a->fn_deinit)((void*) allocator);
}

typedef struct {
    size_t initial_size;
    size_t minimum_bump_size;
    bool safemode;
} BumpAllocatorOptions;

typedef struct {
    allocfn_alloc fn_alloc;
    allocfn_deinit fn_deinit;
    AllocatorStatus status;
    void* buffer;
    size_t buffer_size;
    size_t allocated_bytes;
    BumpAllocatorOptions options;
} BumpAllocator;

BumpAllocatorOptions
allocator_bump_init_options(size_t initial_size, size_t minimum_bump_size, bool safemode)
{
    BumpAllocatorOptions o = {
        initial_size,
        minimum_bump_size,
        safemode
    };
    return o;
}

static void*
bumpallocator_alloc_unsafe(void* allocator, size_t bytes)
{
    BumpAllocator* a = (BumpAllocator*) allocator;
    void* pos = (void*) (((char*) a->buffer) + a->allocated_bytes);
    a->allocated_bytes += bytes;
    a->status = ALLOC_STATUS_SUCCESS;
    return pos;
}

static void*
bumpallocator_alloc_safe(void* allocator, size_t bytes)
{
    BumpAllocator* a = (BumpAllocator*) allocator;

    size_t candidate_size = a->allocated_bytes + bytes;
    if(candidate_size > a->buffer_size) {
        size_t x = candidate_size - a->buffer_size;
        size_t m = x % a->options.minimum_bump_size;
        size_t y = x - m;
        size_t new_size = a->buffer_size + a->options.minimum_bump_size*((y / a->options.minimum_bump_size) + (m > 0));
        void* new_buffer = realloc(a->buffer, new_size);

        if(new_buffer == NULL) {
            a->status = ALLOC_STATUS_OUT_OF_MEMORY;
            return NULL;
        }
            
        a->buffer = new_buffer;
        a->buffer_size = new_size;
    }

    return bumpallocator_alloc_unsafe(allocator, bytes);
}

static void
bumpallocator_deinit(void* allocator)
{
    BumpAllocator* a = (BumpAllocator*) allocator;
    free(a->buffer);
    free(allocator);
    return;
}

BumpAllocator*
bumpallocator_init(BumpAllocatorOptions options)
{
    BumpAllocator* a = (BumpAllocator*) malloc(sizeof(BumpAllocator));

    if(a == NULL) return NULL;

    a->options = options;
    a->fn_alloc = options.safemode ? &bumpallocator_alloc_safe : &bumpallocator_alloc_unsafe;
    a->fn_deinit = &bumpallocator_deinit;
    a->buffer = malloc(options.initial_size);

    if(a->buffer == NULL) {
        free(a);
        return NULL;
    }

    a->status = ALLOC_STATUS_INITIALIZED;
    a->buffer_size = options.initial_size;
    a->allocated_bytes = 0;

    return a;
}

#endif