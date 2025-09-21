// This file was generated automatically by generate.lua

#include "statistics.h"

void init_scout_stats(Statistics *scout_stats)
{
    for (size_t i = 0; i < NUM_OF_STATISTICS; i++)
        scout_stats->stat[i] = 0;
}

const char* stat_name_to_string(StatName name) {
    if (name == COBBLESTONE_OBTAINED_BY_MINING) return "COBBLESTONE_OBTAINED_BY_MINING";
    if (name == DIRT_OBTAINED_BY_MINING) return "DIRT_OBTAINED_BY_MINING";
    if (name == OAK_SAPLING_OBTAINED_BY_MINING) return "OAK_SAPLING_OBTAINED_BY_MINING";
    if (name == OAK_LOG_OBTAINED_BY_MINING) return "OAK_LOG_OBTAINED_BY_MINING";
    if (name == STICK_OBTAINED_BY_MINING) return "STICK_OBTAINED_BY_MINING";
    if (name == APPLE_OBTAINED_BY_MINING) return "APPLE_OBTAINED_BY_MINING";
    if (name == OAK_LEAVES_BROKEN) return "OAK_LEAVES_BROKEN";
    if (name == MOVED) return "MOVED";
    UNREACHABLE;
}

