#ifndef __LIB_ALLOCATORS
#define __LIB_ALLOCATORS

#include <stdlib.h>
#include <stdbool.h>

typedef enum {
    ALLOC_SUCCESS,
    ALLOC_OUT_OF_MEMORY,
} AllocatorError;

typedef struct {
    AllocatorError error_code;
    void* result;
} AllocatorResult;

typedef AllocatorResult (*allocfn_alloc)(void* allocator, size_t bytes, void* options);
typedef AllocatorResult (*allocfn_dealloc)(void* allocator, void* options);

typedef struct {
    allocfn_alloc fn_alloc;
    allocfn_dealloc fn_dealloc;
} AllocatorInterface;

AllocatorResult
alloc(AllocatorInterface* allocator, size_t bytes, void* options)
{
    AllocatorResult r = (allocator->fn_alloc)((void*) allocator, bytes, options);
    return r;
}

AllocatorResult
dealloc(AllocatorInterface* allocator, size_t bytes, void* options)
{
    AllocatorResult r = (allocator->fn_dealloc)((void*) allocator, options);
    return r;
}

typedef struct {
    allocfn_alloc fn_alloc;
    allocfn_dealloc fn_dealloc;
    void* buffer;
    size_t buffer_size;
    size_t allocated_bytes;
} BumpAllocator;

#define DEFAULT_BUMP_ALLOCATOR_INITIAL_SIZE 2048
#define DEFAULT_BUMP_ALLOCATOR_MIN_BUMP_SIZE 2048
#define DEFAULT_BUMP_ALLOCATOR_SAFEMODE true

typedef struct {
    size_t initial_size;
    size_t minimum_bump_size;
    bool safemode;
} BumpAllocatorOptions;

BumpAllocatorOptions
allocator_bump_init_options(size_t initial_size, size_t minimum_bump_size, bool safemode)
{
    return BumpAllocatorOptions {
        initial_size == NULL ? DEFAULT_BUMP_ALLOCATOR_INITIAL_SIZE : initial_size,
        minimum_bump_size == NULL ? DEFAULT_BUMP_ALLOCATOR_MIN_BUMP_SIZE : minimum_bump_size,
        safemode == NULL ? DEFAULT_BUMP_ALLOCATOR_SAFEMODE : safemode
    };
}

static AllocatorResult
bumpallocator_alloc_unsafe(void* allocator, size_t bytes, void* options)
{
    BumpAllocator* a = (BumpAllocator*) allocator;
    void* pos = (void*) (((char*) a->buffer) + a->allocated_bytes);
    a->allocated_bytes += bytes;
    return AllocatorResult{ALLOC_SUCCESS, pos};
}

static AllocatorResult
bumpallocator_alloc_safe(void* allocator, size_t bytes, void* options)
{
    BumpAllocator* a = (BumpAllocator*) allocator;
    BumpAllocatorOptions* o = (BumpAllocatorOptions*) options;

    size_t candidate_size = a->allocated_bytes + bytes;
    if(candidate_size > a->buffer_size)
    {
        size_t new_size = (candidate_size - a->buffer_size) < o->minimum_bump_size ? a->buffer_size + o->minimum_bump_size : candidate_size;
        void* new_buffer = realloc(a->buffer, new_size);

        if(new_buffer == NULL)
        {
            return AllocatorResult{ALLOC_OUT_OF_MEMORY, NULL};
        }
        else
        {
            a->buffer = new_buffer;
            a->buffer_size = new_size;
            return bumpallocator_alloc_unsafe(allocator, bytes, options);
        }
    }
}

static AllocatorResult
bumpallocator_dealloc(void* allocator, void* options)
{
    BumpAllocator* a = (BumpAllocator*) allocator;
    free(a->buffer);
    free(allocator);
    return AllocatorResult{ALLOC_SUCCESS, NULL};
}

AllocatorResult
bumpallocator_init(BumpAllocatorOptions* o)
{
    BumpAllocator* a = (BumpAllocator*) malloc(sizeof(BumpAllocator));

    if(a == NULL)
        return AllocatorResult{ALLOC_OUT_OF_MEMORY, NULL};

    a->fn_alloc = o->safemode ? &bumpallocator_alloc_safe : &bumpallocator_alloc_unsafe;
    a->fn_dealloc = &bumpallocator_dealloc;
    a->buffer = malloc(o->initial_size);

    if(a->buffer == NULL)
        return AllocatorResult{ALLOC_OUT_OF_MEMORY, NULL};
    
    a->buffer_size = o->initial_size;
    a->allocated_bytes = 0;

    return AllocatorResult{ALLOC_SUCCESS, a};
}

#endif