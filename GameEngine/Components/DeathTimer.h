#pragma once

/*
 * DeathTimer.h
 * LifeState=Dead 진입 후 일정 시간 뒤에 GameObject->pendingDestroy를 세팅하는 데이터 보유 컴포넌트.
 *
 * 정책: 컴포넌트는 lifecycle(Start/Update) + public 데이터만 보유.
 * LifeState 변경에 대한 반응 로직은 StateCallbacks::OnLifeDeathTimer가 담당한다.
 * 본 컴포넌트의 Update는 remainingTime이 양수인 동안만 카운트다운한다.
 */

#include "Component.h"

class DeathTimer : public Component
{
public:
    DeathTimer();

    void Start() override;
    void Update(float dt) override;

    // ── 콜백이 직접 접근하는 public 데이터 ──
    // 음수면 "아직 카운트다운 안 시작". 양수면 매 프레임 감소하여 0이 되면 pendingDestroy.
    float remainingTime = -1.0f;
    // Dead 진입 시 OnLifeDeathTimer가 remainingTime에 세팅하는 값(초).
    float delay = 1.5f;
};
