#ifndef __ARENA_H_INCLUDED__
#define __ARENA_H_INCLUDED__

typedef struct arena_t {
    struct arena_t *next;

    void *buffer;
    long capacity;
    long offset;
} ArenaNode;

typedef struct {
    ArenaNode *current;
} Arena;

Arena arena_create(long capacity);
void *arena_alloc(Arena *a, long size);
char *arena_strdup(Arena *a, const char *str);
void arena_destroy(Arena *a);
void *arena_copy(Arena *a, const void *item, long size);
ArenaNode arena_save(Arena *a);
void arena_rewind(Arena *a, ArenaNode n);
char *arena_strjoin(Arena *a, const char *str1, const char *str2);

#endif // __ARENA_H_INCLUDED__

#ifndef __ARENA_H_IMP__
#define __ARENA_H_IMP__

#include <assert.h>
#include <stdlib.h>
#include <string.h>

struct arena_t *new_arena(long capacity) {
    struct arena_t *a = (struct arena_t *)malloc(sizeof(struct arena_t));
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

ArenaNode arena_save(Arena *a) { return *a->current; }

void node_destroy(ArenaNode *n) { free(n->buffer); }

void arena_rewind(Arena *a, ArenaNode n) {
    while (a->current->buffer != n.buffer) {
        ArenaNode *next = a->current->next;
        node_destroy(a->current);
        a->current = next;
    }
    a->current->offset = n.offset;
}

void arena_destroy(Arena *a) {
    struct arena_t *current = a->current;
    while (current) {
        node_destroy(current);
        current = current->next;
    }
}

void *arena_alloc(Arena *a, long size) {
    assert(size < a->current->capacity && "Size to allocate cant be greater than arena capacity");

    if (a->current->offset + size > a->current->capacity) {
        struct arena_t *n = new_arena(a->current->capacity);
        n->next = a->current;
        a->current = n;
    }

    void *buffer = a->current->buffer + a->current->offset;

    a->current->offset += size;

    memset(buffer, 0, size);

    return buffer;
}

void *arena_copy(Arena *a, const void *item, long size) {
    void *buffer = arena_alloc(a, size);
    memcpy(buffer, item, size);
    return buffer;
}

char *arena_strdup(Arena *a, const char *str) {
    int strSize = strlen(str);
    char *out = (char *)arena_copy(a, str, strSize + 1);
    out[strSize] = '\0';
    return out;
}

char *arena_strjoin(Arena *a, const char *str1, const char *str2) {
    int strLen1 = strlen(str1);
    int strLen2 = strlen(str2);
    char *buffer = (char *)arena_alloc(a, strLen1 + strLen2 + 1);
    memcpy(buffer, str1, strLen1);
    memcpy(buffer + strLen1, str2, strLen2);
    buffer[strLen1+strLen2] = '\0';
    return buffer;
}

#endif