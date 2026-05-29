#pragma once

/*
 * HitReactionController.h
 * 피격 시 시각 반응(빨간 깜빡임 + 위치 흔들림)에 필요한 시간/파라미터만 보유하는 컴포넌트.
 *
 * 정책:
 *  - 컴포넌트는 lifecycle(Start/Update) + public 데이터만 보유.
 *  - HealthState 변경에 대한 반응 로직은 StateCallbacks::OnHitReaction가 담당한다.
 *    OnHitReaction이 본 컴포넌트의 remainingTime/elapsedSincePeak을 직접 세팅해
 *    Update가 이후 매 프레임 보간/흔들림을 그려낸다.
 */

#include "Component.h"

class HitReactionController : public Component
{
public:
    HitReactionController();

    void Start() override;
    void Update(float dt) override;

    // ── 콜백이 직접 접근하는 public 데이터 ──
    // 남은 반응 시간. 0이면 비활성. OnHitReaction이 duration으로 세팅.
    float remainingTime = 0.0f;
    // 총 반응 지속 시간. 시각 보간의 분모.
    float duration = 0.25f;
    // 흔들림 진폭 (월드 단위).
    float shakeAmplitude = 0.015f;
    // 흔들림 주파수 (Hz).
    float shakeFrequency = 40.0f;
    // sin 위상을 누적해 흔들림 위치를 매 프레임 계산.
    float elapsedSincePeak = 0.0f;
};
