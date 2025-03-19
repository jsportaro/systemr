#ifndef __SYSTEMR_ARENA_H__
#define __SYSTEMR_ARENA_H__

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define NEW(...)          NEWX(__VA_ARGS__,NEW3,NEW2)(__VA_ARGS__)
#define NEWX(a,b,c,d,...) d
#define NEW2(a, t)        (t *)Allocate(a, sizeof(t), alignof(t), 1)
#define NEW3(a, t, n)     (t *)Allocate(a, sizeof(t), alignof(t), n)

typedef struct 
{
    char *begin;
    char *end;
} Arena;

Arena NewArena(ptrdiff_t capacity);
void *Allocate(Arena *arena, ptrdiff_t size, ptrdiff_t align, ptrdiff_t count);

#endif