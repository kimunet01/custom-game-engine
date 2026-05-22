#include "EnemyController.h"
#include "GameObject.h"
#include "Logger.h"
#include "StateCallbacks.h"
#include "SpriteAnimator.h"
#include "EnemySpawner.h"
#include <cmath>

EnemyController::~EnemyController()
{
    if (pEnemyState != nullptr) {
        pEnemyState->ClearSubscribers();
    }
}

void EnemyController::Reset()
{
    isMovementLocked = false;
    isAttackLocked = false;
    attackTimer = 0.0f;
    deathTimer = 0.0f;
}

void EnemyController::Start()
{
    if (pOwner == nullptr) return;

    pEnemyState = pOwner->GetState<EnemyState>();
    if (pEnemyState == nullptr) {
        Logger::Warning("EnemyController started without EnemyState. owner=%s", pOwner->name.c_str());
    }
    else {
        // 기존 구독 해제 (재사용 시 중복 구독 방지)
        pEnemyState->ClearSubscribers();

        // EnemyState 변경 시 제어 플래그를 업데이트하도록 콜백 등록
        pEnemyState->Subscribe([this](EnemyStateType p, EnemyStateType n) {
            StateCallbacks::OnControlEnemy(this, p, n);
        });

        // 애니메이션 구독도 여기서 처리
        SpriteAnimator* animator = pOwner->GetComponent<SpriteAnimator>();
        if (animator != nullptr) {
            pEnemyState->Subscribe([animator](EnemyStateType p, EnemyStateType n) {
                StateCallbacks::OnAnimEnemy(animator, p, n);
            });
            // 초기 동기화
            StateCallbacks::ReevaluateEnemyAnimClip(animator);
        }
    }

    isStarted = true;
}

void EnemyController::Update(float dt)
{
    if (pOwner == nullptr || pEnemyState == nullptr || pTarget == nullptr) return;

    // 풀링 상태: Z축 뒤로 숨기고 로직 중단
    if (pEnemyState->IsDisabled()) {
        pOwner->position.z = 10.0f;
        pOwner->velocity = { 0, 0, 0 };
        return;
    }

    if (pEnemyState->IsDead()) {
        pOwner->velocity = { 0, 0, 0 };
        
        // 사망 연출 대기 후 풀로 반환
        deathTimer += dt;
        if (deathTimer >= deathDuration) {
            pEnemyState->SetDisabled();
            if (pSpawner) {
                pSpawner->ReturnToPool(pOwner);
            }
        }
        return;
    }

    if (isMovementLocked) {
        pOwner->velocity = { 0, 0, 0 };
        return;
    }

    // 플레이어와의 거리와 방향 계산
    float dx = pTarget->position.x - pOwner->position.x;
    float dy = pTarget->position.y - pOwner->position.y;
    float distance = std::sqrt(dx * dx + dy * dy);
// 거리와 상관없이 항상 플레이어를 향해 이동
if (distance > 0.001f) {
    pEnemyState->SetMove();
    float invDist = 1.0f / distance;
    pOwner->velocity.x = (dx * invDist) * speed;
    pOwner->velocity.y = (dy * invDist) * speed;
}
else {
    pOwner->velocity = { 0, 0, 0 };
}
}
