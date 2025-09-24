// This file was generated automatically by generate.lua

#pragma once
#include "core.h"

#include "item.h"
#include "environment.h"
#include "network.h"
#include "statistics.h"

void set_network_inputs_for_item(NetworkValues *values, const Environment environment, size_t *next_node, Item item);
void perform_dig_action(Environment* environment, Statistics* stats, Item block);
void update_refuel_stat(Statistics* stats, Item item);
