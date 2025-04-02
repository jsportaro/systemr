#ifndef __SYSTEMR_COMMON_H__
#define __SYSTEMR_COMMON_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

//  These need to be changed together.  
//  MAX_HASH_SIZE = CEIL(MAX_ARRAY_SIZE * 1.70)
#define MAX_ARRAY_SIZE 25
#define MAX_HASH_SIZE 43

#define EXECUTION_ARENA_SIZE (1<<16)

#define UNUSED(x) (void)(x)

uint32_t Hash(const char* key, int length);

#endif
