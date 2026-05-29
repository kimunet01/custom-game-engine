#pragma once

/*
 * AttackController.h
 * AttackState의 시간 누적과 자동 해제만 담당하는 데이터 보유 Component.
 *
 * 정책:
 *  - 모든 외부 동작 진입점은 메서드가 아니라 Callbacks/StateCallbacks의 자유 함수로 노출.
 *    예: 검 공격 발동 → StateCallbacks::TriggerSword(owner, duration)
 *  - GameObject가 이미 들고 있는 State/Component 포인터는 멤버로 캐싱하지 않는다.
 *    필요할 때 owner->GetState/GetComponent로 즉시 조회한다. (중복 보관·추적성 저하 방지)
 *
 *  본 컴포넌트는 lifecycle(Start/Update) + 컴포넌트 고유 데이터(타이머/데미지/시스템 주입)만 보유.
 */

#include "Component.h"

class CombatSystem;

class AttackController : public Component
{
public:
    AttackController();

    void Start() override;

    // 매 프레임 remainingTime을 감소시키고, 0 이하가 되면 AttackState를 NoAttack으로 되돌린다.
    void Update(float dt) override;

    // ── 콜백/main이 직접 접근하는 public 데이터 ──
    // 외부(main.cpp)에서 주입하는 CombatSystem 포인터. nullptr이면 hitbox 큐 등록 생략.
    CombatSystem* combatSystem = nullptr;
    // 현재 공격이 끝나기까지 남은 시간(초). 0 이하이면 휴식 상태.
    float remainingTime = 0.0f;
    // 한 번의 공격이 적중 시 적용할 데미지.
    int swordDamage = 1;
};
