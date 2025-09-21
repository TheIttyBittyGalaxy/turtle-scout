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
    bool success;
    switch (block)
    {
    case AIR:
        return;

    case STONE:
        success = add_to_scout_inventory(environment, COBBLESTONE);
        if (success) stats->stat[COBBLESTONE_OBTAINED_BY_MINING]++;
        break;

    case COBBLESTONE:
        success = add_to_scout_inventory(environment, COBBLESTONE);
        if (success) stats->stat[COBBLESTONE_OBTAINED_BY_MINING]++;
        break;

    case DIRT:
        success = add_to_scout_inventory(environment, DIRT);
        if (success) stats->stat[DIRT_OBTAINED_BY_MINING]++;
        break;

    case GRASS_BLOCK:
        success = add_to_scout_inventory(environment, DIRT);
        if (success) stats->stat[DIRT_OBTAINED_BY_MINING]++;
        break;

    case OAK_SAPLING:
        success = add_to_scout_inventory(environment, OAK_SAPLING);
        if (success) stats->stat[OAK_SAPLING_OBTAINED_BY_MINING]++;
        break;

    case OAK_LOG:
        success = add_to_scout_inventory(environment, OAK_LOG);
        if (success) stats->stat[OAK_LOG_OBTAINED_BY_MINING]++;
        break;

    case OAK_LEAVES:
        stats->stat[OAK_LEAVES_BROKEN]++;
        if (rand() % 100 < 1)
        {
            success = add_to_scout_inventory(environment, STICK);
            if (success) stats->stat[STICK_OBTAINED_BY_MINING]++;
        }
        if (rand() % 100 < 1)
        {
            success = add_to_scout_inventory(environment, STICK);
            if (success) stats->stat[STICK_OBTAINED_BY_MINING]++;
        }
        if (rand() % 100 < 5)
        {
            success = add_to_scout_inventory(environment, OAK_SAPLING);
            if (success) stats->stat[OAK_SAPLING_OBTAINED_BY_MINING]++;
        }
        if (rand() % 100 < 20)
        {
            success = add_to_scout_inventory(environment, APPLE);
            if (success) stats->stat[APPLE_OBTAINED_BY_MINING]++;
        }
        break;

    }
}

