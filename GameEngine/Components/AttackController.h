#pragma once

/*
 * AttackController.h
 * AttackState의 시간 누적 로직을 담당하는 Component.
 *
 * AttackState는 순수 데이터(공격 종류 enum 값 + 변경 통보)만 보유한다.
 * "공격을 발동시키고 일정 시간 후 자동으로 해제"하는 로직은 본 Controller가 처리한다.
 *
 * 호출 측(예: PlayerControl)은 본 Controller의 TriggerSword/TriggerMagic만 호출한다.
 * AttackState를 직접 Set하지 않는다 — 단방향 흐름 유지.
 */

#include "Component.h"

class AttackState;

class AttackController : public Component
{
public:
    AttackController();

    void Start() override;

    // 매 프레임 remainingTime을 감소시키고, 0 이하가 되면 AttackState를 NoAttack으로 되돌린다.
    void Update(float dt) override;

    // 공격 시작 트리거. AttackState를 SwordAttack으로 Set하고 duration 초만큼 유지한다.
    void TriggerSword(float duration);
    // 마법 공격 시작 트리거. AttackState를 MagicAttack으로 Set하고 duration 초만큼 유지한다.
    void TriggerMagic(float duration);

private:
    AttackState* attackState = nullptr;
    float remainingTime = 0.0f;
};
