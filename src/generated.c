// This file was generated automatically by generate.lua

#include "generated.h"

size_t set_network_block_inputs(NetworkValues *values, const Environment environment, size_t next_node, Block block)
{
    (*values)[next_node++] = block == STONE ? 1 : 0;
    (*values)[next_node++] = block == DIRT ? 1 : 0;
    (*values)[next_node++] = block == GRASS ? 1 : 0;
    return next_node;
}

