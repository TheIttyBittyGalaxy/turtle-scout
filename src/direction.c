#include "direction.h"

const char *direction_to_mc_string(Direction d)
{
    if (d == NORTH)
        return "north";
    if (d == EAST)
        return "east";
    if (d == SOUTH)
        return "south";
    if (d == WEST)
        return "west";

    UNREACHABLE;
}

Direction left_of(Direction d)
{
    return (Direction)(((int)d + 3) % 4);
}

Direction right_of(Direction d)
{
    return (Direction)(((int)d + 1) % 4);
}

int x_offset_of(Direction d)
{
    if (d == EAST)
        return 1;
    if (d == WEST)
        return -1;
    return 0;
}

int z_offset_of(Direction d)
{
    if (d == NORTH)
        return -1;
    if (d == SOUTH)
        return 1;
    return 0;
}