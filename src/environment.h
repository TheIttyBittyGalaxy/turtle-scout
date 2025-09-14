#pragma once
#include "core.h"

#include "block.h"
#include "direction.h"

typedef struct
{
    int x;
    int y;
    int z;
    Direction facing;
} EnvironmentScout;

typedef struct
{
    int world_x;
    int world_y;
    int world_z;
    Block block[16][16][16];
} Segment;

typedef struct
{
    size_t count;
    size_t capacity;

    // TODO: Make this a hash map rather than an array.
    // This will make look-up faster, and will also mean
    // segments do not have to store their own coordinates.
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