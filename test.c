#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "allocators.h"

int
main(void)
{
    BumpAllocatorOptions opts = allocator_bump_init_options(1000, 2000, true);
    assert(opts.initial_size == 1000);
    assert(opts.minimum_bump_size = 2000);
    assert(opts.safemode == true);

    BumpAllocator* ba = bumpallocator_init(opts);

    if(ba == NULL) return 1;

    printf("allocator status: %d\n", ba->status);
    char* mystr = alloc((void*) ba, 200);
    printf("allocator status: %d\n", ba->status);

    deinit((void*)ba);

    return 0;
}