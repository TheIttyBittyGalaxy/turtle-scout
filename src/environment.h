#pragma once
#include "core.h"

#include "item.h"
#include "direction.h"

//         x/y/z - The position of a block within the minecraft world
// Segment x/y/z - The position of a block within a 16x16x16 segment (0-15)
//    Grid x/y/z - The position of a segment within the world's grid of segments
//
// World position = Grid position * 16 + Segment position

typedef struct
{
    Item item;
    size_t qty;
} InventorySlot;

typedef struct
{
    int x;
    int y;
    int z;
    Direction facing;

    size_t fuel;

    size_t selected_inventory_slot;
    InventorySlot inventory[16];
} EnvironmentScout;

typedef struct
{
    int grid_x;
    int grid_y;
    int grid_z;
    Item block[16][16][16];
} Segment;

typedef struct
{
    size_t count;
    size_t capacity;
    Segment *segment;

    EnvironmentScout scout;
} Environment;

void init_environment(Environment *environment);
void free_environment(Environment *environment);
void copy_environment(const Environment src, Environment *dst);
void dump_environment(const Environment environment);

Segment *get_segment(const Environment environment, int grid_x, int grid_y, int grid_z);

void set_block(Environment *environment, int x, int y, int z, Item block);

Item get_block(const Environment environment, int x, int y, int z);
Item get_block_in_front_of_scout(const Environment environment);
Item get_block_above_scout(const Environment environment);
Item get_block_below_scout(const Environment environment);

bool add_to_scout_inventory(Environment *environment, Item item);
void decrement_increment_slot(InventorySlot *slot);