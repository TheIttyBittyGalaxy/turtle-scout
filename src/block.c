// This file was generated automatically by generate.lua

#include "block.h"

const char* item_to_string(Block b) {
    if (b == AIR) return "AIR";
    if (b == STONE) return "STONE";
    if (b == COBBLESTONE) return "COBBLESTONE";
    if (b == DIRT) return "DIRT";
    if (b == GRASS_BLOCK) return "GRASS_BLOCK";
    if (b == OAK_SAPLING) return "OAK_SAPLING";
    if (b == OAK_LOG) return "OAK_LOG";
    if (b == OAK_LEAVES) return "OAK_LEAVES";
    if (b == STICK) return "STICK";
    if (b == APPLE) return "APPLE";
    UNREACHABLE;
}

const char* item_to_mc(Block b) {
    if (b == AIR) return "minecraft:air";
    if (b == STONE) return "minecraft:stone";
    if (b == COBBLESTONE) return "minecraft:cobblestone";
    if (b == DIRT) return "minecraft:dirt";
    if (b == GRASS_BLOCK) return "minecraft:grass_block";
    if (b == OAK_SAPLING) return "minecraft:oak_sapling";
    if (b == OAK_LOG) return "minecraft:oak_log";
    if (b == OAK_LEAVES) return "minecraft:oak_leaves";
    if (b == STICK) return "minecraft:stick";
    if (b == APPLE) return "minecraft:apple";
    UNREACHABLE;
}

