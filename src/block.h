// This file was generated automatically by generate.lua

#pragma once
#include "core.h"

typedef enum
{
    AIR,
    STONE,
    DIRT,
    GRASS_BLOCK,
    OAK_LOG,
    OAK_LEAVES,
} Block;

const char* block_to_string(Block b);
const char* block_to_mc(Block b);
