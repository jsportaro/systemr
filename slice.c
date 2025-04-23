#include <slice.h>
#include <string.h>

void Grow(void *slice, ptrdiff_t size, ptrdiff_t align, Arena *arena)
{
    struct {
        char     *data;
        ptrdiff_t len;
        ptrdiff_t cap;
    } replica;
    
    memcpy(&replica, slice, sizeof(replica));

    if (!replica.data)
    {
        replica.cap = 1;
        replica.data = Allocate(arena, 2*size, align, replica.cap);
    }
    else if (arena->begin == replica.data + size*replica.cap)
    {
        Allocate(arena, size, 1, replica.cap);
    }
    else
    {
        void *data = Allocate(arena, 2*size, align, replica.cap);
        memcpy(data, replica.data, size*replica.len);
        replica.data = data;
    }

    replica.cap *= 2;
    memcpy(slice, &replica, sizeof(replica));
}
