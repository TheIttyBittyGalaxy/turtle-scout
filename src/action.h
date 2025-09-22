#pragma once
#include "core.h"

#define NUM_OF_ACTION 11

typedef enum
{
    IDLE,

    TURN_LEFT,
    TURN_RIGHT,

    MOVE,
    MOVE_UP,
    MOVE_DOWN,

    DIG,
    DIG_UP,
    DIG_DOWN,

    REFUEL,

    SELECT_FIRST_SLOT,
    SELECT_NEXT_SLOT,
} Action;

const char *action_as_string(const Action action);
bool is_move_action(Action action);
bool is_dig_action(Action action);
bool is_up_action(Action action);
bool is_down_action(Action action);
