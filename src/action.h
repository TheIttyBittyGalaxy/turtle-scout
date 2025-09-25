#pragma once
#include "core.h"

#define NUM_OF_ACTIONS 11

typedef enum
{
    REFUEL,

    SELECT_FIRST_SLOT,
    SELECT_NEXT_SLOT,

    DIG_FORWARD,
    DIG_UP,
    DIG_DOWN,

    MOVE_FORWARD,
    MOVE_UP,
    MOVE_DOWN,

    TURN_LEFT,
    TURN_RIGHT,
} Action;

typedef enum
{
    OUTCOME_NONE,
    OUTCOME_FAIL,
    OUTCOME_PASS,
} Outcome;

typedef struct
{
    Outcome action[NUM_OF_ACTIONS];
} ActionOutcomes;

const char *action_as_string(const Action action);
const char *outcome_as_string(const Outcome outcome);
