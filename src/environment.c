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
        int wx = segment.world_x;
        int wy = segment.world_y;
        int wz = segment.world_z;

        for (int sx = 0; sx < 16; sx++)
            for (int sy = 0; sy < 16; sy++)
                for (int sz = 0; sz < 16; sz++)
                {
                    Block block = segment.block[sx][sy][sz];
                    if (block == AIR)
                        continue;

                    int x = wx * 16 + sx;
                    int y = wy * 16 + sy;
                    int z = wz * 16 + sz;
                    fprintf(f, "setblock ~%d ~%d ~%d minecraft:", x, y, z);

                    if (block == STONE)
                        fprintf(f, "stone");
                    else if (block == DIRT)
                        fprintf(f, "dirt");
                    else if (block == GRASS)
                        fprintf(f, "grass_block");
                    else
                        fprintf(f, "bedrock");

                    fprintf(f, "\n");
                }
    }

    fclose(f);
}

Block get_block(const Environment environment, int block_x, int block_y, int block_z)
{
    int world_x = block_x / 16;
    int world_y = block_y / 16;
    int world_z = block_z / 16;

    Segment *segment = NULL;
    for (size_t i = 0; i < environment.count; i++)
    {
        segment = environment.segment + i;
        if (segment->world_x == world_x && segment->world_y == world_y && segment->world_z == world_z)
            break;
    }

    if (segment == NULL)
    {
        // TODO: Generate this segment (and/or make this scenario impossible?)
        return AIR;
    }

    int segment_x = block_x % 16;
    int segment_y = block_y % 16;
    int segment_z = block_z % 16;

    return segment->block[segment_x][segment_y][segment_z];
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

void set_block(Environment *environment, int block_x, int block_y, int block_z, Block block)
{
    int world_x = block_x / 16;
    int world_y = block_y / 16;
    int world_z = block_z / 16;

    Segment *segment = NULL;
    for (size_t i = 0; i < environment->count; i++)
    {
        segment = environment->segment + i;
        if (segment->world_x == world_x && segment->world_y == world_y && segment->world_z == world_z)
            break;
    }

    if (segment == NULL)
    {
        // TODO: Generate this segment (and/or make this scenario impossible?)
        return;
    }

    int segment_x = block_x % 16;
    int segment_y = block_y % 16;
    int segment_z = block_z % 16;

    segment->block[segment_x][segment_y][segment_z] = block;
}