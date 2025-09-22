// This file was generated automatically by generate.lua

#include "item.h"

const char* item_to_string(Item item) {
    switch (item)
    {
        case AIR:             return "AIR";
        case STONE:           return "STONE";
        case COBBLESTONE:     return "COBBLESTONE";
        case DIRT:            return "DIRT";
        case GRASS_BLOCK:     return "GRASS_BLOCK";
        case OAK_SAPLING:     return "OAK_SAPLING";
        case OAK_LOG:         return "OAK_LOG";
        case OAK_LEAVES:      return "OAK_LEAVES";
        case STICK:           return "STICK";
        case APPLE:           return "APPLE";
        case DIAMOND_PICKAXE: return "DIAMOND_PICKAXE";
    }
    UNREACHABLE;
}

const char* item_to_mc(Item item) {
    switch (item)
    {
        case AIR:             return "minecraft:air";
        case STONE:           return "minecraft:stone";
        case COBBLESTONE:     return "minecraft:cobblestone";
        case DIRT:            return "minecraft:dirt";
        case GRASS_BLOCK:     return "minecraft:grass_block";
        case OAK_SAPLING:     return "minecraft:oak_sapling";
        case OAK_LOG:         return "minecraft:oak_log";
        case OAK_LEAVES:      return "minecraft:oak_leaves";
        case STICK:           return "minecraft:stick";
        case APPLE:           return "minecraft:apple";
        case DIAMOND_PICKAXE: return "minecraft:diamond_pickaxe";
    }
    UNREACHABLE;
}

size_t stack_size_of(Item item)
{
    switch (item)
    {
        case AIR:             UNREACHABLE;
        case DIAMOND_PICKAXE: return 1;
        default:              return 64;
    }
}

size_t fuel_value_of(Item item)
{
    switch (item)
    {
        case AIR:             UNREACHABLE;
        default:              return 0;
    }
}

