#include "core.h"

#include "action.h"
#include "environment.h"
#include "network.h"
#include "statistics.h"

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

// SCOUT POPULATION //

typedef struct
{
    size_t capacity;     // Capacity of the array
    size_t count;        // All members of the population, historic and active
    size_t active_count; // On the active members of the population

    Network *scout_network;
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
        environment->scout.facing = left_of(environment->scout.facing);
        return;
    }

    if (action == TURN_RIGHT)
    {
        environment->scout.facing = right_of(environment->scout.facing);
        return;
    }

    // MOVE
    if (is_move_action(action))
    {
        if (is_up_action(action) && get_block_above_scout(*environment) == AIR)
        {
            environment->scout.y++;
        }
        else if (is_down_action(action) && get_block_below_scout(*environment) == AIR)
        {
            environment->scout.y--;
        }
        else if (get_block_in_front_of_scout(*environment) == AIR)
        {
            environment->scout.x += x_offset_of(environment->scout.facing);
            environment->scout.z += z_offset_of(environment->scout.facing);
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
        int x = environment->scout.x;
        int y = environment->scout.y;
        int z = environment->scout.z;

        if (is_up_action(action))
        {
            y++;
        }
        else if (is_down_action(action))
        {
            y--;
        }
        else
        {
            x += x_offset_of(environment->scout.facing);
            z += z_offset_of(environment->scout.facing);
        }

        Block block = get_block(*environment, x, y, z);
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

// CALCULATE NOVELTY DISTANCE //

// TODO: Is there any way to reason about whether to use log(x+1) or sqrt(x) to implement
//       diminishing returns? Which would be "better" to use? Loose testing suggests
//       log more aggressively punishes scouts that repeat the same action many times?

// NOTE: In the interests of speed, I'm using taxi-cab distance here.
//       i.e. I'm not square rooting the difference between each scout.
//       For this purpose, I think that's fine?
double novelty_distance(const Statistics a, const Statistics b)
{
    double score = 0;
    for (size_t i = 0; i < NUM_OF_STATISTICS; i++)
    {
        double diminished_a = sqrt(a.stat[i]);
        double diminished_b = sqrt(b.stat[i]);
        score += abs(diminished_a - diminished_b);
    }
    return score;
}

// ITERATE TRAINING //
void iterate_training(Population *population)
{
    size_t population_count = population->count;
    size_t active_count = population->active_count;
    Network *scout_network = population->scout_network;
    Statistics *scout_stats = population->scout_stats;
    double *scout_novelty_score = population->scout_novelty_score;
    size_t *scout_generation = population->scout_generation;

    // Generate environment
    Environment environment;
    Environment copy_of_environment;

    init_environment(&environment);
    init_environment(&copy_of_environment);

    {
        environment.scout.x = 0;
        environment.scout.y = 16;
        environment.scout.z = 0;
        environment.scout.facing = EAST;

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
        Network network = scout_network[i];

        reset_network_values(network, &network_values);
        init_scout_stats(scout_stats + i);
        copy_environment(&environment, &copy_of_environment);

        for (size_t n = 0; n < 128; n++)
        {
            set_network_inputs(&network_values, copy_of_environment);
            evaluate_network_values(network, &network_values);
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
                Network scout_network = population->scout_network[i];
                Statistics scout_stats = population->scout_stats[i];
                double scout_novelty_score = population->scout_novelty_score[i];
                size_t scout_generation = population->scout_generation[i];

                population->scout_network[i] = population->scout_network[j];
                population->scout_stats[i] = population->scout_stats[j];
                population->scout_novelty_score[i] = population->scout_novelty_score[j];
                population->scout_generation[i] = population->scout_generation[j];

                population->scout_network[j] = scout_network;
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
        population->scout_network[i] = population->scout_network[j];
        population->scout_generation[i] = population->scout_generation[j] + 1;
        mutate_network(&population->scout_network[i]);

        // NOTE: The below isn't necessary for the program to function, but it
        // avoids confusion in the `info` command by not showing the replaced
        // scout's data next to the new child.

        // FIXME: To avoid this refactor the program so that we purge and
        //        repopulate at the start of every iteration except the first.

        init_scout_stats(population->scout_stats + i);
        population->scout_novelty_score[i] = 0;
    }
}

// MAIN FUNCTION //

#define CMD_CHAR_LIMIT 256

int main(int argc, char const *argv[])
{
    // Command variables
    char cmd_buffer[CMD_CHAR_LIMIT];
    char *cmd_args[8];
    size_t cmd_arg_count;

    // TODO: Allow user to specify the seed
    printf("Seed 42.\n");
    srand(42);

    // Initialise population
    Population population;
    population.count = 64;
    population.capacity = 128;

    population.active_count = population.count;
    population.scout_network = (Network *)malloc(sizeof(Network) * population.capacity);
    population.scout_stats = (Statistics *)malloc(sizeof(Statistics) * population.capacity);
    population.scout_novelty_score = (double *)malloc(sizeof(double) * population.capacity);
    population.scout_generation = (size_t *)malloc(sizeof(size_t) * population.capacity);

    // Generate random initial population
    printf("Creating initial population.\n");
    for (size_t i = 0; i < population.active_count; i++)
    {
        randomise_network(population.scout_network + i);
        init_scout_stats(population.scout_stats + i);
        population.scout_novelty_score[i] = 0;
        population.scout_generation[i] = 0;
    }

// Command loop
#define CMD_IS(s) (strcmp(cmd_buffer, s) == 0)
    while (true)
    {
        // Enter new command
        printf("> ");
        fgets(cmd_buffer, CMD_CHAR_LIMIT, stdin);

        cmd_arg_count = 0;
        for (size_t i = 0; i < CMD_CHAR_LIMIT; i++)
        {
            if (cmd_buffer[i] == ' ')
            {
                cmd_buffer[i] = '\0';
                cmd_args[cmd_arg_count++] = cmd_buffer + i + 1;
            }
            else if (cmd_buffer[i] == '\n')
            {
                cmd_buffer[i] = '\0';
                break;
            }
        }

        // COMMAND: help
        // List the commands available in this program with information about how to use them
        if (CMD_IS("help"))
        {
            // TODO: Write help command
            printf("info               : List the current population of scouts.\n");
            printf("train              : Run a training iteration.\n");
            printf("train <iterations> : Run <iterations> training iterations.\n");
            printf("save               : Save the most novel scout in the population.\n");
            printf("save <scout_index> : Save the scout in <scout_index>.\n");
        }

        // COMMAND: train <iterations>
        // Run a learning iteration
        else if (CMD_IS("train"))
        {
            if (cmd_arg_count == 0)
            {
                iterate_training(&population);
            }
            else if (cmd_arg_count == 1)
            {
                size_t iterations = atoi(cmd_args[0]);
                for (size_t i = 0; i < iterations; i++)
                {
                    printf("\rIteration %d / %d", i + 1, iterations);
                    iterate_training(&population);
                }
                printf("\n");
            }
            else
            {
                printf("Usage: train <iterations>\n");
            }
        }

        // COMMAND: info
        // Get information about the current population
        else if (CMD_IS("info"))
        {
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
        // Save the network parameters for a specific turtle
        else if (CMD_IS("save"))
        {
            if (cmd_arg_count == 0)
            {
                dump_network_to_lua(population.scout_network[0]);
            }
            else if (cmd_arg_count == 1)
            {
                size_t scout_index = atoi(cmd_args[0]);
                dump_network_to_lua(population.scout_network[scout_index]);
            }
            else
            {
                printf("Usage: save <scout_index>\n");
            }
        }

        // COMMAND: quit
        // Stop the program
        else if (CMD_IS("quit"))
        {
            break;
        }

        else
        {
            printf("Command not recognised.\n");
        }
    }
}