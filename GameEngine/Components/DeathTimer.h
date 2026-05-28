#pragma once

/*
 * DeathTimer.h
 * LifeState가 Dead로 전환되면 N초 후 GameObject에 pendingDestroy 플래그를 세우는 Component.
 *
 * 데드 클립 재생 시간을 확보하기 위해 즉시 제거하지 않고 약간의 지연을 둔다.
 * 실제 메모리 해제는 GameLoop가 매 프레임 끝 단계에서 한 곳에서 수행한다.
 */

#include "Component.h"

class LifeState;

class DeathTimer : public Component
{
public:
    DeathTimer();

    void Start() override;
    void Update(float dt) override;

    // 사망 후 제거까지의 지연 시간(초). 기본 1.5s.
    void SetDelay(float seconds);

private:
    LifeState* lifeState = nullptr;

    // Dead 콜백을 받으면 delay로 설정되고 매 프레임 감소한다.
    // 음수 값은 "아직 카운트다운 시작 전" 상태를 의미.
    float remainingTime = -1.0f;
    float delay = 1.5f;

    void OnLifeChanged();
};
