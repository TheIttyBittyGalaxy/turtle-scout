#include "network.h"

void reset_network_values(NetworkValues *values, const NetworkParameters parameters)
{
    for (size_t i = 0; i < NUM_OF_NODES; i++)
        (*values)[i] = parameters.bias[i];
}

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
