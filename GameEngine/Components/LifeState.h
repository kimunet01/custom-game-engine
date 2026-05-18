#pragma once

#include "Component.h"

enum class LifeStateType
{
    Alive,
    Dead
};

class LifeState : public Component
{
public:
    LifeState();

    void SetState(LifeStateType newState);
    void SetAlive();
    void SetDead();

    LifeStateType GetState() const;
    bool IsAlive() const;
    bool IsDead() const;
    const char* GetStateName() const;

private:
    LifeStateType state;

    static const char* ToString(LifeStateType state);
};
