#ifndef __ARENA_H_INCLUDED__
#define __ARENA_H_INCLUDED__

typedef struct arena_t {
  struct arena_t* next;

  void* buffer;
  long capacity;
  long offset;
} ArenaNode;

typedef struct {
  ArenaNode* current;
} Arena;

Arena arena_create(long capacity);
void* arena_alloc(Arena* a, long size);
void arena_destroy();

#endif  // __ARENA_H_INCLUDED__

#ifndef __ARENA_H_IMP__
#define __ARENA_H_IMP__

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct arena_t* new_arena(long capacity) {
  struct arena_t* a = (struct arena_t*)malloc(sizeof(struct arena_t));
  a->capacity = capacity;
  a->buffer = malloc(capacity);
  assert(a->buffer && "Failed to allocate memory for arena");
  memset(a->buffer, 0, capacity);

  a->offset = 0;
  a->next = NULL;
  return a;
}

Arena arena_create(long capacity) {
  Arena arena = {0};
  arena.current = new_arena(capacity);
  return arena;
}

ArenaNode arena_save(Arena* a) { return *a->current; }

void node_destroy(ArenaNode* n) { free(n->buffer); }

void arena_rewind(Arena* a, ArenaNode n) {
  while (a->current->buffer != n.buffer) {
    ArenaNode* next = a->current->next;
    node_destroy(a->current);
    a->current = next;
  }
  a->current->offset = n.offset;
}

void arena_destroy(Arena* a) {
  struct arena_t* current = a->current;
  while (current) {
    node_destroy(current);
    current = current->next;
  }
}

void* arena_alloc(Arena* a, long size) {
  assert(size < a->current->capacity &&
         "Size to allocate cant be greater than arena capacity");

  if (a->current->offset + size >= a->current->capacity) {
    struct arena_t* n = new_arena(a->current->capacity);
    n->next = a->current;
    a->current = n;
  }

  void* buffer = a->current->buffer + a->current->offset;

  a->current->offset += size;

  return buffer;
}

#endif