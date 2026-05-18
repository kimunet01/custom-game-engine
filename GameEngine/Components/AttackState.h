#pragma once

#include "Component.h"

enum class AttackStateType
{
    NoAttack,
    SwordAttack,
    MagicAttack
};

class AttackState : public Component
{
public:
    AttackState();

    void SetState(AttackStateType newState);
    void TriggerSwordAttack(float duration = 0.6f);
    void TriggerMagicAttack(float duration = 0.6f);
    void ClearAttack();

    AttackStateType GetState() const;
    bool IsAttacking() const;
    const char* GetStateName() const;

    void Update(float dt) override;

private:
    AttackStateType state;
    float remainingTime;

    static const char* ToString(AttackStateType state);
};
