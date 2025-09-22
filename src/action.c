#include "action.h"

const char *action_as_string(const Action action)
{
    if (action == IDLE)
        return "IDLE";

    if (action == TURN_LEFT)
        return "TURN_LEFT";
    if (action == TURN_RIGHT)
        return "TURN_RIGHT";

    if (action == MOVE)
        return "MOVE";
    if (action == MOVE_UP)
        return "MOVE_UP";
    if (action == MOVE_DOWN)
        return "MOVE_DOWN";

    if (action == DIG)
        return "DIG";
    if (action == DIG_UP)
        return "DIG_UP";
    if (action == DIG_DOWN)
        return "DIG_DOWN";

    if (action == REFUEL)
        return "REFUEL";

    if (action == SELECT_FIRST_SLOT)
        return "SELECT_FIRST_SLOT";
    if (action == SELECT_NEXT_SLOT)
        return "SELECT_NEXT_SLOT";

    UNREACHABLE;
}

bool is_move_action(Action action)
{
    return action == MOVE ||
           action == MOVE_UP ||
           action == MOVE_DOWN;
}

bool is_dig_action(Action action)
{
    return action == DIG ||
           action == DIG_UP ||
           action == DIG_DOWN;
}
