#include <arena.h>
#include <common.h>

#include <string.h>

/*
    Implementation notes: This is lifted almost wholesale from Chris Wellons
        blog posts at nullprogram.com - Help shift my perspective on why C is 
        generally faster (and it's not only becuase it's compiled)
*/

Arena NewArena(ptrdiff_t capacity)
{
    Arena a = {0};

    a.begin = a.original = malloc(capacity);
    a.end = a.begin ? a.begin + capacity : 0;
    
    return a;
}

void *Allocate(Arena *arena, ptrdiff_t size, ptrdiff_t align, ptrdiff_t count)
{
    ptrdiff_t padding = -(uintptr_t)arena->begin & (align - 1);
    ptrdiff_t available = arena->end - arena->begin - padding;
    if (available < 0 || count > available / size) 
    {
        abort();
    }
    void *p = arena->begin + padding;
    arena->begin += padding + count * size;
    return memset(p, 0, count * size);
}
