#ifndef __SYSTEMR_SLICE_H__
#define __SYSTEMR_SLICE_H__

#include <common.h>
#include <arena.h>

#define Push(s, arena) \
    ((s)->length >= (s)->capacity \
        ?  Grow(s, sizeof(*(s)->data), alignof(*(s)->data), arena), \
          (s)->data + (s)->length++ \
        : (s)->data + (s)->length++)

void Grow(void *slice, ptrdiff_t size, ptrdiff_t align, Arena *arena);

#endif
