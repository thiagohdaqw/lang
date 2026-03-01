#ifndef __ARRAY_H_INCLUDED__
#define __ARRAY_H_INCLUDED__

#include <assert.h>
#include <stdlib.h>

// TODO: ADD SUPPORT TO ARENA
#define da_append(list, item)                                                                                          \
    do {                                                                                                               \
        if ((list)->count + 1 > (list)->capacity) {                                                                    \
            if ((list)->capacity == 0)                                                                                 \
                (list)->capacity = 256;                                                                                \
            else                                                                                                       \
                (list)->capacity *= 2;                                                                                 \
            (list)->items = realloc((list)->items, (list)->capacity * sizeof(*(list)->items));                         \
            assert((list)->items && "Failed to append item to list. Not enough memory");                               \
        }                                                                                                              \
        (list)->items[(list)->count++] = item;                                                                         \
    } while (0)

#define da_destroy(list)                                                                                               \
    do {                                                                                                               \
        if ((list)->items) free((list)->items);                                                                        \
    } while (0)
#endif