#include "PlayerControl.h"

#include "AttackState.h"
#include "LifeState.h"
#include "MovementState.h"
#include "GameObject.h"
#include "Logger.h"

/*
 * PlayerControl.cpp
 * PlayerControl의 입력 해석과 속도 갱신 로직을 구현한다.
 *
 * playerType이 0이면 방향키/N, 그 외에는 WASD/M 조합을 사용한다. 이 방식은
 * 같은 컴포넌트를 다른 조작 키셋으로 재사용하기 위한 간단한 분기다.
 */

PlayerControl::PlayerControl(int type)
    : playerType(type) {
    Logger::Info("PlayerControl created. playerType=%d", playerType);
}

void PlayerControl::Input()
{
    if (playerType == 0) {
        // 1번 플레이어 입력: 방향키 이동, N 키 회전.
        moveUp = localKeyState.up;
        moveDown = localKeyState.down;
        moveLeft = localKeyState.left;
        moveRight = localKeyState.right;
        rotate = localKeyState.n;
        attack = localKeyState.space;
    }
    else {
        // 2번 플레이어 입력: WASD 이동, M 키 회전.
        moveUp = localKeyState.w;
        moveDown = localKeyState.s;
        moveLeft = localKeyState.a;
        moveRight = localKeyState.d;
        rotate = localKeyState.m;
        attack = localKeyState.space;
    }
}

void PlayerControl::Start()
{
    if (pOwner != nullptr) {
        attackState = pOwner->GetComponent<AttackState>();
        lifeState = pOwner->GetComponent<LifeState>();
        movementState = pOwner->GetComponent<MovementState>();
        if (attackState == nullptr) {
            Logger::Warning("PlayerControl started without AttackState. owner=%s", pOwner->name.c_str());
        }
        if (lifeState == nullptr) {
            Logger::Warning("PlayerControl started without LifeState. owner=%s", pOwner->name.c_str());
        }
        if (movementState == nullptr) {
            Logger::Warning("PlayerControl started without MovementState. owner=%s", pOwner->name.c_str());
        }
    }

    // 현재 별도 초기화 리소스는 없지만, GameLoop가 중복 Start를 호출하지 않도록 상태를 표시한다.
    isStarted = true;
    Logger::Info("PlayerControl started. owner=%s playerType=%d", pOwner ? pOwner->name.c_str() : "null", playerType);
}

void PlayerControl::Update(float dt)
{
    if (pOwner == nullptr) {
        Logger::Warning("PlayerControl update skipped because owner is null");
        return;
    }

    const bool attackPressedThisFrame = attack && !wasAttackPressed;

    if (movementState != nullptr) {
        movementState->SetFromDirectionInput(moveUp, moveDown, moveLeft, moveRight);
    }

    if (lifeState != nullptr && lifeState->IsDead()) {
        pOwner->velocity.x = 0.0f;
        pOwner->velocity.y = 0.0f;
        wasAttackPressed = attack;
        return;
    }

    if (attackPressedThisFrame && attackState != nullptr && !attackState->IsAttacking()) {
        attackState->TriggerSwordAttack(0.4f);
    }
    wasAttackPressed = attack;

    if (attackState != nullptr && attackState->IsAttacking()) {
        pOwner->velocity.x = 0.0f;
        pOwner->velocity.y = 0.0f;
        return;
    }

    // 입력이 없는 프레임에는 멈추도록 매 프레임 velocity를 먼저 초기화한다.
    pOwner->velocity.x = 0.0f;
    pOwner->velocity.y = 0.0f;

    if (moveUp) {
        pOwner->velocity.y += speed;
    }
    if (moveDown) {
        pOwner->velocity.y -= speed;
    }
    if (moveLeft) {
        pOwner->velocity.x -= speed;
    }
    if (moveRight) {
        pOwner->velocity.x += speed;
    }
    if (rotate) {
        // 회전은 위치 이동과 달리 이 컴포넌트가 직접 누적한다.
        pOwner->rotation += speed * dt;
    }
}


