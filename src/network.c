#include "network.h"

// RANDOMISE AND MUTATE NETWORK //

void randomise_network(Network *network)
{
    for (size_t i = 0; i < NUM_OF_NODES; i++)
    {
        for (size_t j = 0; j < NUM_OF_NODES; j++)
        {
            bool activate = false;
            bool inhibit = false;

            if ((rand() % 16) == 0)
                activate = true;
            else if ((rand() % 8) == 0)
                inhibit = true;

            network->activations[i][j] = activate;
            network->inhibitions[i][j] = inhibit;
        }
    }
}

void mutate_network(Network *network)
{
    for (size_t i = 0; i < NUM_OF_NODES; i++)
    {
        for (size_t j = 0; j < NUM_OF_NODES; j++)
        {
            if (rand() % 25 > 0)
                continue;

            bool activate = network->activations[i][j];
            bool inhibit = network->inhibitions[i][j];

            if (activate)
            {
                activate = false;
                inhibit = rand() % 3 == 0;
            }
            else if (inhibit)
            {
                activate = rand() % 3 == 0;
                inhibit = false;
            }
            else
            {
                activate = rand() % 2 == 0;
                inhibit = !activate;
            }

            network->activations[i][j] = activate;
            network->inhibitions[i][j] = inhibit;
        }
    }
}

// RESET VALUES //

void reset_network_values(const Network network, NetworkValues *value)
{
    for (size_t i = 0; i < NUM_OF_NODES; i++)
        (*value)[i] = false;
}

// EVALUATE NETWORK //

void evaluate_network_values(const Network network, NetworkValues *value)
{
    NetworkValues result;

    for (size_t i = 0; i < NUM_OF_NODES; i++)
    {
        result[i] = false;
        for (size_t j = 0; j < NUM_OF_NODES; j++)
        {
            if (!((*value)[j]))
                continue;

            if (network.inhibitions[j][i])
            {
                result[i] = false;
                break;
            }

            if (network.activations[j][i])
                result[i] = true;
        }
    }

    for (size_t i = 0; i < NUM_OF_NODES; i++)
        (*value)[i] = result[i];
}

// SAVE NETWORK //

void save_network(const Network network)
{
    FILE *f = fopen("save/network.bytes", "wb");

    uint64_t count = NUM_OF_NODES;
    fwrite(&count, sizeof(uint64_t), 1, f);

    for (size_t i = 0; i < NUM_OF_NODES; ++i)
        fwrite(network.activations[i], sizeof(bool), NUM_OF_NODES, f);

    for (size_t i = 0; i < NUM_OF_NODES; ++i)
        fwrite(network.inhibitions[i], sizeof(bool), NUM_OF_NODES, f);

    fclose(f);
}
