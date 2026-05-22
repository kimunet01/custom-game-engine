#include "StateCallbacks.h"

#include <string>

#include "GameObject.h"
#include "Logger.h"
#include "PlayerControl.h"
#include "SpriteAnimator.h"
#include "EnemyController.h"
#include "EnemyState.h"

namespace
{
    // 현재 owner GameObject의 3개 State 조합을 보고 재생할 클립 이름을 계산한다.
    // 우선순위: Dead → Attacking → Movement.
    // 어떤 State가 없거나 매칭되는 클립이 없으면 빈 문자열을 반환한다.
    std::string ComputeClipName(SpriteAnimator* self)
    {
        if (self == nullptr || self->pOwner == nullptr) {
            return std::string();
        }

        GameObject* owner = self->pOwner;

        LifeState* life = owner->GetState<LifeState>();
        if (life != nullptr && life->IsDead()) {
            return std::string("dead");
        }

        AttackState* attack = owner->GetState<AttackState>();
        if (attack != nullptr && attack->IsAttacking()) {
            if (attack->Get() == AttackStateType::SwordAttack) {
                MovementState* movement = owner->GetState<MovementState>();
                const char* direction = (movement != nullptr) ? movement->GetDirectionName() : "down";
                return std::string("sword_attack_") + direction;
            }
            return std::string(attack->GetStateName());
        }

        MovementState* movement = owner->GetState<MovementState>();
        if (movement != nullptr) {
            return std::string(movement->GetStateName());
        }

        return std::string();
    }

}

namespace StateCallbacks
{
    // 3개 State 중 어느 것이 바뀌어도 동일한 재평가가 필요하므로 공통 진입점으로 묶는다.
    // SpriteAnimator::SwitchToClip(name)이 클립 전환의 primitive이며, 동일 이름 가드는 그쪽에서 처리한다.
    void ReevaluateAnimClip(SpriteAnimator* self)
    {
        const std::string clipName = ComputeClipName(self);
        if (clipName.empty()) {
            return;
        }
        self->SwitchToClip(clipName);
    }


    void OnAnimMovement(SpriteAnimator* self, MovementStateType prev, MovementStateType next)
    {
        Logger::Info("StateCallbacks::OnAnimMovement %s -> %s",
                     MovementState::ToString(prev), MovementState::ToString(next));
        ReevaluateAnimClip(self);
    }

    void OnAnimAttack(SpriteAnimator* self, AttackStateType prev, AttackStateType next)
    {
        Logger::Info("StateCallbacks::OnAnimAttack %s -> %s",
                     AttackState::ToString(prev), AttackState::ToString(next));
        ReevaluateAnimClip(self);
    }

    void OnAnimLife(SpriteAnimator* self, LifeStateType prev, LifeStateType next)
    {
        Logger::Info("StateCallbacks::OnAnimLife %s -> %s",
                     LifeState::ToString(prev), LifeState::ToString(next));
        ReevaluateAnimClip(self);
    }

    void OnControlLife(PlayerControl* self, LifeStateType prev, LifeStateType next)
    {
        Logger::Info("StateCallbacks::OnControlLife %s -> %s",
                     LifeState::ToString(prev), LifeState::ToString(next));
        if (self == nullptr) {
            return;
        }
        // PlayerControl이 노출하는 입력 잠금 플래그를 갱신한다. Update는 이 값만 보고 분기한다.
        self->isMovementLocked = (next == LifeStateType::Dead);
    }

    void OnControlAttack(PlayerControl* self, AttackStateType prev, AttackStateType next)
    {
        Logger::Info("StateCallbacks::OnControlAttack %s -> %s",
                     AttackState::ToString(prev), AttackState::ToString(next));
        if (self == nullptr) {
            return;
        }
        self->isAttackLocked = (next != AttackStateType::NoAttack);
    }

    // --- 2026-05-23: 리팩토링 가이드(중앙 집중화) 원칙에 따른 Enemy 관련 콜백 통합 추가 ---

    void OnAnimEnemy(SpriteAnimator* self, EnemyStateType prev, EnemyStateType next)
    {
        Logger::Info("StateCallbacks::OnAnimEnemy %s -> %s",
                     EnemyState::ToString(prev), EnemyState::ToString(next));
        
        if (self == nullptr || self->pOwner == nullptr) return;
        self->SwitchToClip(EnemyState::ToString(next));
    }

    void OnControlEnemy(EnemyController* self, EnemyStateType prev, EnemyStateType next)
    {
        Logger::Info("StateCallbacks::OnControlEnemy %s -> %s",
                     EnemyState::ToString(prev), EnemyState::ToString(next));
        if (self == nullptr) return;

        // Move 이외의 상태(Dead, Disabled)에서는 움직임을 잠금
        self->isMovementLocked = (next != EnemyStateType::Move);
    }

    void ReevaluateEnemyAnimClip(SpriteAnimator* self)
    {
        if (self == nullptr || self->pOwner == nullptr) return;

        EnemyState* enemy = self->pOwner->GetState<EnemyState>();
        if (enemy != nullptr) {
            self->SwitchToClip(enemy->GetStateName());
        }
    }
}
