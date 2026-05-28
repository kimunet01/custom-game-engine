#pragma once

/*
 * HealthState.h
 * 캐릭터의 체력을 표현하는 관측 가능한 State.
 *
 * 현재 HP는 ObservableState<int>의 값으로 보관되어 변경 시 (prev, next) 콜백이 발화된다.
 * 이를 통해 HitReactionController 같은 외부 컴포넌트가 데미지 발생을 구독할 수 있다.
 *
 * 최대 HP는 통보 대상이 아니므로 별도 멤버로 보관한다.
 * 실제 데미지 누적/사망 판정 로직은 HealthController가 담당한다 (데이터/로직 분리).
 */

#include "State.h"

class HealthState : public ObservableState<int>
{
public:
    explicit HealthState(int maxHp = 1);

    // 외부에서 현재 HP를 직접 설정하고 싶을 때 사용한다.
    // 내부적으로 베이스 Set을 호출하므로 값이 바뀌면 구독자에게 통보된다.
    void SetCurrent(int hp);

    int GetCurrent() const;
    int GetMax() const;
    float GetRatio() const;

private:
    // 최댓값. 변경되더라도 통보 대상이 아니므로 ObservableState 메커니즘 밖에서 관리한다.
    int maxHP;
};
