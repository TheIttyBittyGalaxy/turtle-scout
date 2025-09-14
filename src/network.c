#include "network.h"

// RANDOMISE AND MUTATE NETWORK //

void randomise_network(Network *network)
{
    for (size_t i = 0; i < NUM_OF_NODES; i++)
        network->bias[i] = rand_normal();

    for (size_t i = 0; i < NUM_OF_NODES; i++)
        for (size_t j = 0; j < NUM_OF_NODES; j++)
            network->weight[i][j] = rand_normal();
}

void mutate_network(Network *network)
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
        MUTATE(network->bias[i])

    for (size_t i = 0; i < NUM_OF_NODES; i++)
        for (size_t j = 0; j < NUM_OF_NODES; j++)
            MUTATE(network->weight[i][j])

#undef MUTATE
}

// ACTIVATION //

// double activation(double x)
// {
//     if (x > 1)
//         return 1;
//     if (x < -1)
//         return -1;
//     return x;
// }

double activation(double x)
{
    if (x > 0)
        return 1;
    return 0;
}

// RESET VALUES //

void reset_network_values(const Network network, NetworkValues *value)
{
    for (size_t i = 0; i < NUM_OF_NODES; i++)
        (*value)[i] = activation(network.bias[i]);
}

// EVALUATE NETWORK //

void evaluate_network_values(const Network network, NetworkValues *value)
{
    NetworkValues result;

    for (size_t i = 0; i < NUM_OF_NODES; i++)
    {
        result[i] = network.bias[i];
        for (size_t j = 0; j < NUM_OF_NODES; j++)
            result[i] += (*value)[j] * network.weight[j][i];
    }

    for (size_t i = 0; i < NUM_OF_NODES; i++)
        (*value)[i] = activation(result[i]);
}

// SAVE NETWORK //

void save_network(const Network network)
{
    FILE *f = fopen("save/network.bytes", "wb");

    uint64_t count = NUM_OF_NODES;
    fwrite(&count, sizeof(uint64_t), 1, f);

    fwrite(network.bias, sizeof(double), NUM_OF_NODES, f);

    for (size_t i = 0; i < NUM_OF_NODES; ++i)
        fwrite(network.weight[i], sizeof(double), NUM_OF_NODES, f);

    fclose(f);
}
