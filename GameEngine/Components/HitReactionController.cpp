#include "HitReactionController.h"

#include <cmath>

#include "GameObject.h"
#include "HealthState.h"
#include "LifeState.h"
#include "Logger.h"
#include "MeshRenderer.h"

HitReactionController::HitReactionController()
{
    Logger::Info("HitReactionController created");
}

void HitReactionController::Start()
{
    if (pOwner == nullptr) {
        Logger::Warning("HitReactionController started without owner");
        isStarted = true;
        return;
    }

    healthState = pOwner->GetState<HealthState>();
    lifeState = pOwner->GetState<LifeState>();
    meshRenderer = pOwner->GetComponent<MeshRenderer>();

    if (healthState == nullptr) {
        Logger::Warning("HitReactionController started without HealthState. owner=%s", pOwner->name.c_str());
    }
    else {
        // HealthState 값(int) 변경을 구독한다. prev>next, 즉 데미지를 입은 경우만 반응.
        healthState->Subscribe([this](int prev, int next) {
            this->OnHealthChanged(prev, next);
        });
    }

    if (meshRenderer == nullptr) {
        Logger::Warning("HitReactionController started without MeshRenderer. owner=%s — tint will be skipped", pOwner->name.c_str());
    }

    isStarted = true;
    Logger::Info("HitReactionController started. owner=%s", pOwner->name.c_str());
}

void HitReactionController::Update(float dt)
{
    if (remainingTime <= 0.0f) {
        return;
    }

    remainingTime -= dt;
    elapsedSincePeak += dt;

    if (remainingTime <= 0.0f) {
        EndReaction();
        return;
    }

    // 남은 시간 비율을 기준으로 tint를 흰색으로 복귀시킨다.
    // t=1(반응 시작 직후): 빨강(1,0,0,1)
    // t=0(반응 종료): 흰색(1,1,1,1)
    const float t = (duration > 0.0f) ? (remainingTime / duration) : 0.0f;
    const float r = 1.0f;
    const float g = 1.0f - t; // t=1이면 0(빨강), t=0이면 1(흰색)
    const float b = 1.0f - t;
    if (meshRenderer != nullptr) {
        meshRenderer->SetTint(r, g, b, 1.0f);
    }

    // 좌우 흔들림: sin(2π * freq * elapsed) * amplitude.
    // 반응이 끝날수록 진폭이 줄어들도록 t로 스케일링.
    if (pOwner != nullptr) {
        const float phase = 6.2831853f * shakeFrequency * elapsedSincePeak;
        pOwner->renderOffset.x = std::sin(phase) * shakeAmplitude * t;
        pOwner->renderOffset.y = 0.0f;
    }
}

void HitReactionController::SetDuration(float seconds)
{
    duration = (seconds > 0.0f) ? seconds : 0.25f;
}

void HitReactionController::OnHealthChanged(int prev, int next)
{
    // 데미지를 입은 경우(next < prev)만 반응. 회복은 시각 반응 없음.
    if (next >= prev) {
        return;
    }
    // 사망한 대상의 새 반응은 무시 (사망 후 Update 가드).
    // 단, 이번 데미지로 사망한 경우는 HealthController가 SetDead를 호출하기 전이므로 여기서는 alive로 보일 수 있음.
    // 그 경우는 반응이 잠깐 시작되어도 시각적으로 자연스러우므로 허용한다.
    if (lifeState != nullptr && lifeState->IsDead()) {
        // 이미 죽은 시점에 추가 데미지가 들어오는 경우만 무시.
        return;
    }
    StartReaction();
}

void HitReactionController::StartReaction()
{
    remainingTime = duration;
    elapsedSincePeak = 0.0f;
    Logger::Info("HitReactionController triggered. owner=%s duration=%.3f",
                 pOwner ? pOwner->name.c_str() : "(null)", duration);
}

void HitReactionController::EndReaction()
{
    remainingTime = 0.0f;
    elapsedSincePeak = 0.0f;
    if (meshRenderer != nullptr) {
        meshRenderer->SetTint(1.0f, 1.0f, 1.0f, 1.0f);
    }
    if (pOwner != nullptr) {
        pOwner->renderOffset.x = 0.0f;
        pOwner->renderOffset.y = 0.0f;
        pOwner->renderOffset.z = 0.0f;
    }
}
