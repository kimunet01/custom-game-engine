#pragma once

/*
 * LifeState.h
 * 캐릭터의 생존 여부를 표현하는 관측 가능한 State.
 *
 * 데이터(Alive/Dead enum)만 보유하며, 변경 시 구독자에게 통보한다.
 * 사망 처리 시점은 외부(CollisionSystem, 게임 규칙 등)가 SetDead를 호출하여 결정한다.
 */

#include "State.h"

enum class LifeStateType
{
    Alive,
    Dead
};

class LifeState : public ObservableState<LifeStateType>
{
public:
    LifeState();

    // 의미를 명확히 하기 위한 편의 메서드. 내부적으로 베이스의 Set을 호출한다.
    void SetAlive();
    void SetDead();

    bool IsAlive() const;
    bool IsDead() const;

    const char* GetStateName() const;
    static const char* ToString(LifeStateType state);
};
