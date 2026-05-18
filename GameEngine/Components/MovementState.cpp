#include "MovementState.h"

#include "Logger.h"

MovementState::MovementState()
    : state(MovementStateType::StandDown)
{
    Logger::Info("MovementState created. state=%s", GetStateName());
}

void MovementState::SetState(MovementStateType newState)
{
    if (state == newState) {
        return;
    }

    state = newState;
    Logger::Info("MovementState changed. state=%s", GetStateName());
}

void MovementState::SetFromDirectionInput(bool moveUp, bool moveDown, bool moveLeft, bool moveRight)
{
    if (moveRight) {
        SetState(MovementStateType::WalkRight);
        return;
    }
    if (moveLeft) {
        SetState(MovementStateType::WalkLeft);
        return;
    }
    if (moveUp) {
        SetState(MovementStateType::WalkUp);
        return;
    }
    if (moveDown) {
        SetState(MovementStateType::WalkDown);
        return;
    }

    SetState(GetStandStateForCurrentDirection());
}

MovementStateType MovementState::GetState() const
{
    return state;
}

const char* MovementState::GetStateName() const
{
    return ToString(state);
}

const char* MovementState::GetDirectionName() const
{
    switch (state) {
    case MovementStateType::WalkRight:
    case MovementStateType::StandRight:
        return "right";
    case MovementStateType::WalkLeft:
    case MovementStateType::StandLeft:
        return "left";
    case MovementStateType::WalkUp:
    case MovementStateType::StandUp:
        return "up";
    case MovementStateType::WalkDown:
    case MovementStateType::StandDown:
        return "down";
    default:
        return "down";
    }
}

MovementStateType MovementState::GetStandStateForCurrentDirection() const
{
    switch (state) {
    case MovementStateType::WalkRight:
    case MovementStateType::StandRight:
        return MovementStateType::StandRight;
    case MovementStateType::WalkLeft:
    case MovementStateType::StandLeft:
        return MovementStateType::StandLeft;
    case MovementStateType::WalkUp:
    case MovementStateType::StandUp:
        return MovementStateType::StandUp;
    case MovementStateType::WalkDown:
    case MovementStateType::StandDown:
        return MovementStateType::StandDown;
    default:
        return MovementStateType::StandDown;
    }
}

const char* MovementState::ToString(MovementStateType state)
{
    switch (state) {
    case MovementStateType::StandRight: return "stand_right";
    case MovementStateType::StandLeft: return "stand_left";
    case MovementStateType::StandUp: return "stand_up";
    case MovementStateType::StandDown: return "stand_down";
    case MovementStateType::WalkRight: return "walk_right";
    case MovementStateType::WalkLeft: return "walk_left";
    case MovementStateType::WalkUp: return "walk_up";
    case MovementStateType::WalkDown: return "walk_down";
    default: return "unknown";
    }
}
