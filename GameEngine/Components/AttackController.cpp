#include "AttackController.h"

#include "AttackState.h"
#include "GameObject.h"
#include "Logger.h"

AttackController::AttackController()
{
    Logger::Info("AttackController created");
}

void AttackController::Start()
{
    if (pOwner == nullptr) {
        Logger::Warning("AttackController started without owner");
        isStarted = true;
        return;
    }

    attackState = pOwner->GetState<AttackState>();
    if (attackState == nullptr) {
        Logger::Warning("AttackController started without AttackState. owner=%s", pOwner->name.c_str());
    }

    isStarted = true;
    Logger::Info("AttackController started. owner=%s", pOwner->name.c_str());
}

void AttackController::Update(float dt)
{
    if (attackState == nullptr) {
        return;
    }
    if (remainingTime <= 0.0f) {
        return;
    }

    remainingTime -= dt;
    if (remainingTime <= 0.0f) {
        // 타이머 만료 → 공격 해제. Set이 콜백을 발화해 SpriteAnimator/PlayerControl이 자동 반응한다.
        remainingTime = 0.0f;
        attackState->Set(AttackStateType::NoAttack);
    }
}

void AttackController::TriggerSword(float duration)
{
    if (attackState == nullptr) {
        Logger::Warning("AttackController::TriggerSword ignored — no AttackState");
        return;
    }
    if (duration <= 0.0f) {
        Logger::Warning("AttackController::TriggerSword ignored — non-positive duration %.3f", duration);
        return;
    }

    remainingTime = duration;
    attackState->Set(AttackStateType::SwordAttack);
}

void AttackController::TriggerMagic(float duration)
{
    if (attackState == nullptr) {
        Logger::Warning("AttackController::TriggerMagic ignored — no AttackState");
        return;
    }
    if (duration <= 0.0f) {
        Logger::Warning("AttackController::TriggerMagic ignored — non-positive duration %.3f", duration);
        return;
    }

    remainingTime = duration;
    attackState->Set(AttackStateType::MagicAttack);
}
