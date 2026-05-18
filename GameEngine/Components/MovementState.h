#pragma once

#include "Component.h"

enum class MovementStateType
{
    StandRight,
    StandLeft,
    StandUp,
    StandDown,
    WalkRight,
    WalkLeft,
    WalkUp,
    WalkDown
};

class MovementState : public Component
{
public:
    MovementState();

    void SetState(MovementStateType newState);
    void SetFromDirectionInput(bool moveUp, bool moveDown, bool moveLeft, bool moveRight);

    MovementStateType GetState() const;
    const char* GetStateName() const;

private:
    MovementStateType state;

    MovementStateType GetStandStateForCurrentDirection() const;
    static const char* ToString(MovementStateType state);
};
