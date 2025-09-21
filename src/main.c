#include "core.h"

#include "action.h"
#include "environment.h"
#include "network.h"
#include "statistics.h"
#include "generated.h"

// WORLD GEN //

Environment standard_environment;

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

inline void set_network_inputs(NetworkValues *values, const Environment environment)
{
    size_t next_node = 0;
    set_network_value(values, next_node++, true);
    set_network_block_inputs(values, environment, &next_node, get_block_in_front_of_scout(environment));
    set_network_block_inputs(values, environment, &next_node, get_block_above_scout(environment));
    set_network_block_inputs(values, environment, &next_node, get_block_below_scout(environment));
}

Action determine_network_action(const NetworkValues network_values)
{
    // TODO: Select a random active node at random(???)
    for (size_t i = 1; i < NUM_OF_ACTION; i++)
    {
        size_t n = NUM_OF_NODES - i;
        if (get_network_value(network_values, n))
            return (Action)i;
    }

    return IDLE;
}

bool perform_action(Environment *environment, const Action action, Statistics *stats)
{
    // IDLE
    if (action == IDLE)
        return true;

    // TURN
    if (action == TURN_LEFT)
    {
        environment->scout.facing = left_of(environment->scout.facing);
        return true;
    }

    if (action == TURN_RIGHT)
    {
        environment->scout.facing = right_of(environment->scout.facing);
        return true;
    }

    // MOVE
    if (is_move_action(action))
    {
        if (action == MOVE_UP && get_block_above_scout(*environment) == AIR)
        {
            environment->scout.y++;
        }
        else if (action == MOVE_DOWN && get_block_below_scout(*environment) == AIR)
        {
            environment->scout.y--;
        }
        else if (action == MOVE && get_block_in_front_of_scout(*environment) == AIR)
        {
            environment->scout.x += x_offset_of(environment->scout.facing);
            environment->scout.z += z_offset_of(environment->scout.facing);
        }
        else
        {
            return false; // Could not move
        }

        stats->stat[MOVED]++;

        return true;
    }

    // DIG
    if (is_dig_action(action))
    {
        int x = environment->scout.x;
        int y = environment->scout.y;
        int z = environment->scout.z;

        if (action == DIG_UP)
        {
            y++;
        }
        else if (action == DIG_DOWN)
        {
            y--;
        }
        else
        {
            x += x_offset_of(environment->scout.facing);
            z += z_offset_of(environment->scout.facing);
        }

        Item block = get_block(*environment, x, y, z);
        if (block == AIR)
            return false;

        set_block(environment, x, y, z, AIR);
        perform_dig_action(environment, stats, block);
        return true;
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

// SCOUT SIMULATION //

NetworkValues simulation_network_values;
Environment simulation_environment;
Statistics simulation_statistics;
size_t simulation_iteration;

FILE *simulation_action_log = NULL;
FILE *simulation_network_log = NULL;

void initialise_simulation(const Network network, const Environment environment)
{
    reset_network_values(network, &simulation_network_values);
    copy_environment(environment, &simulation_environment);
    init_scout_stats(&simulation_statistics);
    simulation_iteration = 0;
}

inline void iterate_simulation(const Network network)
{
    set_network_inputs(&simulation_network_values, simulation_environment);
    evaluate_network_values(network, &simulation_network_values);
    Action action = determine_network_action(simulation_network_values);
    perform_action(&simulation_environment, action, &simulation_statistics);
    simulation_iteration++;
}

inline void iterate_simulation_and_log(const Network network)
{
    Item front = get_block_in_front_of_scout(simulation_environment);
    Item above = get_block_above_scout(simulation_environment);
    Item below = get_block_below_scout(simulation_environment);

    set_network_inputs(&simulation_network_values, simulation_environment);
    evaluate_network_values(network, &simulation_network_values);
    Action action = determine_network_action(simulation_network_values);
    bool success = perform_action(&simulation_environment, action, &simulation_statistics);

    fprintf(simulation_action_log,
            "%d,%s,%s,%s,%s,%s\n",
            simulation_iteration,
            item_to_string(front),
            item_to_string(above),
            item_to_string(below),
            action_as_string(action),
            success ? "true" : "false");

    set_network_inputs(&simulation_network_values, simulation_environment);
    fprintf(simulation_network_log, "%d", get_network_value(simulation_network_values, 0) ? 1 : 0);
    for (size_t i = 1; i < NUM_OF_NODES; i++)
        fprintf(simulation_network_log, ",%d", get_network_value(simulation_network_values, i) ? 1 : 0);
    fprintf(simulation_network_log, "\n");

    simulation_iteration++;
}

void open_simulation_logs()
{
    simulation_action_log = fopen("export/sim_action_log.csv", "w");
    fprintf(simulation_action_log, "#,Front,Above,Below,Action,Success\n");

    simulation_network_log = fopen("export/sim_network_log.csv", "w");
    fprintf(simulation_network_log, "%d", get_network_value(simulation_network_values, 0) ? 1 : 0);
    for (size_t i = 1; i < NUM_OF_NODES; i++)
        fprintf(simulation_network_log, ",%d", get_network_value(simulation_network_values, i) ? 1 : 0);
    fprintf(simulation_network_log, "\n");
}

void close_simulation_logs()
{
    fclose(simulation_action_log);
    fclose(simulation_network_log);
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

    // Evaluate each scout
    for (size_t i = 0; i < active_count; i++)
    {
        initialise_simulation(scout_network[i], standard_environment);
        for (size_t n = 0; n < 128; n++)
            iterate_simulation(scout_network[i]);
        scout_stats[i] = simulation_statistics;
    }

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

    // TODO: Add scouts with a particularly high novelty into the historic population
    // NOTE: I'm not sure the best way to go about this, consult with the paper!

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
    // TODO: Allow user to specify the seed
    printf("Seed 42.\n");
    srand(42);

    // Generate standard environment
    // TODO: Create a unique environment for each training iteration

    init_environment(&standard_environment);

    standard_environment.scout.x = 0;
    standard_environment.scout.y = 16;
    standard_environment.scout.z = 0;
    standard_environment.scout.facing = EAST;

    standard_environment.capacity = 27;
    standard_environment.count = 27;
    standard_environment.segment = (Segment *)malloc(27 * sizeof(Segment));

    size_t i = 0;
    for (int gx = -1; gx <= 1; gx++)
        for (int gy = -1; gy <= 1; gy++)
            for (int gz = -1; gz <= 1; gz++)
                standard_environment.segment[i++] = generate_segment(gx, gy, gz);

    set_block(&standard_environment, 1, 15, 0, DIRT);
    for (int y = 16; y <= 22; y++)
    {
        set_block(&standard_environment, 1, y, 0, OAK_LOG);
        if (y > 17)
        {
            set_block(&standard_environment, 2, y, 0, OAK_LEAVES);
            set_block(&standard_environment, 0, y, 0, OAK_LEAVES);
            set_block(&standard_environment, 1, y, 1, OAK_LEAVES);
            set_block(&standard_environment, 1, y, -1, OAK_LEAVES);
        }
    }

    dump_environment(standard_environment);

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
    char cmd_buffer[CMD_CHAR_LIMIT];
    char *cmd_args[8];
    size_t cmd_arg_count;

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

        // COMMAND: sim
        // Simulate a scout and log the outcome
        else if (CMD_IS("sim"))
        {
            if (cmd_arg_count == 1)
            {
                size_t scout_id = atoi(cmd_args[0]);
                bool found_scout = false;
                for (size_t i = 0; i < population.count; i++)
                {
                    if (population.scout_id[i] != scout_id)
                        continue;

                    initialise_simulation(population.scout_network[i], standard_environment);

                    open_simulation_logs();
                    for (size_t n = 0; n < 128; n++)
                        iterate_simulation_and_log(population.scout_network[i]);
                    close_simulation_logs();

                    found_scout = true;
                    break;
                }

                if (!found_scout)
                    printf("There is no scout in the population with ID %d.\n", scout_id);
            }
            else
            {
                printf("Usage: sim <scout_id>\n");
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

                    save_network(population.scout_network[i]);
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