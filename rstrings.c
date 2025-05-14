#include <rstrings.h>

#include <string.h>

bool Equals(String a, String b)
{
    if (a.length != b.length) 
    {
        return 0;
    }

    return !a.length || !memcmp(a.data, b.data, a.length);
}

String Copy(String s, Arena *arena)
{
    char *buffer = NEW(arena, char, s.length);

    if (s.length > 0) 
    {
        memcpy(buffer, s.data, s.length);
    }

    return (String){ buffer, s.length, s.capacity };
}

String Concat(String head, String tail, Arena *arena)
{
    if (!head.data || head.data + head.length != arena->begin)
    {
        head = Copy(head, arena);
    }

    head.length += Copy(tail, arena).length;
    head.capacity = head.length;
    
    return head;
}

uint64_t HashString(String s)
{
    uint64_t h = 0x100;

    for (ptrdiff_t i = 0; i < s.length; i++) 
    {
        h ^= s.data[i] & 255;
        h *= 1111111111111111111;
    }

    return h;
}
