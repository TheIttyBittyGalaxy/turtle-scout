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

// NETWORK VALUES & PARAMETERS //

#define NUM_OF_NODES 64

typedef int NetworkValues[NUM_OF_NODES];

typedef struct
{
    double bias[NUM_OF_NODES];
    double weight[NUM_OF_NODES][NUM_OF_NODES]; // weight[a][b] is connection from a to b
} NetworkParameters;

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

// SCOUT ACTIONS //

typedef struct
{
    // TODO: Implement
    void *unimplemented;
} Action;

// STATISTICS //

#define NUM_OF_STATISTICS 8

typedef struct
{
    int stat[NUM_OF_STATISTICS];
} Statistics;

void initialise_scout_stats(Statistics *scout_stats)
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

// UNIMPLEMENTED FUNCTIONS //

void set_network_inputs(NetworkValues *values, const Environment environment)
{
    // TODO: Implement
}

Action determine_network_action(const NetworkValues network_values)
{
    return {}; // TODO: Implement
}

void update_environment(Environment *world, const Action action, Statistics *stats_delta)
{
    // TODO: Implement
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
    NetworkValues network_values;
    for (size_t i = 0; i < active_count; i++)
    {
        NetworkParameters parameters = scout_parameters[i];

        for (size_t i = 0; i < NUM_OF_NODES; i++)
            network_values[i] = parameters.bias[i];

        copy_environment(&environment, &copy_of_environment);
        initialise_scout_stats(scout_stats + i);

        for (size_t n = 0; n < 1000; n++)
        {
            set_network_inputs(&network_values, copy_of_environment);
            evaluate_network(parameters, &network_values);
            Action action = determine_network_action(network_values);

            Statistics stats_delta;
            update_environment(&copy_of_environment, action, &stats_delta);
            add_stats(scout_stats + i, stats_delta);
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
                initialise_scout_stats(population.scout_stats + i);
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
            printf("   | ACTIVE POPULATION\n");
            for (size_t i = 0; i < population.active_count; i++)
            {
                printf("%02d | %f\t%d\n",
                       i,
                       population.scout_novelty_score[i],
                       population.scout_generation[i]);
            }

            printf("   | \n");
            printf("   | HISTORIC NOVELTY\n");
            for (size_t i = population.active_count; i < population.count; i++)
            {
                printf("%02d | %f\t%d\n",
                       i,
                       population.scout_novelty_score[i],
                       population.scout_generation[i]);
            }
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