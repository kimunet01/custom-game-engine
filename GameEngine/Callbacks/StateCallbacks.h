#pragma once

/*
 * StateCallbacks.h
 * State 변경에 대한 반응 로직을 한 곳에 모아둔 콜백 모음.
 *
 * 각 콜백은 `(self*, prev, next)` 시그니처를 따르며, 컴포넌트는 Start() 시점에
 *   xxxState->Subscribe([this](auto p, auto n){ StateCallbacks::OnXxx(this, p, n); });
 * 형태로 등록한다. 이렇게 하면 컴포넌트는 "어떤 State에 어떤 반응 함수를 연결할지"만 알면 되고,
 * 실제 반응 로직(데이터에 따른 처리)은 모두 본 모듈에 응집된다.
 *
 * 컴포넌트 인스턴스는 첫 인자(self)로 받아 그 안의 public 멤버나 메서드를 통해
 * 부수 효과(클립 전환, 입력 잠금 플래그 갱신 등)를 적용한다.
 */

#include "MovementState.h"
#include "AttackState.h"
#include "LifeState.h"

class SpriteAnimator;
class PlayerControl;

namespace StateCallbacks
{
    // SpriteAnimator 측 반응: 3개 State 중 어느 하나가 바뀌어도 클립을 재평가한다.
    // priority(우선순위): Dead > Attacking > Movement
    void OnAnimMovement(SpriteAnimator* self, MovementStateType prev, MovementStateType next);
    void OnAnimAttack  (SpriteAnimator* self, AttackStateType   prev, AttackStateType   next);
    void OnAnimLife    (SpriteAnimator* self, LifeStateType     prev, LifeStateType     next);

    // 초기 동기화용 명시적 진입점. 컴포넌트 Start()에서 현재 상태 기준 초기 클립을 선택할 때 호출한다.
    // 콜백은 "변경 시"만 발화하므로 시작 시 1회는 명시적으로 호출해야 한다.
    void ReevaluateAnimClip(SpriteAnimator* self);

    // PlayerControl 측 반응: Update가 사용할 입력 잠금 플래그를 갱신한다.
    void OnControlLife  (PlayerControl* self, LifeStateType   prev, LifeStateType   next);
    void OnControlAttack(PlayerControl* self, AttackStateType prev, AttackStateType next);
}
