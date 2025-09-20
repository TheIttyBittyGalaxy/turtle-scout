// This file was generated automatically by generate.lua

#include "generated.h"

void set_network_block_inputs(NetworkValues *values, const Environment environment, size_t *next_node, Block block)
{
    set_network_value(values, (*next_node)++, block == STONE);
    set_network_value(values, (*next_node)++, block == DIRT);
    set_network_value(values, (*next_node)++, block == GRASS_BLOCK);
    set_network_value(values, (*next_node)++, block == OAK_LOG);
    set_network_value(values, (*next_node)++, block == OAK_LEAVES);
}

void perform_dig_action(Environment* environment, Statistics* stats, Block block)
{
    if (block == AIR) return;
    else if (block == STONE) stats->stat[BROKE_STONE]++;
    else if (block == DIRT) stats->stat[BROKE_DIRT]++;
    else if (block == GRASS_BLOCK) stats->stat[BROKE_GRASS_BLOCK]++;
    else if (block == OAK_LOG) stats->stat[BROKE_OAK_LOG]++;
    else if (block == OAK_LEAVES) stats->stat[BROKE_OAK_LEAVES]++;
}

