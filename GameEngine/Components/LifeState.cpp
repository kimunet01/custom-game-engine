#include "LifeState.h"

#include "Logger.h"

LifeState::LifeState()
    : state(LifeStateType::Alive)
{
    Logger::Info("LifeState created. state=%s", GetStateName());
}

void LifeState::SetState(LifeStateType newState)
{
    if (state == newState) {
        return;
    }

    state = newState;
    Logger::Info("LifeState changed. state=%s", GetStateName());
}

void LifeState::SetAlive()
{
    SetState(LifeStateType::Alive);
}

void LifeState::SetDead()
{
    SetState(LifeStateType::Dead);
}

LifeStateType LifeState::GetState() const
{
    return state;
}

bool LifeState::IsAlive() const
{
    return state == LifeStateType::Alive;
}

bool LifeState::IsDead() const
{
    return state == LifeStateType::Dead;
}

const char* LifeState::GetStateName() const
{
    return ToString(state);
}

const char* LifeState::ToString(LifeStateType state)
{
    switch (state) {
    case LifeStateType::Alive: return "alive";
    case LifeStateType::Dead: return "dead";
    default: return "unknown";
    }
}
