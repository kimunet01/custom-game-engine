#pragma once

/*
 * HealthController.h
 * 무적 타이머만 가진 데이터 보유 Component.
 *
 * 정책:
 *  - 외부 동작 진입점은 StateCallbacks의 자유 함수로 노출:
 *    StateCallbacks::ApplyDamage(target, amount) / ApplyHeal(target, amount)
 *  - GameObject가 이미 보유한 HealthState/LifeState 포인터는 멤버로 캐싱하지 않는다.
 *    콜백이 owner->GetState<>로 즉시 조회한다.
 *
 *  본 컴포넌트는 lifecycle(Start/Update) + 무적 타이머 데이터만 보유.
 */

#include "Component.h"

class HealthController : public Component
{
public:
    HealthController();

    void Start() override;
    // 무적 시간(invincibility) 카운트다운만 처리한다.
    void Update(float dt) override;

    // ── 콜백이 직접 접근하는 public 데이터 ──
    // 0보다 크면 무적 중. 매 프레임 Update에서 감소한다.
    float invincibilityRemaining = 0.0f;
    // 데미지 후 무적 시간(초). 기본 0.4s. main에서 직접 수정 가능.
    float invincibilityDuration = 0.4f;
};
