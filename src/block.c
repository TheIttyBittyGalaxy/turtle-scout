// This file was generated automatically by generate.lua

#include "block.h"

const char* item_to_string(Item item) {
    if (item == AIR) return "AIR";
    if (item == STONE) return "STONE";
    if (item == COBBLESTONE) return "COBBLESTONE";
    if (item == DIRT) return "DIRT";
    if (item == GRASS_BLOCK) return "GRASS_BLOCK";
    if (item == OAK_SAPLING) return "OAK_SAPLING";
    if (item == OAK_LOG) return "OAK_LOG";
    if (item == OAK_LEAVES) return "OAK_LEAVES";
    if (item == STICK) return "STICK";
    if (item == APPLE) return "APPLE";
    UNREACHABLE;
}

const char* item_to_mc(Item item) {
    if (item == AIR) return "minecraft:air";
    if (item == STONE) return "minecraft:stone";
    if (item == COBBLESTONE) return "minecraft:cobblestone";
    if (item == DIRT) return "minecraft:dirt";
    if (item == GRASS_BLOCK) return "minecraft:grass_block";
    if (item == OAK_SAPLING) return "minecraft:oak_sapling";
    if (item == OAK_LOG) return "minecraft:oak_log";
    if (item == OAK_LEAVES) return "minecraft:oak_leaves";
    if (item == STICK) return "minecraft:stick";
    if (item == APPLE) return "minecraft:apple";
    UNREACHABLE;
}

