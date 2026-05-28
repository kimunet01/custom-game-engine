#pragma once

/*
 * HealthController.h
 * HP 변동(데미지/회복)과 사망 판정을 담당하는 Component.
 *
 * HealthState는 순수 데이터(현재 HP + 변경 통보)만 가지며, "HP가 0 이하가 되면 LifeState를 Dead로 전환"
 * 같은 결정은 본 Controller가 담당한다. 이는 AttackController가 AttackState 타이머를 관리하는 패턴과 같다.
 *
 * 외부 호출자(예: CombatSystem)는 본 Controller의 TakeDamage만 호출한다.
 * HealthState/LifeState를 직접 Set하지 않는다 — 단방향 흐름 유지.
 */

#include "Component.h"

class HealthState;
class LifeState;

class HealthController : public Component
{
public:
    HealthController();

    void Start() override;

    // 데미지를 적용한다. amount > 0만 의미가 있으며, HP가 0 이하가 되면 LifeState.SetDead()를 호출한다.
    // 이미 사망한 상태에서는 무시한다.
    void TakeDamage(int amount);

    // 회복을 적용한다. amount > 0만 의미가 있다. 이미 사망한 경우는 무시한다.
    void Heal(int amount);

private:
    HealthState* healthState = nullptr;
    LifeState* lifeState = nullptr;
};
