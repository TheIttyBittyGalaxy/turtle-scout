// This file was generated automatically by generate.lua

#include "generated.h"

void set_network_inputs_for_item(NetworkValues *values, const Environment environment, size_t *next_node, Item item)
{
    set_network_value(values, (*next_node)++, item == STONE);
    set_network_value(values, (*next_node)++, item == COBBLESTONE);
    set_network_value(values, (*next_node)++, item == DIRT);
    set_network_value(values, (*next_node)++, item == GRASS_BLOCK);
    set_network_value(values, (*next_node)++, item == OAK_SAPLING);
    set_network_value(values, (*next_node)++, item == OAK_LOG);
    set_network_value(values, (*next_node)++, item == OAK_LEAVES);
    set_network_value(values, (*next_node)++, item == STICK);
    set_network_value(values, (*next_node)++, item == APPLE);
    set_network_value(values, (*next_node)++, item == DIAMOND_PICKAXE);
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

void update_refuel_stat(Statistics* stats, Item item)
{
    switch (item)
    {
        case OAK_LOG:
             stats->stat[OAK_LOG_USED_AS_FUEL]++;
             break;
        case STICK:
             stats->stat[STICK_USED_AS_FUEL]++;
             break;
        default: UNREACHABLE;
    }
}

