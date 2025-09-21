// This file was generated automatically by generate.lua

#pragma once
#include "core.h"

#include "block.h"
#include "environment.h"
#include "network.h"
#include "statistics.h"

void set_network_block_inputs(NetworkValues *values, const Environment environment, size_t *next_node, Item block);
void perform_dig_action(Environment* environment, Statistics* stats, Item block);