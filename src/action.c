#include "action.h"

const char *action_as_string(const Action action)
{
    switch (action)
    {
    case REFUEL:            return "REFUEL";

    case SELECT_FIRST_SLOT: return "SELECT_FIRST_SLOT";
    case SELECT_NEXT_SLOT:  return "SELECT_NEXT_SLOT";

    case DIG_FORWARD:       return "DIG_FORWARD";
    case DIG_UP:            return "DIG_UP";
    case DIG_DOWN:          return "DIG_DOWN";

    case MOVE_FORWARD:      return "MOVE_FORWARD";
    case MOVE_UP:           return "MOVE_UP";
    case MOVE_DOWN:         return "MOVE_DOWN";

    case TURN_LEFT:         return "TURN_LEFT";
    case TURN_RIGHT:        return "TURN_RIGHT";
    }

    UNREACHABLE;
}

const char *outcome_as_string(const Outcome outcome) {
    switch (outcome)
    {
        case OUTCOME_NONE: return "";
        case OUTCOME_FAIL: return "FAIL";
        case OUTCOME_PASS: return "PASS";
    }

    UNREACHABLE;
}