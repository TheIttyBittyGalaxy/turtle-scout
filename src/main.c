#include <float.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define UNREACHABLE __builtin_unreachable();

// ENVIRONMENTS //

typedef enum
{
    NORTH,
    EAST,
    SOUTH,
    WEST
} Direction;

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

    UNREACHABLE
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

typedef enum
{
    AIR,
    STONE,
    DIRT,
    GRASS,
} Block;

typedef struct
{
    int world_x;
    int world_y;
    int world_z;
    Block block[16][16][16];
} Segment;

typedef struct
{
    int scout_x;
    int scout_y;
    int scout_z;
    Direction scout_facing;

    size_t count;
    size_t capacity;

    // TODO: Make this a hash map rather than an array.
    // This will make look-up faster, and will also mean
    // segments do not have to store their own coordinates.
    Segment *segment;
} Environment;

void init_environment(Environment *environment)
{
    environment->scout_x = 0;
    environment->scout_y = 0;
    environment->scout_z = 0;
    environment->scout_facing = NORTH;

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

// FIXME: Change src to a const
void copy_environment(Environment *src, Environment *dst)
{
    dst->scout_x = src->scout_x;
    dst->scout_y = src->scout_y;
    dst->scout_z = src->scout_z;
    dst->scout_facing = src->scout_facing;

    if (dst->capacity == 0)
    {
        dst->capacity = src->capacity;
        dst->segment = (Segment *)malloc(dst->capacity * sizeof(Segment));
    }
    else if (dst->capacity < src->capacity)
    {
        dst->capacity = src->capacity;
        dst->segment = (Segment *)realloc(dst->segment, dst->capacity * sizeof(Segment));
    }

    dst->count = src->count;
    for (size_t i = 0; i < src->count; i++)
        dst->segment[i] = src->segment[i];
}

void dump_environment(const Environment environment)
{
    FILE *f;
    f = fopen("mcfunct/environment_dump.mcfunction", "w");

    // Turtle (without program, just for reference)
    fprintf(f, "setblock ~%d ~%d ~%d ", environment.scout_x, environment.scout_y, environment.scout_z);
    fprintf(f, "computercraft:turtle_normal[facing=%s", direction_to_mc_string(environment.scout_facing));
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

// GET BLOCKS //

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
    int x = environment.scout_x + x_offset_of(environment.scout_facing);
    int y = environment.scout_y;
    int z = environment.scout_z + z_offset_of(environment.scout_facing);
    return get_block(environment, x, y, z);
}

Block get_block_above_scout(const Environment environment)
{
    return get_block(environment, environment.scout_x, environment.scout_y + 1, environment.scout_z);
}

Block get_block_below_scout(const Environment environment)
{
    return get_block(environment, environment.scout_x, environment.scout_y - 1, environment.scout_z);
}

// SET BLOCKS //

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

// WORLD GEN //

Segment generate_segment(int world_x, int world_y, int world_z)
{
    Segment segment;
    segment.world_x = world_x;
    segment.world_y = world_y;
    segment.world_z = world_z;

    if (world_y > 0)
    {
        for (size_t x = 0; x < 16; x++)
            for (size_t y = 0; y < 16; y++)
                for (size_t z = 0; z < 16; z++)
                    segment.block[x][y][z] = AIR;
    }
    else
    {
        for (size_t x = 0; x < 16; x++)
            for (size_t y = 0; y < 16; y++)
                for (size_t z = 0; z < 16; z++)
                    segment.block[x][y][z] = STONE;

        if (world_y == 0)
        {
            for (size_t x = 0; x < 16; x++)
                for (size_t z = 0; z < 16; z++)
                {
                    segment.block[x][15][z] = GRASS;
                    segment.block[x][14][z] = DIRT;
                    segment.block[x][13][z] = DIRT;
                    segment.block[x][12][z] = DIRT;
                }
        }
    }

    return segment;
}

// ACTIONS //

#define NUM_OF_ACTION 9

typedef enum
{
    IDLE,

    MOVE,
    MOVE_UP,
    MOVE_DOWN,

    DIG,
    DIG_UP,
    DIG_DOWN,

    TURN_LEFT,
    TURN_RIGHT,
} Action;

bool is_move_action(Action action)
{
    return action == MOVE ||
           action == MOVE_UP ||
           action == MOVE_DOWN;
}

bool is_dig_action(Action action)
{
    return action == DIG ||
           action == DIG_UP ||
           action == DIG_DOWN;
}

bool is_up_action(Action action)
{
    return action == MOVE_UP || action == DIG_UP;
}

bool is_down_action(Action action)
{
    return action == MOVE_DOWN || action == DIG_DOWN;
}

// NETWORK VALUES & PARAMETERS //

#define NUM_OF_NODES 64

typedef double NetworkValues[NUM_OF_NODES];

typedef struct
{
    double bias[NUM_OF_NODES];
    double weight[NUM_OF_NODES][NUM_OF_NODES]; // weight[a][b] is connection from a to b
} NetworkParameters;

void reset_network_values(NetworkValues *values, const NetworkParameters parameters)
{
    for (size_t i = 0; i < NUM_OF_NODES; i++)
        (*values)[i] = parameters.bias[i];
}

// EVALUATE NETWORK //

void evaluate_network(const NetworkParameters parameter, NetworkValues *value)
{
    NetworkValues result;

    for (size_t i = 0; i < NUM_OF_NODES; i++)
    {
        result[i] = parameter.bias[i];
        for (size_t j = 0; j < NUM_OF_NODES; j++)
            result[i] += (*value)[j] * parameter.weight[j][i];
    }

    for (size_t i = 0; i < NUM_OF_NODES; i++)
        (*value)[i] = result[i];
}

// STATISTICS //

typedef enum
{
    BROKE_STONE,
    BROKE_DIRT,
    BROKE_GRASS,
    MOVED,
} StatName;

#define NUM_OF_STATISTICS 4

typedef struct
{
    int stat[NUM_OF_STATISTICS];
} Statistics;

void init_scout_stats(Statistics *scout_stats)
{
    for (size_t i = 0; i < NUM_OF_STATISTICS; i++)
        scout_stats->stat[i] = 0;
}

void add_stats(Statistics *scout_stats, const Statistics stats_delta)
{
    for (size_t i = 0; i < NUM_OF_STATISTICS; i++)
        scout_stats->stat[i] += stats_delta.stat[i];
}

// SCOUT POPULATION //

typedef struct
{
    size_t capacity;     // Capacity of the array
    size_t count;        // All members of the population, historic and active
    size_t active_count; // On the active members of the population

    NetworkParameters *scout_parameters;
    Statistics *scout_stats;
    double *scout_novelty_score;
    size_t *scout_generation;
} Population;

// PERFORM ACTIONS //

void set_network_inputs(NetworkValues *values, const Environment environment)
{
#define SET_BLOCK_INPUTS(block)              \
    {                                        \
        Block b = block;                     \
        (*values)[i++] = b == STONE ? 1 : 0; \
        (*values)[i++] = b == DIRT ? 1 : 0;  \
        (*values)[i++] = b == GRASS ? 1 : 0; \
    }

    size_t i = 0;
    SET_BLOCK_INPUTS(get_block_in_front_of_scout(environment));
    SET_BLOCK_INPUTS(get_block_above_scout(environment));
    SET_BLOCK_INPUTS(get_block_below_scout(environment));

#undef SET_BLOCK_INPUT
}

Action determine_network_action(const NetworkValues network_values)
{
    Action action = IDLE;
    double highest_activation = 0;

    for (size_t i = 1; i < NUM_OF_ACTION; i++)
    {
        size_t n = NUM_OF_NODES - i;
        double activation = network_values[n];
        if (activation > highest_activation)
        {
            action = (Action)i;
            highest_activation = activation;
        }
    }

    return action;
}

void perform_action(Environment *environment, const Action action, Statistics *stats)
{
    // IDLE
    if (action == IDLE)
        return;

    // TURN
    if (action == TURN_LEFT)
    {
        environment->scout_facing = left_of(environment->scout_facing);
        return;
    }

    if (action == TURN_RIGHT)
    {
        environment->scout_facing = right_of(environment->scout_facing);
        return;
    }

    // MOVE
    if (is_move_action(action))
    {
        if (is_up_action(action) && get_block_above_scout(*environment) == AIR)
        {
            environment->scout_y++;
        }
        else if (is_down_action(action) && get_block_below_scout(*environment) == AIR)
        {
            environment->scout_y--;
        }
        else if (get_block_in_front_of_scout(*environment) == AIR)
        {
            environment->scout_x += x_offset_of(environment->scout_facing);
            environment->scout_z += z_offset_of(environment->scout_facing);
        }
        else
        {
            return; // Could not move
        }

        stats->stat[MOVED]++;

        return;
    }

    // DIG
    if (is_dig_action(action))
    {
        Block block = AIR;
        int x = environment->scout_x;
        int y = environment->scout_y;
        int z = environment->scout_z;

        if (is_up_action(action))
        {
            block = get_block_above_scout(*environment);
            y++;
        }
        else if (is_down_action(action))
        {
            block = get_block_below_scout(*environment);
            y--;
        }
        else
        {
            block = get_block_in_front_of_scout(*environment);
            x += x_offset_of(environment->scout_facing);
            z += z_offset_of(environment->scout_facing);
        }

        if (block == AIR)
            return;

        set_block(environment, x, y, z, AIR);

        if (block == STONE)
            stats->stat[BROKE_STONE]++;
        else if (block == DIRT)
            stats->stat[BROKE_DIRT]++;
        else if (block == GRASS)
            stats->stat[BROKE_GRASS]++;

        return;
    }

    UNREACHABLE;
}

// MANIPULATE NETWORK PARAMETERS //

// Generates a random number from -1 to 1, with a bias towards 0
// Not an actual normal distribution, but quick and cheap to calculate
double rand_normal()
{
    double a = (double)rand() / RAND_MAX;
    double b = (double)rand() / RAND_MAX;
    return a + b - 1;
}

void randomise_scout_parameters(NetworkParameters *parameters)
{
    for (size_t i = 0; i < NUM_OF_NODES; i++)
        parameters->bias[i] = rand_normal();

    for (size_t i = 0; i < NUM_OF_NODES; i++)
        for (size_t j = 0; j < NUM_OF_NODES; j++)
            parameters->weight[i][j] = rand_normal();
}

void mutate_parameters(NetworkParameters *parameters)
{
#define MUTATE(x)                     \
    {                                 \
        int r = rand() % 2000;        \
        if (r < 1)                    \
            x += rand_normal();       \
        else if (r < 10)              \
            x += rand_normal() / 10;  \
        else if (r < 100)             \
            x += rand_normal() / 100; \
    }

    for (size_t i = 0; i < NUM_OF_NODES; i++)
        MUTATE(parameters->bias[i])

    for (size_t i = 0; i < NUM_OF_NODES; i++)
        for (size_t j = 0; j < NUM_OF_NODES; j++)
            MUTATE(parameters->weight[i][j])

#undef MUTATE
}

// CALCULATE NOVELTY DISTANCE //

// NOTE: In the interests of speed, I'm using taxi-cab distance here (i.e. I'm not square rooting anything)
//       For this purpose, I think that's fine?
double novelty_distance(const Statistics a, const Statistics b)
{
    double score = 0;
    for (size_t i = 0; i < NUM_OF_STATISTICS; i++)
        score += abs(a.stat[i] - b.stat[i]);
    return score;
}

// ITERATE TRAINING //
void iterate_training(Population *population)
{
    size_t population_count = population->count;
    size_t active_count = population->active_count;
    NetworkParameters *scout_parameters = population->scout_parameters;
    Statistics *scout_stats = population->scout_stats;
    double *scout_novelty_score = population->scout_novelty_score;
    size_t *scout_generation = population->scout_generation;

    // Generate environment
    Environment environment;
    Environment copy_of_environment;

    init_environment(&environment);
    init_environment(&copy_of_environment);

    {
        environment.scout_x = 0;
        environment.scout_y = 16;
        environment.scout_z = 0;
        environment.scout_facing = EAST;

        environment.capacity = 27;
        environment.count = 27;
        environment.segment = (Segment *)malloc(27 * sizeof(Segment));

        size_t i = 0;
        for (int world_x = -1; world_x <= 1; world_x++)
            for (int world_y = -1; world_y <= 1; world_y++)
                for (int world_z = -1; world_z <= 1; world_z++)
                    environment.segment[i++] = generate_segment(world_x, world_y, world_z);
    }

    dump_environment(environment);

    // Evaluate each scout
    for (size_t i = 0; i < active_count; i++)
    {
        NetworkValues network_values;
        NetworkParameters parameters = scout_parameters[i];

        reset_network_values(&network_values, parameters);
        init_scout_stats(scout_stats + i);
        copy_environment(&environment, &copy_of_environment);

        for (size_t n = 0; n < 128; n++)
        {
            set_network_inputs(&network_values, copy_of_environment);
            evaluate_network(parameters, &network_values);
            Action action = determine_network_action(network_values);
            perform_action(&copy_of_environment, action, scout_stats + i);
        }
    }

    // FIXME: It's not worth allocating and freeing this memory on every training iteration.
    free_environment(&environment);
    free_environment(&copy_of_environment);

    // Generate novelty scores
    for (size_t i = 0; i < active_count; i++)
    {
        // Find the 8 scouts with the most similar stats and track how similar they are
        double nearest_scouts[8];
        nearest_scouts[0] = DBL_MAX;
        nearest_scouts[1] = DBL_MAX;
        nearest_scouts[2] = DBL_MAX;
        nearest_scouts[3] = DBL_MAX;
        nearest_scouts[4] = DBL_MAX;
        nearest_scouts[5] = DBL_MAX;
        nearest_scouts[6] = DBL_MAX;
        nearest_scouts[7] = DBL_MAX;

        for (size_t j = 0; j < population_count; j++)
        {
            if (i == j)
                continue;

            double dist = novelty_distance(scout_stats[i], scout_stats[j]);

            for (size_t s = 0; s < 8; s++)
            {
                if (dist > nearest_scouts[s])
                    continue;

                for (size_t n = 7; n > s; n--)
                    nearest_scouts[n] = nearest_scouts[n - 1];
                nearest_scouts[s] = dist;
                break;
            }
        }

        // This scout's novelty score is the average of the 8 closest distances
        double score = (nearest_scouts[0] +
                        nearest_scouts[1] +
                        nearest_scouts[2] +
                        nearest_scouts[3] +
                        nearest_scouts[4] +
                        nearest_scouts[5] +
                        nearest_scouts[6] +
                        nearest_scouts[7]) /
                       8;

        scout_novelty_score[i] = score;
    }

    // Sort active population by novelty score
    // TODO: This is bubble sort oopsie!
    for (size_t i = 0; i < population->active_count; i++)
    {
        for (size_t j = i + 1; j < population->active_count; j++)
        {
            if (population->scout_novelty_score[i] < population->scout_novelty_score[j])
            {
                NetworkParameters scout_parameters = population->scout_parameters[i];
                Statistics scout_stats = population->scout_stats[i];
                double scout_novelty_score = population->scout_novelty_score[i];
                size_t scout_generation = population->scout_generation[i];

                population->scout_parameters[i] = population->scout_parameters[j];
                population->scout_stats[i] = population->scout_stats[j];
                population->scout_novelty_score[i] = population->scout_novelty_score[j];
                population->scout_generation[i] = population->scout_generation[j];

                population->scout_parameters[j] = scout_parameters;
                population->scout_stats[j] = scout_stats;
                population->scout_novelty_score[j] = scout_novelty_score;
                population->scout_generation[j] = scout_generation;
            }
        }
    }

    // Replace the least novel half of the population with children from the most novel
    size_t safe_count = active_count / 2;
    for (size_t i = safe_count; i < active_count; i++)
    {
        size_t j = rand() % safe_count;
        population->scout_parameters[i] = population->scout_parameters[j];
        population->scout_generation[i] = population->scout_generation[j] + 1;
        mutate_parameters(&population->scout_parameters[i]);
    }
}

// SAVE METHODS //

void save_parameters_to_lua(const NetworkParameters parameters)
{
    FILE *f;
    f = fopen("save/parameters.lua", "w");

    fprintf(f, "local parameters = {\n\tnodes = %d,", NUM_OF_NODES);

    fprintf(f, "\n\tbias = { %f", parameters.bias[0]);

    for (size_t i = 1; i < NUM_OF_NODES; i++)
        fprintf(f, ", %f", parameters.bias[i]);

    fprintf(f, " },\n\tweight = {");

    for (size_t i = 0; i < NUM_OF_NODES; i++)
    {
        fprintf(f, "\n\t\t{ %f", parameters.weight[i][0]);
        for (size_t j = 1; j < NUM_OF_NODES; j++)
            fprintf(f, ", %f", parameters.weight[i][j]);
        fprintf(f, " },");
    }

    fprintf(f, "\n\t}\n}\n");

    fclose(f);
}

// MAIN FUNCTION //

#define CMD_CHAR_LIMIT 256

int main(int argc, char const *argv[])
{
    char cmd_buffer[CMD_CHAR_LIMIT];

    Population population;
    population.count = 0;
    population.active_count = 0;
    population.capacity = 128;
    population.scout_parameters = (NetworkParameters *)malloc(sizeof(NetworkParameters) * 128);
    population.scout_stats = (Statistics *)malloc(sizeof(Statistics) * 128);
    population.scout_novelty_score = (double *)malloc(sizeof(double) * 128);
    population.scout_generation = (size_t *)malloc(sizeof(size_t) * 128);

    while (true)
    {
        printf("> ");
        fgets(cmd_buffer, CMD_CHAR_LIMIT, stdin);
        for (size_t i = 0; i < CMD_CHAR_LIMIT; i++)
        {
            if (cmd_buffer[i] == '\n')
            {
                cmd_buffer[i] = '\0';
                break;
            }
        }

        // COMMAND: help
        // List the commands available in this program with information about how to use them
        if (strcmp(cmd_buffer, "help") == 0)
        {
            // TODO: Write help command
            printf("Sorry, I haven't written this yet..\n");
        }

        // COMMAND: spawn
        // Create a new population of scouts
        else if (strcmp(cmd_buffer, "spawn") == 0)
        {
            population.count = 64;
            population.active_count = 64;
            for (size_t i = 0; i < population.active_count; i++)
            {
                randomise_scout_parameters(population.scout_parameters + i);
                init_scout_stats(population.scout_stats + i);
                population.scout_novelty_score[i] = 0;
                population.scout_generation[i] = 0;
            }
        }

        // COMMAND: train <iterations>
        // Run a learning iteration
        else if (strcmp(cmd_buffer, "train") == 0)
        {
            // TODO: Repeat this as many times as dictated by the command argument
            iterate_training(&population);
        }

        // COMMAND: info
        // Get information about the current population
        else if (strcmp(cmd_buffer, "info") == 0)
        {
            population.active_count--;
            for (size_t i = 0; i < population.count; i++)
            {
                if (i == population.active_count)
                {
                    printf("   | \n");
                    printf("   | HISTORIC NOVELTY\n");
                }

                printf("%02d | %f\t%d\t",
                       i,
                       population.scout_novelty_score[i],
                       population.scout_generation[i]);

                for (size_t s = 0; s < NUM_OF_STATISTICS; s++)
                    printf("%03d   ", population.scout_stats[i].stat[s]);
                printf("\n");
            }
        }

        // COMMAND: save
        // Save data from the program
        else if (strcmp(cmd_buffer, "save") == 0)
        {
            // TODO: Allow this program to take sub commands so the user can decide what to save
            save_parameters_to_lua(population.scout_parameters[0]);
        }

        // COMMAND: quit
        // Stop the program
        else if (strcmp(cmd_buffer, "quit") == 0)
        {
            break;
        }

        else
        {
            printf("Command not recognised.\n");
        }

        printf("\n");
    }
}