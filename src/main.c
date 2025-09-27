#include "core.h"

#include "action.h"
#include "environment.h"
#include "network.h"
#include "statistics.h"
#include "generated.h"

// WORLD GEN //

Environment standard_environment;

void generate_segment(Segment *segment, int grid_x, int grid_y, int grid_z)
{
    if (grid_y > 0)
    {
        for (size_t sx = 0; sx < 16; sx++)
            for (size_t sy = 0; sy < 16; sy++)
                for (size_t sz = 0; sz < 16; sz++)
                    segment->block[sx][sy][sz] = AIR;
    }
    else
    {
        for (size_t sx = 0; sx < 16; sx++)
            for (size_t sy = 0; sy < 16; sy++)
                for (size_t sz = 0; sz < 16; sz++)
                    segment->block[sx][sy][sz] = STONE;

        if (grid_y == 0)
        {
            for (size_t sx = 0; sx < 16; sx++)
                for (size_t sz = 0; sz < 16; sz++)
                {
                    segment->block[sx][15][sz] = GRASS_BLOCK;
                    segment->block[sx][14][sz] = DIRT;
                    segment->block[sx][13][sz] = DIRT;
                    segment->block[sx][12][sz] = DIRT;
                }
        }
    }
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

    Item block_front = get_block_in_front_of_scout(environment);
    set_network_inputs_for_item(values, environment, &next_node, block_front);

    Item block_above = get_block_above_scout(environment);
    set_network_inputs_for_item(values, environment, &next_node, block_above);

    Item block_below = get_block_below_scout(environment);
    set_network_inputs_for_item(values, environment, &next_node, block_below);

    // TODO: What is the best way to enter the qty into the network?
    InventorySlot slot = environment.scout.inventory[environment.scout.selected_inventory_slot];
    Item selected = (slot.qty > 0) ? slot.item : AIR;
    set_network_inputs_for_item(values, environment, &next_node, selected);
}

bool perform_refuel(Environment *environment, Statistics *stats)
{
    InventorySlot *slot = &environment->scout.inventory[environment->scout.selected_inventory_slot];
    if (slot->qty == 0)
        return false;

    Item fuel = slot->item;
    size_t fuel_value = fuel_value_of(fuel);
    if (fuel_value_of(slot->item) == 0)
        return false;

    decrement_increment_slot(slot);
    environment->scout.fuel += fuel_value;
    update_refuel_stat(stats, fuel);
    return true;
}

bool perform_select_first_slot(Environment *environment, Statistics *stats)
{
    environment->scout.selected_inventory_slot = 0;
    return true;
}

bool perform_select_next_slot(Environment *environment, Statistics *stats)
{
    size_t i = environment->scout.selected_inventory_slot;
    environment->scout.selected_inventory_slot = (i + 1) % 16;
    return true;
}

// FIXME: Don't have a bool for up and a bool for down, just have a sensible enum
bool perform_dig(Environment *environment, Statistics *stats, bool up, bool down)
{
    int x = environment->scout.x;
    int y = environment->scout.y;
    int z = environment->scout.z;

    if (up)
    {
        y++;
    }
    else if (down)
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
    update_dig_action(environment, stats, block);
    return true;
}

// FIXME: Don't have a bool for up and a bool for down, just have a sensible enum
bool perform_move(Environment *environment, Statistics *stats, bool up, bool down)
{
    if (environment->scout.fuel == 0)
        return false;

    if (up && get_block_above_scout(*environment) == AIR)
    {
        environment->scout.y++;
    }
    else if (down && get_block_below_scout(*environment) == AIR)
    {
        environment->scout.y--;
    }
    else if (!up && !down && get_block_in_front_of_scout(*environment) == AIR)
    {
        environment->scout.x += x_offset_of(environment->scout.facing);
        environment->scout.z += z_offset_of(environment->scout.facing);
    }
    else
    {
        return false;
    }

    stats->stat[MOVED]++;
    environment->scout.fuel--;
    return true;
}

bool perform_turn_left(Environment *environment, Statistics *stats)
{
    environment->scout.facing = left_of(environment->scout.facing);
    return true;
}

bool perform_turn_right(Environment *environment, Statistics *stats)
{
    environment->scout.facing = right_of(environment->scout.facing);
    return true;
}

ActionOutcomes perform_network_actions(Environment *environment, Statistics *stats, const NetworkValues network_values)
{
    ActionOutcomes outcome;

    for (size_t i = 0; i < NUM_OF_ACTIONS; i++)
        outcome.action[i] = OUTCOME_NONE;

#define do_action(action_name, call)                                              \
    if (get_network_value(network_values, NUM_OF_NODES - (int)(action_name) - 1)) \
    {                                                                             \
        bool success = call;                                                      \
        outcome.action[action_name] = success ? OUTCOME_PASS : OUTCOME_FAIL;      \
    }

    // REFUEL
    do_action(REFUEL, perform_refuel(environment, stats));

    // INVENTORY
    do_action(SELECT_FIRST_SLOT, perform_select_first_slot(environment, stats)) // comments to stop auto-formatter putting this all on one line
        else do_action(SELECT_NEXT_SLOT, perform_select_next_slot(environment, stats));

    // DIG
    do_action(DIG_FORWARD, perform_dig(environment, stats, false, false));

    do_action(DIG_UP, perform_dig(environment, stats, true, false));

    do_action(DIG_DOWN, perform_dig(environment, stats, false, true));

    // MOVE / TURN
    do_action(MOVE_FORWARD, perform_move(environment, stats, false, false))      //
        else do_action(MOVE_UP, perform_move(environment, stats, true, false))   //
        else do_action(MOVE_DOWN, perform_move(environment, stats, false, true)) //
        else do_action(TURN_LEFT, perform_turn_left(environment, stats))         //
        else do_action(TURN_RIGHT, perform_turn_right(environment, stats));

    return outcome;

#undef NODE_ACTIVATED
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
    perform_network_actions(&simulation_environment, &simulation_statistics, simulation_network_values);
    simulation_iteration++;
}

inline void iterate_simulation_and_log(const Network network)
{
    Item front = get_block_in_front_of_scout(simulation_environment);
    Item above = get_block_above_scout(simulation_environment);
    Item below = get_block_below_scout(simulation_environment);

    set_network_inputs(&simulation_network_values, simulation_environment);
    evaluate_network_values(network, &simulation_network_values);
    ActionOutcomes outcome = perform_network_actions(&simulation_environment, &simulation_statistics, simulation_network_values);

    // Action log
    fprintf(simulation_action_log,
            "%d,%s,%s,%s",
            simulation_iteration,
            item_to_string(front),
            item_to_string(above),
            item_to_string(below));

    for (size_t i = 0; i < NUM_OF_ACTIONS; i++)
        fprintf(simulation_action_log, ",%s", outcome_as_string(outcome.action[i]));

    fprintf(simulation_action_log, ",%d\n", simulation_environment.scout.fuel);

    // Network log
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
    fprintf(simulation_action_log, "#,Front,Above,Below");
    for (size_t i = 0; i < NUM_OF_ACTIONS; i++)
        fprintf(simulation_action_log, ",%s", action_as_string((Action)i));
    fprintf(simulation_action_log, ",Fuel\n");

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

bool is_historic_scout(Population *population, size_t scout_id)
{
    for (size_t i = population->active_count; i < population->count; i++)
    {
        if (population->scout_id[i] == scout_id)
            return true;
    }

    return false;
}

void iterate_training(Population *population)
{
    size_t population_count = population->count;
    size_t active_count = population->active_count;

    size_t *scout_id = population->scout_id;
    size_t *scout_generation = population->scout_generation;
    Network *scout_network = population->scout_network;
    Statistics *scout_stats = population->scout_stats;
    double *scout_novelty_score = population->scout_novelty_score;

    // Repopulate scouts, with a slight bias towards more novel scouts repopulating
    for (size_t child = population->active_count - 1; child > 0; child--)
    {
        if (population->scout_id[child] != 0)
            continue;

        size_t parent = rand() % child;

        population->scout_id[child] = population->next_id++;
        population->scout_generation[child] = population->scout_generation[parent] + 1;
        population->scout_network[child] = population->scout_network[parent];
        mutate_network(&population->scout_network[child]);
    }

    // Evaluate each scout
    for (size_t i = 0; i < active_count; i++)
    {
        initialise_simulation(scout_network[i], standard_environment);
        for (size_t n = 0; n < 128; n++)
            iterate_simulation(scout_network[i]);
        scout_stats[i] = simulation_statistics;
    }

// Generate novelty scores
#define N_NEAREST_NEIGHBOURS 32
    for (size_t i = 0; i < active_count; i++)
    {
        // Find the scouts with the most similar stats and track how similar they are
        double nearest_scouts[N_NEAREST_NEIGHBOURS];
        for (size_t j = 0; j < N_NEAREST_NEIGHBOURS; j++)
            nearest_scouts[j] = DBL_MAX;

        for (size_t j = 0; j < population_count; j++)
        {
            if (i == j)
                continue;

            double dist = novelty_distance(scout_stats[i], scout_stats[j]);

            for (size_t s = 0; s < N_NEAREST_NEIGHBOURS; s++)
            {
                if (dist > nearest_scouts[s])
                    continue;

                for (size_t n = N_NEAREST_NEIGHBOURS - 1; n > s; n--)
                    nearest_scouts[n] = nearest_scouts[n - 1];

                nearest_scouts[s] = dist;
                break;
            }
        }

        // This scout's novelty score is the average of the closest distances
        double score = 0;
        for (size_t j = 0; j < N_NEAREST_NEIGHBOURS; j++)
            score += nearest_scouts[j];
        score /= N_NEAREST_NEIGHBOURS;

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

    // Add the most novel scout to the archive
    // TODO: What is the best way to calibrate the novelty threshold?
    if (population->scout_novelty_score[0] > 2 && !is_historic_scout(population, population->scout_id[0]))
    {
        // Expand the population array if required
        if (population->capacity == population->count)
        {
            population->capacity += 128;
            REALLOCATE(population->scout_id, population->capacity, size_t);
            REALLOCATE(population->scout_generation, population->capacity, size_t);
            REALLOCATE(population->scout_network, population->capacity, Network);
            REALLOCATE(population->scout_novelty_score, population->capacity, double);
            REALLOCATE(population->scout_stats, population->capacity, Statistics);
        }

        // Add the novel scout to the historic population
        population->scout_id[population->count] = population->scout_id[0];
        population->scout_generation[population->count] = population->scout_generation[0];
        population->scout_network[population->count] = population->scout_network[0];
        population->scout_stats[population->count] = population->scout_stats[0];
        population->scout_novelty_score[population->count] = population->scout_novelty_score[0];
        population->count++;
    }

    // Eliminate scouts with lower novelty, with some random variation

    // FIXME: Refactor the program so that we purge and repopulate at the start of every iteration except the first.

    // FIXME: This method of eliminating scouts gives random variation while always eliminating 32 scouts.
    //        However, it is means there is symmetry between the eliminated and non-eliminated scouts, which
    //        is a bit strange. e.g. for scouts at index 31 and 32, always exactly one will be eliminated.

    size_t elimination_target = active_count / 2;
    for (size_t i = 0; i < elimination_target; i++)
    {
        double p = (i + 1) / (double)active_count;
        double risk = (i > 0) // "Risk of a more novel scout being eliminated"
                          ? 1.0 / (1.0 + exp(8 - 16 * p))
                          : 0; // Most novel scout needs immunity or else the repopulation method does not work

        double r = rand() / (double)(RAND_MAX - 1);
        if (r < risk)
            population->scout_id[i] = 0;
        else
            population->scout_id[population->active_count - 1 - i] = 0;
    }
}

// MAIN FUNCTION //

size_t find_scout_with_id(const Population population, size_t scout_id)
{
    if (scout_id == 0)
        return population.count;

    for (size_t i = 0; i < population.count; i++)
    {
        if (population.scout_id[i] == scout_id)
        {
            return i;
        }
    }

    return population.count;
}

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

    size_t i = 0;
    for (int gx = -1; gx <= 1; gx++)
        for (int gy = -1; gy <= 1; gy++)
            for (int gz = -1; gz <= 1; gz++)
            {
                SegmentGet result = get_or_create_segment(&standard_environment, gx, gy, gz);
                if (result.new_segment)
                    generate_segment(result.segment, gx, gy, gz);
            }

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
    population.next_id = 1; // 0 indicates NULL

    ALLOCATE(population.scout_id, population.capacity, size_t);
    ALLOCATE(population.scout_generation, population.capacity, size_t);
    ALLOCATE(population.scout_network, population.capacity, Network);
    ALLOCATE(population.scout_stats, population.capacity, Statistics);
    ALLOCATE(population.scout_novelty_score, population.capacity, double);

    // Generate random initial population
    printf("Creating initial population.\n");
    for (size_t i = 0; i < population.active_count; i++)
    {
        population.scout_id[i] = population.next_id++;
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
                size_t scout_index = find_scout_with_id(population, scout_id);

                if (scout_index < population.count)
                {
                    initialise_simulation(population.scout_network[scout_index], standard_environment);
                    open_simulation_logs();
                    for (size_t n = 0; n < 128; n++)
                        iterate_simulation_and_log(population.scout_network[scout_index]);
                    close_simulation_logs();
                }
                else
                {
                    printf("There is no scout in the population with ID %d.\n", scout_id);
                }
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
                printf("Rank |    ID | Score    | Gen\n");
                printf("---- | ----- | -------- | ---\n");
                for (size_t i = 0; i < population.count; i++)
                {
                    if (i == population.active_count)
                    {
                        printf("     |\n");
                        printf("     | HISTORIC NOVELTY\n");
                    }

                    if (population.scout_id[i] == 0)
                        printf("%4d |       |          |    \n", i);
                    else
                        printf("%4d | %5d | %f | %3d\n",
                               i,
                               population.scout_id[i],
                               population.scout_novelty_score[i],
                               population.scout_generation[i]);
                }
            }
            else if (cmd_arg_count == 1)
            {
                size_t scout_id = atoi(cmd_args[0]);
                size_t scout_index = find_scout_with_id(population, scout_id);

                if (scout_index < population.count)
                {
                    Statistics stats = population.scout_stats[scout_index];
                    for (size_t s = 0; s < NUM_OF_STATISTICS; s++)
                        printf("%4d  %s\n", stats.stat[s], stat_name_to_string((StatName)s));
                }
                else
                {
                    printf("There is no scout in the population with ID %d.\n", scout_id);
                }
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
                size_t scout_index = find_scout_with_id(population, scout_id);

                if (scout_index < population.count)
                {
                    save_network(population.scout_network[scout_index]);
                }
                else
                {
                    printf("There is no scout in the population with ID %d.\n", scout_id);
                }
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