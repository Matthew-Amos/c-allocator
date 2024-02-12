#ifndef _ARENA_H
#define _ARENA_H
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct {
	void* buffer;
	size_t pos;
	size_t size;
	size_t bump_size;
} Arena;

Arena*
arena_init(size_t initial_buffer_size, size_t bump_size)
{
	Arena *a = (Arena*) malloc(sizeof(Arena));
	void *tmp = malloc(initial_buffer_size);
	assert(tmp != NULL);
	a->buffer = tmp;
	a->pos = 0;
	a->size = initial_buffer_size;
	a->bump_size = bump_size;
	return a;
}

void
arena_add(Arena *a, size_t nbytes)
{
	void *tmp = realloc(a->buffer, a->size + nbytes);
	assert(tmp != NULL);
	a->size += nbytes;
	a->buffer = tmp;
}

static void
arena_check(Arena *a, size_t nwritebytes)
{
	if(a->pos + nwritebytes > a->size) {
		arena_add(a, a->bump_size > nwritebytes ? a->bump_size : nwritebytes);
	}	
}

void
arena_shrink(Arena *a)
{
	void *tmp = realloc(a->buffer, a->pos + 1);
	assert(tmp != NULL);
	a->buffer = tmp;
	a->size = a->pos + 1;
}

void
arena_free(Arena *a)
{
	free(a->buffer);
	free(a);
}

void*
arena_create(Arena *a, size_t nbytes)
{
	arena_check(a, nbytes);
	void *p = &(((char*) a->buffer)[a->pos]);
	a->pos += nbytes;
	return p;
}

void*
arena_write(Arena *a, void *data, size_t nbytes)
{
	arena_check(a, nbytes);
	void *tmp = memcpy(&(((char*) a->buffer)[a->pos]), data, nbytes);
	assert(tmp != NULL);
	void *p = &(((char*) a->buffer)[a->pos]);
	a->pos += nbytes;
	return p;
}
#endif