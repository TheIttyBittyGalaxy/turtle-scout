#include "core.h"

#include "action.h"
#include "environment.h"
#include "network.h"
#include "statistics.h"
#include "generated.h"

// WORLD GEN //

Segment generate_segment(int grid_x, int grid_y, int grid_z)
{
    Segment segment;
    segment.grid_x = grid_x;
    segment.grid_y = grid_y;
    segment.grid_z = grid_z;

    if (grid_y > 0)
    {
        for (size_t sx = 0; sx < 16; sx++)
            for (size_t sy = 0; sy < 16; sy++)
                for (size_t sz = 0; sz < 16; sz++)
                    segment.block[sx][sy][sz] = AIR;
    }
    else
    {
        for (size_t sx = 0; sx < 16; sx++)
            for (size_t sy = 0; sy < 16; sy++)
                for (size_t sz = 0; sz < 16; sz++)
                    segment.block[sx][sy][sz] = STONE;

        if (grid_y == 0)
        {
            for (size_t sx = 0; sx < 16; sx++)
                for (size_t sz = 0; sz < 16; sz++)
                {
                    segment.block[sx][15][sz] = GRASS_BLOCK;
                    segment.block[sx][14][sz] = DIRT;
                    segment.block[sx][13][sz] = DIRT;
                    segment.block[sx][12][sz] = DIRT;
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

    size_t next_id;

    size_t *scout_id;
    size_t *scout_generation;
    Network *scout_network;
    Statistics *scout_stats;
    double *scout_novelty_score;
} Population;

// PERFORM ACTIONS //

void set_network_inputs(NetworkValues *values, const Environment environment)
{
    size_t next_node = 0;
    next_node = set_network_block_inputs(values, environment, next_node, get_block_in_front_of_scout(environment));
    next_node = set_network_block_inputs(values, environment, next_node, get_block_above_scout(environment));
    next_node = set_network_block_inputs(values, environment, next_node, get_block_below_scout(environment));
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
        perform_dig_action(environment, stats, block);
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

    size_t *scout_id = population->scout_id;
    size_t *scout_generation = population->scout_generation;
    Network *scout_network = population->scout_network;
    Statistics *scout_stats = population->scout_stats;
    double *scout_novelty_score = population->scout_novelty_score;

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
        for (int gx = -1; gx <= 1; gx++)
            for (int gy = -1; gy <= 1; gy++)
                for (int gz = -1; gz <= 1; gz++)
                    environment.segment[i++] = generate_segment(gx, gy, gz);

        for (int y = 16; y <= 22; y++)
        {
            set_block(&environment, 1, y, 0, OAK_LOG);
            if (y > 17)
            {
                set_block(&environment, 2, y, 0, OAK_LEAVES);
                set_block(&environment, 0, y, 0, OAK_LEAVES);
                set_block(&environment, 1, y, 1, OAK_LEAVES);
                set_block(&environment, 1, y, -1, OAK_LEAVES);
            }
        }
    }

    dump_environment(environment);

    // Evaluate each scout
    for (size_t i = 0; i < active_count; i++)
    {
        NetworkValues network_values;
        Network network = scout_network[i];

        reset_network_values(network, &network_values);
        init_scout_stats(scout_stats + i);
        copy_environment(environment, &copy_of_environment);

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
                size_t scout_id = population->scout_id[i];
                size_t scout_generation = population->scout_generation[i];
                Network scout_network = population->scout_network[i];
                Statistics scout_stats = population->scout_stats[i];
                double scout_novelty_score = population->scout_novelty_score[i];

                population->scout_id[i] = population->scout_id[j];
                population->scout_generation[i] = population->scout_generation[j];
                population->scout_network[i] = population->scout_network[j];
                population->scout_stats[i] = population->scout_stats[j];
                population->scout_novelty_score[i] = population->scout_novelty_score[j];

                population->scout_id[j] = scout_id;
                population->scout_generation[j] = scout_generation;
                population->scout_network[j] = scout_network;
                population->scout_stats[j] = scout_stats;
                population->scout_novelty_score[j] = scout_novelty_score;
            }
        }
    }

    // Replace the least novel half of the population with children from the most novel
    size_t safe_count = active_count / 2;
    for (size_t i = safe_count; i < active_count; i++)
    {
        size_t j = rand() % safe_count;
        population->scout_id[i] = population->next_id++;
        population->scout_generation[i] = population->scout_generation[j] + 1;
        population->scout_network[i] = population->scout_network[j];
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
    population.next_id = population.count;

    population.scout_id = (size_t *)malloc(sizeof(size_t) * population.capacity);
    population.scout_generation = (size_t *)malloc(sizeof(size_t) * population.capacity);
    population.scout_network = (Network *)malloc(sizeof(Network) * population.capacity);
    population.scout_stats = (Statistics *)malloc(sizeof(Statistics) * population.capacity);
    population.scout_novelty_score = (double *)malloc(sizeof(double) * population.capacity);

    // Generate random initial population
    printf("Creating initial population.\n");
    for (size_t i = 0; i < population.active_count; i++)
    {
        population.scout_id[i] = i;
        population.scout_generation[i] = 0;

        randomise_network(population.scout_network + i);
        init_scout_stats(population.scout_stats + i);
        population.scout_novelty_score[i] = 0;
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
            printf("info <scout_id>    : List the stats of the scout with ID <scout_id>.\n");
            printf("train              : Run a training iteration.\n");
            printf("train <iterations> : Run <iterations> training iterations.\n");
            printf("save <scout_id>    : Save the scout with ID <scout_id>.\n");
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
            if (cmd_arg_count == 0)
            {
                printf("Rank |   ID | Score    | Gen\n");
                printf("---- | ---- | -------- | ---\n");
                for (size_t i = 0; i < population.count; i++)
                {
                    if (i == population.active_count)
                    {
                        printf("   |\n");
                        printf("   | HISTORIC NOVELTY\n");
                    }

                    printf("%4d | %4d | %f | %3d\n",
                           i,
                           population.scout_id[i],
                           population.scout_novelty_score[i],
                           population.scout_generation[i]);
                }
            }
            else if (cmd_arg_count == 1)
            {
                size_t scout_id = atoi(cmd_args[0]);
                bool found_scout = false;
                for (size_t i = 0; i < population.count; i++)
                {
                    if (population.scout_id[i] != scout_id)
                        continue;

                    for (size_t s = 0; s < NUM_OF_STATISTICS; s++)
                        printf("%4d  %s\n", population.scout_stats[i].stat[s], stat_name_to_string((StatName)s));

                    found_scout = true;
                    break;
                }

                if (!found_scout)
                    printf("There is no scout in the population with ID %d.\n", scout_id);
            }
            else
            {
                printf("Usage: info\n");
                printf("       info <scout_id>\n");
            }
        }

        // COMMAND: save
        // Save the network parameters for a specific turtle
        else if (CMD_IS("save"))
        {
            if (cmd_arg_count == 1)
            {
                size_t scout_id = atoi(cmd_args[0]);
                bool found_scout = false;
                for (size_t i = 0; i < population.count; i++)
                {
                    if (population.scout_id[i] != scout_id)
                        continue;

                    dump_network_to_lua(population.scout_network[i]);
                    found_scout = true;
                    break;
                }

                if (!found_scout)
                    printf("There is no scout in the population with ID %d.\n", scout_id);
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