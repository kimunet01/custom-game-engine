#include "EnemyState.h"
#include "Logger.h"

EnemyState::EnemyState()
{
    // 초기 상태를 Disabled로 설정 (풀링 대기)
    current = EnemyStateType::Disabled;
}

void EnemyState::SetMove()
{
    Set(EnemyStateType::Move);
}

void EnemyState::SetDead()
{
    Set(EnemyStateType::Dead);
}

void EnemyState::SetDisabled()
{
    Set(EnemyStateType::Disabled);
}

bool EnemyState::IsMoving() const
{
    return Get() == EnemyStateType::Move;
}

bool EnemyState::IsDead() const
{
    return Get() == EnemyStateType::Dead;
}

bool EnemyState::IsDisabled() const
{
    return Get() == EnemyStateType::Disabled;
}

const char* EnemyState::GetStateName() const
{
    return ToString(Get());
}

const char* EnemyState::ToString(EnemyStateType state)
{
    switch (state) {
    case EnemyStateType::Move:     return "move";
    case EnemyStateType::Dead:     return "dead";
    case EnemyStateType::Disabled: return "disabled";
    default:                       return "unknown";
    }
}
