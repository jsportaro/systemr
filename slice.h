#ifndef __SYSTEMR_SLICE_H__
#define __SYSTEMR_SLICE_H__

#include <common.h>
#include <arena.h>

#define PUSH(a, s) \
    ((s)->length == (s)->capacity) \
        ? (s)->data = push((a), (s)->data, &(s)->capacity, sizeof(*(s)->data)), \
          (s)->data + (s)->length++ \
        : (s)->data + (s)->length++)

void *push(Arena *a, void *data, ptrdiff_t *pcap, ptrdiff_t size);

#endif
