// arena.h

#ifndef _ARENA_H
#define _ARENA_H

#include "common.h"

#ifndef ARENA_MEMORY_MALLOC
  #define ARENA_MEMORY_MALLOC malloc
#endif
#ifndef ARENA_MEMORY_CALLOC
  #define ARENA_MEMORY_CALLOC calloc
#endif
#ifndef ARENA_MEMORY_REALLOC
  #define ARENA_MEMORY_REALLOC realloc
#endif
#ifndef ARENA_MEMORY_FREE
  #define ARENA_MEMORY_FREE free
#endif

void* (*arena_memory_malloc)(size_t size)           = ARENA_MEMORY_MALLOC;
void* (*arena_memory_calloc)(size_t n, size_t size) = ARENA_MEMORY_CALLOC;
void* (*arena_memory_realloc)(void* p, size_t size) = ARENA_MEMORY_REALLOC;
void (*arena_memory_free)(void* p)                  = ARENA_MEMORY_FREE;

typedef struct Arena {
  u8* data;
  size_t index;
  size_t size;
} Arena;

Arena arena_new(const size_t size);
void* arena_alloc(Arena* arena, const size_t size);
void arena_reset(Arena* arena);
void arena_free(Arena* arena);

#endif // _ARENA_H

#ifdef ARENA_IMPLEMENTATION

Arena arena_new(const size_t size) {
  Arena arena = {
    .data = arena_memory_malloc(size),
    .index = 0,
    .size = size,
  };
  ASSERT(arena.data != NULL && "out of memory");
  return arena;
}

void* arena_alloc(Arena* arena, const size_t size) {
  ASSERT(arena != NULL);
  if (arena->index + size <= arena->size) {
    void* p = (void*)&arena->data[arena->index];
    arena->index += size;
    return p;
  }
  return NULL;
}

void arena_reset(Arena* arena) {
  arena->index = 0;
}

void arena_free(Arena* arena) {
  ASSERT(arena != NULL);
  arena_memory_free(arena->data);
  arena->index = 0;
  arena->size = 0;
}

#endif // ARENA_IMPLEMENTATION
