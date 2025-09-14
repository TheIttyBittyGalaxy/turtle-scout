// This file was generated automatically by generate.lua

#include "generated.h"

size_t set_network_block_inputs(NetworkValues *values, const Environment environment, size_t next_node, Block block)
{
    (*values)[next_node++] = block == STONE ? 1 : 0;
    (*values)[next_node++] = block == DIRT ? 1 : 0;
    (*values)[next_node++] = block == GRASS ? 1 : 0;
    return next_node;
}

void perform_dig_action(Environment* environment, Statistics* stats, Block block)
{
    if (block == AIR) return;
    else if (block == STONE) stats->stat[BROKE_STONE]++;
    else if (block == DIRT) stats->stat[BROKE_DIRT]++;
    else if (block == GRASS) stats->stat[BROKE_GRASS]++;
}

