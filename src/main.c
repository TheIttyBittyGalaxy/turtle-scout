#include <float.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// TODO: Implement
typedef struct
{
    void *unimplemented;
} NetworkParameters;

typedef struct
{
    void *unimplemented;
} Stats;

typedef struct
{
    void *unimplemented;
} NetworkValues;

typedef struct
{
    void *unimplemented;
} World;

typedef struct
{
    void *unimplemented;
} Action;

// SCOUT POPULATION //
typedef struct
{
    size_t scout_count;
    size_t array_length;

    NetworkParameters *scout_parameters;
    Stats *scout_stats;
    double *scout_novelty_score;
    size_t *scout_generation;
} Population;

// TODO: Implement
void set_network_inputs(NetworkValues *values, const World world);
void evaluate_network(const NetworkParameters parameters, NetworkValues *values);
Action determine_network_action(network_values);
Stats update_world(World *world, const Action action);
void add_stats(Stats *stats, const Stats stats_delta);
double stats_distance(const Stats stats_one, const Stats stats_two);

// ITERATE TRAINING //
void iterate_training(Population *population)
{
    size_t scout_count = population->scout_count;
    NetworkParameters *scout_parameters = population->scout_parameters;
    Stats *scout_stats = population->scout_stats;
    double *scout_novelty_score = population->scout_novelty_score;
    size_t *scout_generation = population->scout_generation;

    // Generate world
    World world_blueprint;
    generate_world(&world_blueprint);
    World active_world;

    // Evaluate each scout
    NetworkValues network_values;
    for (size_t i = 0; i < scout_count; i++)
    {
        NetworkParameters parameters = scout_parameters[i];
        memcpy(&active_world, &world_blueprint, sizeof(World));
        for (size_t n = 0; n < 1000; n++)
        {
            set_network_inputs(&network_values, active_world);
            evaluate_network(parameters, &network_values);
            Action action = determine_network_action(network_values);
            Stats stats_delta = update_world(&active_world, action);
            add_stats(scout_stats + i, stats_delta);
        }
    }

    // Generate novelty scores
    for (size_t i = 0; i < scout_count; i++)
    {
        // Find the 8 scouts with the most similar stats and their distances
        double nearest_scouts[8];
        nearest_scouts[0] = DBL_MAX;
        nearest_scouts[1] = DBL_MAX;
        nearest_scouts[2] = DBL_MAX;
        nearest_scouts[3] = DBL_MAX;
        nearest_scouts[4] = DBL_MAX;
        nearest_scouts[5] = DBL_MAX;
        nearest_scouts[6] = DBL_MAX;
        nearest_scouts[7] = DBL_MAX;

        for (size_t j = 0; j < scout_count; j++)
        {
            if (i == j)
                continue;

            double dist = stats_distance(scout_stats[i], scout_stats[j]);
            for (size_t s = 0; s < 8; s++)
            {
                if (dist < nearest_scouts[s])
                {
                    for (size_t n = 7; n > s; n--)
                        nearest_scouts[n] = nearest_scouts[n - 1];
                    nearest_scouts[s] = dist;
                    break;
                }
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

    // TODO: Remove half the population with a bias towards removing scouts with a low novelty score.
    // TODO: Repopulate by duplicating surviving members of the population and adding mutations.
}

// MAIN FUNCTION //
#define CMD_CHAR_LIMIT 256

int main(int argc, char const *argv[])
{
    char cmd_buffer[CMD_CHAR_LIMIT];

    Population population;
    population.scout_count = 0;
    population.array_length = 128;
    population.scout_parameters = (NetworkParameters *)malloc(sizeof(NetworkParameters) * 128);
    population.scout_stats = (Stats *)malloc(sizeof(Stats) * 128);
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
            // TODO: Implement
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
            printf("| ACTIVE POPULATION\n");
            for (size_t i = 0; i < 64; i++)
            {
                printf("%02d | %f\t%d\n",
                       i,
                       population.scout_novelty_score[i],
                       population.scout_generation[i]);
            }

            printf("| HISTORIC NOVELTY\n");
            for (size_t i = 64; i < population.scout_count; i++)
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