# c-allocator
Allocators in C.

# arena.h

An arena/bump allocator in a single-header file implementation.

```C
// Interface:
typedef struct {
	void* buffer;
	size_t pos;
	size_t size;
	size_t bump_size;
} Arena;

// Initialize allocator with an initial memory size and default bump size, in bytes.
Arena*
arena_init(size_t initial_buffer_size, size_t bump_size);

// Add space to arena.
void
arena_add(Arena *a, size_t nbytes);

// Shrink arena buffer to a.pos.
void
arena_shrink(Arena *a);

// Deallocate arena.
void
arena_free(Arena *a);

// Create pointer to an area of buffer of size nbytes.
void*
arena_create(Arena *a, size_t nbytes);

// Write void *data to arena with size nbytes and return a pointer to first element.
void*
arena_write(Arena *a, void *data, size_t nbytes);
```

```C
// Use:
#include <stdio.h>
#include "arena.h"

int
main(void)
{
	Arena *a = arena_init(1000, 100);
	char *s = arena_write(a, "hello, world", 13);
	int *i = arena_create(a, sizeof(int));
	*i = 2000;

	printf("i = %d, s = %s\n", *i, s);
	// i = 2000, s = hello, world

	arena_free(a);
	
	return 0;
}
```

# License.

See LICENSE. This repository uses MIT and can be used for personal and commercial use.
