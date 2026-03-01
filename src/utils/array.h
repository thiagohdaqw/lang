#ifndef __ARRAY_H_INCLUDED__
#define __ARRAY_H_INCLUDED__

#include <assert.h>
#include <stdlib.h>

#define da_append(list, item)                                                  \
    do {                                                                       \
        if ((list)->count + 1 > (list)->capacity) {                            \
            if ((list)->capacity == 0)                                         \
                (list)->capacity = 256;                                        \
            else                                                               \
                (list)->capacity *= 2;                                         \
            (list)->items = realloc(                                           \
                (list)->items, (list)->capacity * sizeof(*(list)->items));     \
            assert((list)->items &&                                            \
                   "Failed to append item to list. Not enough memory");        \
        }                                                                      \
        (list)->items[(list)->count++] = item;                                 \
    } while (0);

#endif