#include "action.h"

const char *inventory_action_as_string(const InventoryAction action)
{
    switch (action)
    {
    case NO_INVENTORY_ACTION: return "NO_INVENTORY_ACTION";
    case SELECT_FIRST_SLOT:   return "SELECT_FIRST_SLOT";
    case SELECT_NEXT_SLOT:    return "SELECT_NEXT_SLOT";
    }

    UNREACHABLE;
}

const char *move_action_as_string(const MoveAction action)
{
    switch (action)
    {
    case NO_MOVE_ACTION: return "NO_MOVE_ACTION";

    case MOVE_FORWARD:   return "MOVE_FORWARD";
    case MOVE_UP:        return "MOVE_UP";
    case MOVE_DOWN:      return "MOVE_DOWN";

    case TURN_LEFT:      return "TURN_LEFT";
    case TURN_RIGHT:     return "TURN_RIGHT";
    }

    UNREACHABLE;
}