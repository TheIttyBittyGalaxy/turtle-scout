#pragma once
#include "core.h"

#define NODES_IN_CLUSTER 64
#define NUM_OF_CLUSTERS 1
#define NUM_OF_NODES (NUM_OF_CLUSTERS * NODES_IN_CLUSTER)

typedef uint64_t NetworkValues[NUM_OF_CLUSTERS];

typedef struct
{
    uint64_t activations[NUM_OF_NODES][NUM_OF_CLUSTERS];
    uint64_t inhibitions[NUM_OF_NODES][NUM_OF_CLUSTERS];
} Network;

void randomise_network(Network *network);
void mutate_network(Network *network);

void reset_network_values(const Network network, NetworkValues *values);
void evaluate_network_values(const Network network, NetworkValues *values);
void set_network_value(NetworkValues *values, size_t node_index, bool node_value);
bool get_network_value(const NetworkValues values, size_t node_index);

void save_network(const Network network);
