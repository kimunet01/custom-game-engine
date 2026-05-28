#include "AttackController.h"

#include "AttackState.h"
#include "CombatSystem.h"
#include "GameObject.h"
#include "LifeState.h"
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
    // 사망 상태에서는 공격을 발동하지 않는다 (사망 후 Update 가드와 일관).
    if (pOwner != nullptr) {
        LifeState* life = pOwner->GetState<LifeState>();
        if (life != nullptr && life->IsDead()) {
            return;
        }
    }

    remainingTime = duration;
    attackState->Set(AttackStateType::SwordAttack);
    // hitbox 판정 요청은 공격 진입 1프레임에서만 1회 발생한다.
    // 같은 공격 동안 매 프레임 데미지가 들어가는 것을 방지한다.
    if (combatSystem != nullptr && pOwner != nullptr) {
        combatSystem->RequestHit(pOwner, AttackStateType::SwordAttack, swordDamage);
    }
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
    if (pOwner != nullptr) {
        LifeState* life = pOwner->GetState<LifeState>();
        if (life != nullptr && life->IsDead()) {
            return;
        }
    }

    remainingTime = duration;
    attackState->Set(AttackStateType::MagicAttack);
    if (combatSystem != nullptr && pOwner != nullptr) {
        combatSystem->RequestHit(pOwner, AttackStateType::MagicAttack, swordDamage);
    }
}

void AttackController::SetCombatSystem(CombatSystem* combat)
{
    combatSystem = combat;
}

void AttackController::SetSwordDamage(int dmg)
{
    swordDamage = (dmg > 0) ? dmg : 1;
}
