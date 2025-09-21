// This file was generated automatically by generate.lua

#pragma once
#include "core.h"

typedef enum
{
    AIR,
    STONE,
    COBBLESTONE,
    DIRT,
    GRASS_BLOCK,
    OAK_SAPLING,
    OAK_LOG,
    OAK_LEAVES,
    STICK,
    APPLE,
} Block;

const char* item_to_string(Block b);
const char* item_to_mc(Block b);
