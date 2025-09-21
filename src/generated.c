// This file was generated automatically by generate.lua

#include "generated.h"

void set_network_block_inputs(NetworkValues *values, const Environment environment, size_t *next_node, Item block)
{
    set_network_value(values, (*next_node)++, block == STONE);
    set_network_value(values, (*next_node)++, block == COBBLESTONE);
    set_network_value(values, (*next_node)++, block == DIRT);
    set_network_value(values, (*next_node)++, block == GRASS_BLOCK);
    set_network_value(values, (*next_node)++, block == OAK_SAPLING);
    set_network_value(values, (*next_node)++, block == OAK_LOG);
    set_network_value(values, (*next_node)++, block == OAK_LEAVES);
}

void perform_dig_action(Environment* environment, Statistics* stats, Item block)
{
    switch (block)
    {
    case AIR:
        return;

    case STONE:
        stats->stat[COBBLESTONE_OBTAINED_BY_MINING]++;
        break;

    case COBBLESTONE:
        stats->stat[COBBLESTONE_OBTAINED_BY_MINING]++;
        break;

    case DIRT:
        stats->stat[DIRT_OBTAINED_BY_MINING]++;
        break;

    case GRASS_BLOCK:
        stats->stat[DIRT_OBTAINED_BY_MINING]++;
        break;

    case OAK_SAPLING:
        stats->stat[OAK_SAPLING_OBTAINED_BY_MINING]++;
        break;

    case OAK_LOG:
        stats->stat[OAK_LOG_OBTAINED_BY_MINING]++;
        break;

    case OAK_LEAVES:
        stats->stat[OAK_LEAVES_BROKEN]++;
        if (rand() % 100 < 1)
            stats->stat[STICK_OBTAINED_BY_MINING]++;
        if (rand() % 100 < 1)
            stats->stat[STICK_OBTAINED_BY_MINING]++;
        if (rand() % 100 < 5)
            stats->stat[OAK_SAPLING_OBTAINED_BY_MINING]++;
        if (rand() % 100 < 20)
            stats->stat[APPLE_OBTAINED_BY_MINING]++;
        break;

    }
}

