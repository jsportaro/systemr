#ifndef __SYSTEMR_SLICE_H__
#define __SYSTEMR_SLICE_H__

#include <common.h>
#include <arena.h>

#define Push(s, a) ({                                        \
    typeof(s) s_ = (s);                                      \
    typeof(a) a_ = (a);                                      \
    if (s_->length >= s_->capacity) {                        \
        Grow(s_, sizeof(*s_->data), alignof(*s_->data), a_); \
    }                                                        \
    s_->data + s_->len++;                                    \
})

void Grow(void *slice, ptrdiff_t size, ptrdiff_t align, Arena *arena);

#endif
