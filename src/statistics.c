// This file was generated automatically by generate.lua

#include "statistics.h"

void init_scout_stats(Statistics *scout_stats)
{
    for (size_t i = 0; i < NUM_OF_STATISTICS; i++)
        scout_stats->stat[i] = 0;
}

const char* stat_name_to_string(StatName name) {
    if (name == BROKE_STONE) return "BROKE_STONE";
    if (name == BROKE_DIRT) return "BROKE_DIRT";
    if (name == BROKE_GRASS_BLOCK) return "BROKE_GRASS_BLOCK";
    if (name == BROKE_OAK_LOG) return "BROKE_OAK_LOG";
    if (name == BROKE_OAK_LEAVES) return "BROKE_OAK_LEAVES";
    if (name == MOVED) return "MOVED";
    UNREACHABLE;
}

