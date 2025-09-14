// This file was generated automatically by generate.lua

#include "block.h"

const char* block_to_string(Block b) {
    if (b == AIR) return "AIR";
    if (b == STONE) return "STONE";
    if (b == DIRT) return "DIRT";
    if (b == GRASS) return "GRASS";
    UNREACHABLE;
}

const char* block_to_mc(Block b) {
    if (b == AIR) return "minecraft:air";
    if (b == STONE) return "minecraft:stone";
    if (b == DIRT) return "minecraft:dirt";
    if (b == GRASS) return "minecraft:grass";
    UNREACHABLE;
}

