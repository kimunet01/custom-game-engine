#pragma once

/*
 * HitReactionController.h
 * 피격 시 시각 반응(빨간 깜빡임 + 위치 흔들림)을 담당하는 Component.
 *
 * HealthState의 변경 콜백을 구독해 (prev > next), 즉 데미지를 입은 시점에만 트리거된다.
 * 트리거되면 일정 시간 동안:
 *   - MeshRenderer의 tint를 빨강(1,0,0,1)에서 흰색(1,1,1,1)으로 보간
 *   - GameObject의 renderOffset에 sin 기반 좌우 흔들림 적용
 * 종료 시 두 효과 모두 원상복구한다.
 *
 * position/velocity는 건드리지 않는다 — CollisionSystem 오동작 방지.
 * 사망 상태에서는 새로운 반응을 시작하지 않는다.
 */

#include "Component.h"

class HealthState;
class LifeState;
class MeshRenderer;

class HitReactionController : public Component
{
public:
    HitReactionController();

    void Start() override;
    void Update(float dt) override;

    // 외부에서 반응 시간을 조정할 수 있는 setter (기본 0.25s).
    void SetDuration(float seconds);

private:
    HealthState* healthState = nullptr;
    LifeState* lifeState = nullptr;
    MeshRenderer* meshRenderer = nullptr;

    // 남은 반응 시간. 0이면 비활성.
    float remainingTime = 0.0f;
    // 총 반응 지속 시간. 시각 보간의 분모로 사용.
    float duration = 0.25f;
    // 흔들림 진폭 (월드 단위).
    float shakeAmplitude = 0.015f;
    // 흔들림 주파수 (Hz). 반응 시간 내 몇 번 진동할지 결정.
    float shakeFrequency = 40.0f;
    // sin 위상을 누적해 흔들림 위치를 매 프레임 계산.
    float elapsedSincePeak = 0.0f;

    // 데미지 콜백. HealthState 변경 시 호출되어 prev>next면 반응 시작.
    void OnHealthChanged(int prev, int next);
    // 반응 시작 (timer 리셋, tint/offset 적용 준비).
    void StartReaction();
    // 반응 종료 (tint 복원, offset 0).
    void EndReaction();
};
