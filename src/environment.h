#pragma once
#include "core.h"

#include "block.h"
#include "direction.h"

//         x/y/z - The position of a block within the minecraft world
// Segment x/y/z - The position of a block within a 16x16x16 segment (0-15)
//    Grid x/y/z - The position of a segment within the world's grid of segments
//
// World position = Grid position * 16 + Segment position

typedef struct
{
    int x;
    int y;
    int z;
    Direction facing;
} EnvironmentScout;

typedef struct
{
    int grid_x;
    int grid_y;
    int grid_z;
    Block block[16][16][16];
} Segment;

typedef struct
{
    size_t count;
    size_t capacity;

    // TODO: Make this a hash map rather than an array to improve look-up speed.
    Segment *segment;

    EnvironmentScout scout;
} Environment;

void init_environment(Environment *environment);
void free_environment(Environment *environment);
void copy_environment(const Environment src, Environment *dst);
void dump_environment(const Environment environment);

Block get_block(const Environment environment, int block_x, int block_y, int block_z);
Block get_block_in_front_of_scout(const Environment environment);
Block get_block_above_scout(const Environment environment);
Block get_block_below_scout(const Environment environment);

void set_block(Environment *environment, int block_x, int block_y, int block_z, Block block);