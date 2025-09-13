#include <float.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// NETWORK VALUES & PARAMETERS //

#define NUM_OF_NODES 64

typedef int NetworkValues[NUM_OF_NODES];

typedef struct
{
    double bias[NUM_OF_NODES];
    double weight[NUM_OF_NODES][NUM_OF_NODES];
} NetworkParameters;

// TODO: Implement

typedef struct
{
    void *unimplemented;
} Environment;

typedef struct
{
    void *unimplemented;
} Action;

// STATISTICS //

#define NUM_OF_STATISTICS 8

typedef int Statistics[NUM_OF_STATISTICS];

void initialise_scout_stats(Statistics *stats)
{
    for (size_t i = 0; i < NUM_OF_STATISTICS; i++)
        (*stats)[i] = 0;
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

void generate_environment(Environment *environment)
{
    // TODO: Implement
}

void set_network_inputs(NetworkValues *values, const Environment world)
{
    // TODO: Implement
}

void evaluate_network(const NetworkParameters parameters, NetworkValues *values)
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

void add_stats(Statistics *stats, const Statistics stats_delta)
{
    // TODO: Implement
}

// RANDOMISE NETWORK PARAMETER

double rand_param()
{
    double a = (double)rand() / RAND_MAX;
    double b = (double)rand() / RAND_MAX;
    return a + b - 1;
}

void randomise_scout_parameters(NetworkParameters *parameters)
{
    for (size_t i = 0; i < NUM_OF_NODES; i++)
        parameters->bias[i] = rand_param();

    for (size_t i = 0; i < NUM_OF_NODES; i++)
        for (size_t j = 0; j < NUM_OF_NODES; j++)
            parameters->weight[i][j] = rand_param();
}

// CALCULATE NOVELTY DISTANCE //

// NOTE: In the interests of speed, I'm using taxi-cab distance here (i.e. I'm not square rooting anything)
//       For this purpose, I think that's fine?
double novelty_distance(const Statistics a, const Statistics b)
{
    double score = 0;
    for (size_t i = 0; i < NUM_OF_STATISTICS; i++)
        score += abs(a[i] - b[i]);
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
    generate_environment(&environment);

    // Evaluate each scout
    NetworkValues network_values;
    for (size_t i = 0; i < active_count; i++)
    {
        NetworkParameters parameters = scout_parameters[i];

        // TODO: Ensure this creates a deep copy of the environment
        memcpy(&copy_of_environment, &environment, sizeof(Environment));

        initialise_scout_stats(scout_stats + i);

        for (size_t i = 0; i < NUM_OF_NODES; i++)
            network_values[i] = parameters.bias[i];

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

    // TODO: Remove half the active population with a bias towards removing scouts with a low novelty score.
    // TODO: Repopulate by duplicating surviving members of the population and adding mutations.
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
            printf("| ACTIVE POPULATION\n");
            for (size_t i = 0; i < population.active_count; i++)
            {
                printf("%02d | %f\t%d\n",
                       i,
                       population.scout_novelty_score[i],
                       population.scout_generation[i]);
            }

            printf("| HISTORIC NOVELTY\n");
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