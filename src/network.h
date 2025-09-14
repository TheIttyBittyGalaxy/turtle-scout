#pragma once
#include "core.h"

#define NUM_OF_NODES 64

typedef double NetworkValues[NUM_OF_NODES];

typedef struct
{
    double bias[NUM_OF_NODES];
    double weight[NUM_OF_NODES][NUM_OF_NODES]; // weight[a][b] is connection from a to b
} Network;

void randomise_network(Network *network);
void mutate_network(Network *network);

void reset_network_values(const Network network, NetworkValues *value);
void evaluate_network_values(const Network network, NetworkValues *value);

void save_network(const Network network);