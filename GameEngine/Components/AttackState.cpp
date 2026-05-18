#include "AttackState.h"

#include "Logger.h"

AttackState::AttackState()
    : state(AttackStateType::NoAttack)
    , remainingTime(0.0f)
{
    Logger::Info("AttackState created. state=%s", GetStateName());
}

void AttackState::SetState(AttackStateType newState)
{
    if (state == newState) {
        return;
    }

    state = newState;
    Logger::Info("AttackState changed. state=%s", GetStateName());
}

void AttackState::TriggerSwordAttack(float duration)
{
    remainingTime = duration;
    SetState(AttackStateType::SwordAttack);
}

void AttackState::TriggerMagicAttack(float duration)
{
    remainingTime = duration;
    SetState(AttackStateType::MagicAttack);
}

void AttackState::ClearAttack()
{
    remainingTime = 0.0f;
    SetState(AttackStateType::NoAttack);
}

AttackStateType AttackState::GetState() const
{
    return state;
}

bool AttackState::IsAttacking() const
{
    return state != AttackStateType::NoAttack;
}

const char* AttackState::GetStateName() const
{
    return ToString(state);
}

void AttackState::Update(float dt)
{
    if (!IsAttacking()) {
        return;
    }

    remainingTime -= dt;
    if (remainingTime <= 0.0f) {
        ClearAttack();
    }
}

const char* AttackState::ToString(AttackStateType state)
{
    switch (state) {
    case AttackStateType::NoAttack: return "no_attack";
    case AttackStateType::SwordAttack: return "sword_attack";
    case AttackStateType::MagicAttack: return "magic_attack";
    default: return "unknown";
    }
}
