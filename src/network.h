#pragma once
#include "core.h"

#define NUM_OF_NODES 64

typedef double NetworkValues[NUM_OF_NODES];

typedef struct
{
    double bias[NUM_OF_NODES];
    double weight[NUM_OF_NODES][NUM_OF_NODES]; // weight[a][b] is connection from a to b
} NetworkParameters;

void reset_network_values(NetworkValues *values, const NetworkParameters parameters);
void evaluate_network(const NetworkParameters parameter, NetworkValues *value);
void randomise_scout_parameters(NetworkParameters *parameters);
void mutate_parameters(NetworkParameters *parameters);
void save_parameters_to_lua(const NetworkParameters parameters);