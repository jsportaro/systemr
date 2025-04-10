#ifndef __SYSTEMR_RSTRINGS_H__
#define __SYSTEMR_RSTRINGS_H__

#include <arena.h>
#include <common.h>

#include <stddef.h>
#include <string.h>

#define S(s) (String){ (s), strlen(s) }

typedef struct 
{
    const char *data;
    ptrdiff_t length; 
} String;

bool Equals(String a, String b);
String Copy(String s, Arena *arena);
String Concat(String a, String b, Arena *arena);
uint64_t HashString(String s);

#endif
