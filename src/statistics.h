#pragma once
#include "core.h"

typedef enum
{
    BROKE_STONE,
    BROKE_DIRT,
    BROKE_GRASS,
    MOVED,
} StatName;

#define NUM_OF_STATISTICS 4

typedef struct
{
    int stat[NUM_OF_STATISTICS];
} Statistics;

void init_scout_stats(Statistics *scout_stats);