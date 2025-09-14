#include "environment.h"

void init_environment(Environment *environment)
{
    environment->scout.x = 0;
    environment->scout.y = 0;
    environment->scout.z = 0;
    environment->scout.facing = NORTH;

    environment->count = 0;
    environment->capacity = 0;
    environment->segment = NULL;
}

void free_environment(Environment *environment)
{
    environment->count = 0;
    environment->capacity = 0;
    if (environment->segment)
        free(environment->segment);
}

void copy_environment(const Environment src, Environment *dst)
{
    dst->scout.x = src.scout.x;
    dst->scout.y = src.scout.y;
    dst->scout.z = src.scout.z;
    dst->scout.facing = src.scout.facing;

    if (dst->capacity == 0)
    {
        dst->capacity = src.capacity;
        dst->segment = (Segment *)malloc(dst->capacity * sizeof(Segment));
    }
    else if (dst->capacity < src.capacity)
    {
        dst->capacity = src.capacity;
        dst->segment = (Segment *)realloc(dst->segment, dst->capacity * sizeof(Segment));
    }

    dst->count = src.count;
    for (size_t i = 0; i < src.count; i++)
        dst->segment[i] = src.segment[i];
}

void dump_environment(const Environment environment)
{
    FILE *f;
    f = fopen("mcfunct/environment_dump.mcfunction", "w");

    // Turtle (without program, just for reference)
    fprintf(f, "setblock ~%d ~%d ~%d ", environment.scout.x, environment.scout.y, environment.scout.z);
    fprintf(f, "computercraft:turtle_normal[facing=%s", direction_to_mc_string(environment.scout.facing));
    fprintf(f, "]{LeftUpgrade: {id: \"minecraft:diamond_pickaxe\"}}\n");

    // Blocks in each segment
    for (size_t i = 0; i < environment.count; i++)
    {
        Segment segment = environment.segment[i];

        for (int sx = 0; sx < 16; sx++)
            for (int sy = 0; sy < 16; sy++)
                for (int sz = 0; sz < 16; sz++)
                {
                    Block block = segment.block[sx][sy][sz];
                    if (block == AIR)
                        continue;

                    int x = segment.grid_x * 16 + sx;
                    int y = segment.grid_y * 16 + sy;
                    int z = segment.grid_z * 16 + sz;
                    fprintf(f, "setblock ~%d ~%d ~%d %s\n", x, y, z, block_to_mc(block));
                }
    }

    fclose(f);
}

Block get_block(const Environment environment, int x, int y, int z)
{
    int sx = mod(x, 16);
    int sy = mod(y, 16);
    int sz = mod(z, 16);

    int gx = (x - sx) / 16;
    int gy = (y - sy) / 16;
    int gz = (z - sz) / 16;

    Segment *segment = NULL;
    for (size_t i = 0; i < environment.count; i++)
    {
        segment = environment.segment + i;
        if (segment->grid_x == gx && segment->grid_y == gy && segment->grid_z == gz)
            break;
    }

    if (segment == NULL)
    {
        // TODO: Generate this segment (and/or make this scenario impossible?)
        return AIR;
    }

    return segment->block[sx][sy][sz];
}

Block get_block_in_front_of_scout(const Environment environment)
{
    int x = environment.scout.x + x_offset_of(environment.scout.facing);
    int y = environment.scout.y;
    int z = environment.scout.z + z_offset_of(environment.scout.facing);
    return get_block(environment, x, y, z);
}

Block get_block_above_scout(const Environment environment)
{
    return get_block(environment, environment.scout.x, environment.scout.y + 1, environment.scout.z);
}

Block get_block_below_scout(const Environment environment)
{
    return get_block(environment, environment.scout.x, environment.scout.y - 1, environment.scout.z);
}

void set_block(Environment *environment, int x, int y, int z, Block block)
{
    int sx = mod(x, 16);
    int sy = mod(y, 16);
    int sz = mod(z, 16);

    int gx = (x - sx) / 16;
    int gy = (y - sy) / 16;
    int gz = (z - sz) / 16;

    Segment *segment = NULL;
    for (size_t i = 0; i < environment->count; i++)
    {
        segment = environment->segment + i;
        if (segment->grid_x == gx && segment->grid_y == gy && segment->grid_z == gz)
            break;
    }

    if (segment == NULL)
    {
        // TODO: Generate this segment (and/or make this scenario impossible?)
        return;
    }

    segment->block[sx][sy][sz] = block;
}