#pragma once

#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __GNUC__
#define UNREACHABLE __builtin_unreachable()
#else
#define UNREACHABLE
#endif

#define ALLOCATE(memory, amt, Type) memory = (Type *)malloc(sizeof(Type) * amt)
#define REALLOCATE(memory, amt, Type) memory = (Type *)realloc(memory, sizeof(Type) * amt)
#define COPY(src, dst, amt) memcpy(dst, src, amt * sizeof(src[0]))

int mod(int a, int b);

int max2(int a, int b);
int max3(int a, int b, int c);

// Source: https://stackoverflow.com/questions/26237419/faster-than-rand
//         Thanks Asis :)
static unsigned int g_seed;

inline void fast_srand(int seed)
{
    g_seed = seed;
}

inline int fast_rand(void)
{
    g_seed = (214013 * g_seed + 2531011);
    return (g_seed >> 16) & 0x7FFF;
}