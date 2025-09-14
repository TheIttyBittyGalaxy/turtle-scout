#pragma once
#include "core.h"

typedef enum
{
    NORTH,
    EAST,
    SOUTH,
    WEST
} Direction;

const char *direction_to_mc_string(Direction d);
Direction left_of(Direction d);
Direction right_of(Direction d);
int x_offset_of(Direction d);
int z_offset_of(Direction d);
