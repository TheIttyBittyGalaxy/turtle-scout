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
    DIAMOND_PICKAXE,
} Item;

const char* item_to_string(Item item);
const char* item_to_mc(Item item);

size_t stack_size_of(Item item);
size_t fuel_value_of(Item item);
