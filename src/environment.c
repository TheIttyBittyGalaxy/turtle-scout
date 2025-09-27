#include "environment.h"

inline bool empty_segment_in_list(const Segment segment)
{
    return segment.grid_x == INT_MAX && segment.grid_y == INT_MAX && segment.grid_z == INT_MAX;
}

inline bool last_segment_in_list(const Segment segment)
{
    return segment.next == SIZE_MAX;
}

void init_environment(Environment *environment)
{
    // Scout
    environment->scout.x = 0;
    environment->scout.y = 0;
    environment->scout.z = 0;
    environment->scout.facing = NORTH;

    environment->scout.fuel = 0;

    environment->scout.selected_inventory_slot = 0;
    for (size_t i = 0; i < 16; i++)
    {
        environment->scout.inventory[i].item = AIR;
        environment->scout.inventory[i].qty = 0;
    }

    // Segments
    environment->count = 0;
    environment->capacity = SEGMENT_HASH_MAP_SIZE;
    environment->overflow = SEGMENT_HASH_MAP_SIZE;
    environment->segment = (Segment *)malloc(environment->capacity * sizeof(Segment));

    for (size_t i = 0; i < environment->capacity; i++)
    {
        environment->segment[i].grid_x = INT_MAX; // Indicates that this segment is empty / unassigned
        environment->segment[i].grid_y = INT_MAX;
        environment->segment[i].grid_z = INT_MAX;
        environment->segment[i].next = SIZE_MAX; // Indicates that this is the last segment in the linked list
    }
}

void copy_environment(const Environment src, Environment *dst)
{
    // Scout
    dst->scout.x = src.scout.x;
    dst->scout.y = src.scout.y;
    dst->scout.z = src.scout.z;
    dst->scout.facing = src.scout.facing;

    dst->scout.fuel = src.scout.fuel;

    dst->scout.selected_inventory_slot = src.scout.selected_inventory_slot;
    for (size_t i = 0; i < 16; i++)
    {
        dst->scout.inventory[i].item = src.scout.inventory[i].item;
        dst->scout.inventory[i].qty = src.scout.inventory[i].qty;
    }

    // Segments
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
    dst->overflow = src.overflow;
    for (size_t i = 0; i < src.capacity; i++)
        dst->segment[i] = src.segment[i];
}

void dump_environment(const Environment environment)
{
    FILE *f;
    f = fopen("export/place_environment.mcfunction", "w");

    // Iterate over each linked list in the hash map
    for (size_t n = 0; n < SEGMENT_HASH_MAP_SIZE; n++)
    {
        if (empty_segment_in_list(environment.segment[n]))
            continue;

        size_t i = n;
        while (true)
        {
            int base_x = environment.segment[i].grid_x * 16;
            int base_y = environment.segment[i].grid_y * 16;
            int base_z = environment.segment[i].grid_z * 16;

            for (int sx = 0; sx < 16; sx++)
                for (int sy = 0; sy < 16; sy++)
                    for (int sz = 0; sz < 16; sz++)
                    {
                        Item block = environment.segment[i].block[sx][sy][sz];
                        if (block == AIR)
                            continue;

                        int x = base_x + sx;
                        int y = base_y + sy;
                        int z = base_z + sz;
                        fprintf(f, "setblock ~%d ~%d ~%d %s\n", x, y, z, item_to_mc(block));
                    }

            if (last_segment_in_list(environment.segment[i]))
                break;

            i = environment.segment[i].next;
        }
    }

    // Turtle (without program or inventory, just for reference)
    fprintf(f, "setblock ~%d ~%d ~%d ", environment.scout.x, environment.scout.y, environment.scout.z);
    fprintf(f, "computercraft:turtle_normal[facing=%s", direction_to_mc_string(environment.scout.facing));
    fprintf(f, "]{LeftUpgrade: {id: \"minecraft:diamond_pickaxe\"}}\n");

    fclose(f);
}

// Source: https://dmauro.com/post/77011214305/a-hashing-function-for-x-y-z-coordinates
//         Thanks David! :)
size_t hash_coordinate(int x, int y, int z)
{
    x = (x >= 0) ? (2 * x) : (-2 * x - 1);
    y = (y >= 0) ? (2 * y) : (-2 * y - 1);
    z = (z >= 0) ? (2 * z) : (-2 * z - 1);

    size_t m = max3(x, y, z);
    size_t h = (m * m * m) + (2 * m * z) + z;

    if (m == z)
        h += max2(x, y) * max2(x, y);
    if (y >= x)
        h += x + y;
    else
        h += y;

    return h;
}

void create_segment(Environment *environment, int grid_x, int grid_y, int grid_z)
{
    size_t i = hash_coordinate(grid_x, grid_y, grid_z) % SEGMENT_HASH_MAP_SIZE;

    // Collision has occurred
    if (!empty_segment_in_list(environment->segment[i]))
    {
        // Assert that the segment does not already exist in the linked list
        assert(!(environment->segment[i].grid_x == grid_x && environment->segment[i].grid_y == grid_y && environment->segment[i].grid_z == grid_z));

        // Traverse to end of linked list
        while (!last_segment_in_list(environment->segment[i]))
        {
            i = environment->segment[i].next;

            // Assert that the segment does not already exist in the linked list
            assert(!(environment->segment[i].grid_x == grid_x && environment->segment[i].grid_y == grid_y && environment->segment[i].grid_z == grid_z));
        }

        // Reallocate memory if overflow has hit capacity
        if (environment->overflow == environment->capacity)
        {
            environment->capacity += SEGMENT_HASH_MAP_SIZE;
            environment->segment = (Segment *)realloc(environment->segment, sizeof(Segment) * environment->capacity);
        }

        // Add overflow segment and link previous segment to it
        size_t next_index = environment->overflow++;
        environment->segment[i].next = next_index;
        i = next_index;
    }

    // Set segment
    environment->count++;
    environment->segment[i].grid_x = grid_x;
    environment->segment[i].grid_y = grid_y;
    environment->segment[i].grid_z = grid_z;
    environment->segment[i].next = SIZE_MAX; // Indicate that this is the last segment in the linked list
}

Segment *get_segment(const Environment environment, int grid_x, int grid_y, int grid_z)
{
    size_t i = hash_coordinate(grid_x, grid_y, grid_z) % SEGMENT_HASH_MAP_SIZE;
    Segment *segment = environment.segment + i;

    while (true)
    {
        // We do not need to check for empty segments, as they will fail the following check
        if (segment->grid_x == grid_x && segment->grid_y == grid_y && segment->grid_z == grid_z)
            return segment;

        if (last_segment_in_list(*segment))
            return NULL;

        segment = environment.segment + segment->next;
    }
}

void set_block(Environment *environment, int x, int y, int z, Item block)
{
    int sx = mod(x, 16);
    int sy = mod(y, 16);
    int sz = mod(z, 16);

    int gx = (x - sx) / 16;
    int gy = (y - sy) / 16;
    int gz = (z - sz) / 16;

    Segment *segment = get_segment(*environment, gx, gy, gz);

    if (segment == NULL)
    {
        // TODO: Once segments are being procedurally generated, this
        //       should no longer occur. Turn warnings back on.
        // printf("WARNING: Attempt to SET block in a segment that does not exist.\n");
        return;
    }

    segment->block[sx][sy][sz] = block;
}

Item get_block(const Environment environment, int x, int y, int z)
{
    int sx = mod(x, 16);
    int sy = mod(y, 16);
    int sz = mod(z, 16);

    int gx = (x - sx) / 16;
    int gy = (y - sy) / 16;
    int gz = (z - sz) / 16;

    Segment *segment = get_segment(environment, gx, gy, gz);

    if (segment == NULL)
    {
        // TODO: Once segments are being procedurally generated, this
        //       should no longer occur. Turn warnings back on.
        // printf("WARNING: Attempt to GET block in a segment that does not exist.\n");
        return AIR;
    }

    return segment->block[sx][sy][sz];
}

Item get_block_in_front_of_scout(const Environment environment)
{
    return get_block(
        environment,
        environment.scout.x + x_offset_of(environment.scout.facing),
        environment.scout.y,
        environment.scout.z + z_offset_of(environment.scout.facing));
}

Item get_block_above_scout(const Environment environment)
{
    return get_block(
        environment,
        environment.scout.x,
        environment.scout.y + 1,
        environment.scout.z);
}

Item get_block_below_scout(const Environment environment)
{
    return get_block(
        environment,
        environment.scout.x,
        environment.scout.y - 1,
        environment.scout.z);
}

bool add_to_scout_inventory(Environment *environment, Item item)
{
    for (size_t offset = 0; offset < 16; offset++)
    {
        size_t i = (environment->scout.selected_inventory_slot + offset) % 16;
        InventorySlot *slot = environment->scout.inventory + i;

        if (slot->qty == 0)
        {
            slot->item = item;
            slot->qty = 1;
            return true;
        }

        if (slot->item == item && slot->qty < stack_size_of(item))
        {
            slot->qty++;
            return true;
        }
    }

    return false;
}

void decrement_increment_slot(InventorySlot *slot)
{
    if (slot->qty == 0)
        return;

    slot->qty--;

    if (slot->qty == 0)
        slot->item = AIR;
}