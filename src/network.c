#include "network.h"

#define BITMASK_FOR(n) (1ULL << n)

// RANDOMISE AND MUTATE NETWORK //

void randomise_network(Network *network)
{
    for (size_t i = 0; i < NUM_OF_NODES; i++)
    {
        for (size_t j = 0; j < NUM_OF_CLUSTERS; j++)
        {
            uint64_t cluster_activations = 0;
            uint64_t cluster_inhibitions = 0;

            for (size_t n = 0; n < NODES_IN_CLUSTER; n++)
            {
                if ((rand() % 16) == 0)
                    cluster_activations |= BITMASK_FOR(n);
                else if ((rand() % 8) == 0)
                    cluster_inhibitions |= BITMASK_FOR(n);
            }

            network->activations[i][j] = cluster_activations;
            network->inhibitions[i][j] = cluster_inhibitions;
        }
    }
}

void mutate_network(Network *network)
{
    for (size_t i = 0; i < NUM_OF_NODES; i++)
    {
        for (size_t j = 0; j < NUM_OF_CLUSTERS; j++)
        {
            for (size_t n = 0; n < NODES_IN_CLUSTER; n++)
            {
                if (rand() % 25 > 0)
                    continue;

                bool activate = network->activations[i][j] & BITMASK_FOR(n);
                bool inhibit = network->inhibitions[i][j] & BITMASK_FOR(n);

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

                if (activate)
                    network->activations[i][j] ^= BITMASK_FOR(n);
                if (inhibit)
                    network->inhibitions[i][j] ^= BITMASK_FOR(n);
            }
        }
    }
}

// RESET VALUES //

void reset_network_values(const Network network, NetworkValues *values)
{
    for (size_t i = 0; i < NUM_OF_CLUSTERS; i++)
        (*values)[i] = 0;
}

// EVALUATE NETWORK //

void evaluate_network_values(const Network network, NetworkValues *values)
{
    NetworkValues result;

    for (size_t i = 0; i < NUM_OF_CLUSTERS; i++)
    {
        uint64_t cluster_values = 0;

        for (size_t n = 0; n < NODES_IN_CLUSTER; n++)
        {
            size_t node_index = i * NODES_IN_CLUSTER + n;
            bool node_is_active = false;
            for (size_t j = 0; j < NUM_OF_CLUSTERS; j++)
            {
                uint64_t inhibitions = (*values)[j] & network.inhibitions[node_index][j];
                if (inhibitions != 0)
                {
                    node_is_active = false;
                    break;
                }

                uint64_t activations = (*values)[j] & network.activations[node_index][j];
                if (activations != 0)
                {
                    node_is_active = true;
                }
            }

            if (node_is_active)
                cluster_values |= BITMASK_FOR(n);
        }

        result[i] = cluster_values;
    }

    for (size_t i = 0; i < NUM_OF_CLUSTERS; i++)
        (*values)[i] = result[i];
}

void set_network_value(NetworkValues *values, size_t node_index, bool node_value)
{
    size_t i = node_index / NODES_IN_CLUSTER;
    size_t n = node_index % NODES_IN_CLUSTER;
    if (node_value)
        (*values)[i] |= BITMASK_FOR(n);
    else
        (*values)[i] &= ~BITMASK_FOR(n);
}

bool get_network_value(const NetworkValues values, size_t node_index)
{
    size_t i = node_index / NODES_IN_CLUSTER;
    size_t n = node_index % NODES_IN_CLUSTER;
    return values[i] & BITMASK_FOR(n);
}

// SAVE NETWORK //

void save_network(const Network network)
{
    FILE *f = fopen("save/network.bytes", "wb");

    // FIXME: Save into a format the lua script can understand

    // uint64_t count = NUM_OF_NODES;
    // fwrite(&count, sizeof(uint64_t), 1, f);

    // for (size_t i = 0; i < NUM_OF_NODES; ++i)
    //     fwrite(network.activations[i], sizeof(bool), NUM_OF_NODES, f);

    // for (size_t i = 0; i < NUM_OF_NODES; ++i)
    //     fwrite(network.inhibitions[i], sizeof(bool), NUM_OF_NODES, f);

    fclose(f);
}

#undef BITMASK_FOR