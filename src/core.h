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

#define UNREACHABLE __builtin_unreachable();

#define ALLOCATE(memory, amt, Type) memory = (Type *)malloc(sizeof(Type) * amt)
#define REALLOCATE(memory, amt, Type) memory = (Type *)realloc(memory, sizeof(Type) * amt)

int mod(int a, int b);

int max2(int a, int b);
int max3(int a, int b, int c);