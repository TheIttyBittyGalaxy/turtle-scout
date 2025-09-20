#pragma once
#include "core.h"

#define NUM_OF_NODES 64

typedef bool NetworkValues[NUM_OF_NODES];

typedef struct
{
    bool activations[NUM_OF_NODES][NUM_OF_NODES];
    bool inhibitions[NUM_OF_NODES][NUM_OF_NODES];
} Network;

void randomise_network(Network *network);
void mutate_network(Network *network);

void reset_network_values(const Network network, NetworkValues *value);
void evaluate_network_values(const Network network, NetworkValues *value);

void save_network(const Network network);