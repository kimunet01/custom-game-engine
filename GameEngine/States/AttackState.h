#pragma once

/*
 * AttackState.h
 * 현재 공격 종류를 표현하는 관측 가능한 State.
 *
 * 순수 데이터다. 공격 시작/지속 시간 관리 같은 시간 누적 로직은 별도 Component
 * (AttackController)가 담당한다. 본 클래스에는 timer나 trigger 메서드가 없다.
 */

#include "State.h"

enum class AttackStateType
{
    NoAttack,
    SwordAttack,
    MagicAttack
};

class AttackState : public ObservableState<AttackStateType>
{
public:
    AttackState();

    // 현재 공격 중인지 여부. NoAttack이 아닐 때 true.
    bool IsAttacking() const;

    const char* GetStateName() const;
    static const char* ToString(AttackStateType state);
};
