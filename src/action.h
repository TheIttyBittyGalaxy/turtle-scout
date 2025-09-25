#pragma once
#include "core.h"

typedef enum
{
    NO_INVENTORY_ACTION,

    SELECT_FIRST_SLOT,
    SELECT_NEXT_SLOT,
} InventoryAction;

typedef enum
{
    NO_MOVE_ACTION,

    MOVE_FORWARD,
    MOVE_UP,
    MOVE_DOWN,

    TURN_LEFT,
    TURN_RIGHT,
} MoveAction;

typedef struct
{
    bool refuel_action;
    bool refuel_success;

    InventoryAction inventory_action;
    bool inventory_success;

    bool dig_action;
    bool dig_success;

    bool dig_up_action;
    bool dig_up_success;

    bool dig_down_action;
    bool dig_down_success;

    MoveAction move_action;
    bool move_success;
} ActionResults;

const char *inventory_action_as_string(const InventoryAction action);
const char *move_action_as_string(const MoveAction action);