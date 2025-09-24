// This file was generated automatically by generate.lua

#pragma once
#include "core.h"

typedef enum
{
    COBBLESTONE_OBTAINED_BY_MINING,
    DIRT_OBTAINED_BY_MINING,
    OAK_SAPLING_OBTAINED_BY_MINING,
    OAK_LOG_OBTAINED_BY_MINING,
    STICK_OBTAINED_BY_MINING,
    APPLE_OBTAINED_BY_MINING,

    OAK_LEAVES_BROKEN,

    OAK_LOG_USED_AS_FUEL,
    STICK_USED_AS_FUEL,

    MOVED,
} StatName;

#define NUM_OF_STATISTICS 10

typedef struct
{
    int stat[NUM_OF_STATISTICS];
} Statistics;

void init_scout_stats(Statistics *scout_stats);
const char* stat_name_to_string(StatName name);
