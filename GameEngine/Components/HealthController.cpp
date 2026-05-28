#include "HealthController.h"

#include "GameObject.h"
#include "HealthState.h"
#include "LifeState.h"
#include "Logger.h"

HealthController::HealthController()
{
    Logger::Info("HealthController created");
}

void HealthController::Start()
{
    if (pOwner == nullptr) {
        Logger::Warning("HealthController started without owner");
        isStarted = true;
        return;
    }

    healthState = pOwner->GetState<HealthState>();
    if (healthState == nullptr) {
        Logger::Warning("HealthController started without HealthState. owner=%s", pOwner->name.c_str());
    }

    lifeState = pOwner->GetState<LifeState>();
    if (lifeState == nullptr) {
        Logger::Warning("HealthController started without LifeState. owner=%s", pOwner->name.c_str());
    }

    isStarted = true;
    Logger::Info("HealthController started. owner=%s", pOwner->name.c_str());
}

void HealthController::Update(float dt)
{
    if (invincibilityRemaining > 0.0f) {
        invincibilityRemaining -= dt;
        if (invincibilityRemaining < 0.0f) {
            invincibilityRemaining = 0.0f;
        }
    }
}

void HealthController::TakeDamage(int amount)
{
    if (healthState == nullptr) {
        Logger::Warning("HealthController::TakeDamage ignored — no HealthState");
        return;
    }
    if (amount <= 0) {
        return;
    }
    // 이미 사망한 대상에는 추가 데미지를 적용하지 않는다. (사망 후 Update 가드 원칙)
    if (lifeState != nullptr && lifeState->IsDead()) {
        return;
    }
    // 무적 시간 중: 연속 충돌에서 매 프레임 데미지가 누적되지 않도록 무시.
    if (invincibilityRemaining > 0.0f) {
        return;
    }

    const int prev = healthState->GetCurrent();
    const int next = prev - amount;
    healthState->SetCurrent(next);
    invincibilityRemaining = invincibilityDuration;
    Logger::Info("HealthController::TakeDamage owner=%s amount=%d hp=%d->%d invincible=%.2fs",
                 pOwner ? pOwner->name.c_str() : "(null)", amount, prev, healthState->GetCurrent(), invincibilityDuration);

    // HP가 0 이하가 되면 LifeState를 Dead로 전환한다.
    // SetDead가 콜백을 발화해 SpriteAnimator/PlayerControl/DeathTimer 등이 자동 반응한다.
    if (healthState->GetCurrent() <= 0 && lifeState != nullptr && lifeState->IsAlive()) {
        lifeState->SetDead();
    }
}

void HealthController::SetInvincibilityDuration(float seconds)
{
    invincibilityDuration = (seconds >= 0.0f) ? seconds : 0.4f;
}

void HealthController::Heal(int amount)
{
    if (healthState == nullptr) {
        return;
    }
    if (amount <= 0) {
        return;
    }
    if (lifeState != nullptr && lifeState->IsDead()) {
        return;
    }

    const int prev = healthState->GetCurrent();
    healthState->SetCurrent(prev + amount);
    Logger::Info("HealthController::Heal owner=%s amount=%d hp=%d->%d",
                 pOwner ? pOwner->name.c_str() : "(null)", amount, prev, healthState->GetCurrent());
}
