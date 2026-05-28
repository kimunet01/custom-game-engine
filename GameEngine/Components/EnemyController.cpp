#include "EnemyController.h"
#include "GameObject.h"
#include "Logger.h"
#include "StateCallbacks.h"
#include "SpriteAnimator.h"
#include "MeshRenderer.h"
#include "EnemySpawner.h"
#include "LevelLayout.h"
#include "GameLoop.h"
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
    hasDashed = false;
    dashTimer = 0.0f;

    // 풀링 재사용 시 비주얼 상태 초기화
    if (pOwner) {
        MeshRenderer* renderer = pOwner->GetComponent<MeshRenderer>();
        if (renderer) renderer->color = { 1.0f, 1.0f, 1.0f, 1.0f };

        SpriteAnimator* animator = pOwner->GetComponent<SpriteAnimator>();
        if (animator) animator->isPaused = false;
    }
}

void EnemyController::Start()
{
    if (pOwner == nullptr) return;

    // [Gemini CLI] 벽 인식 및 우회를 위해 LevelLayout을 찾습니다.
    if (pSpawner && pSpawner->pLoop) {
        for (auto obj : pSpawner->pLoop->gameWorld) {
            if (obj) {
                pLayout = obj->GetComponent<LevelLayout>();
                if (pLayout) break;
            }
        }
    }

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

    // --- 대쉬 스킬 처리 ---
    EnemyStateType currentState = pEnemyState->Get();
    
    if (currentState == EnemyStateType::DashPrep) {
        pOwner->velocity = { 0, 0, 0 }; // 0.5초 정지
        dashTimer += dt;

        // 1. 애니메이션 일시 정지 (기존 방향 유지)
        SpriteAnimator* animator = pOwner->GetComponent<SpriteAnimator>();
        if (animator) animator->isPaused = true;

        // 2. 색상 연출 (연한 노랑 -> 진한 노랑)
        MeshRenderer* renderer = pOwner->GetComponent<MeshRenderer>();
        if (renderer) {
            float progress = dashTimer / dashPrepTime;
            if (progress > 1.0f) progress = 1.0f;
            
            // White(1,1,1) -> Yellow(1,1,0)
            renderer->color.x = 1.0f;
            renderer->color.y = 1.0f;
            renderer->color.z = 1.0f - progress;
            renderer->color.w = 1.0f;
        }

        if (dashTimer >= dashPrepTime) {
            // 애니메이션 재개 및 색상 원복
            if (animator) animator->isPaused = false;
            if (renderer) renderer->color = { 1.0f, 1.0f, 1.0f, 1.0f };

            // 플레이어 방향 벡터 고정 및 Dashing 상태 진입
            float dx = pTarget->position.x - pOwner->position.x;
            float dy = pTarget->position.y - pOwner->position.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance > 0.001f) {
                dashDirX = dx / distance;
                dashDirY = dy / distance;
            } else {
                dashDirX = 1.0f; dashDirY = 0.0f;
            }
            dashTimer = 0.0f;
            pEnemyState->SetMove(EnemyStateType::Dashing);
        }
        return;
    }
    
    if (currentState == EnemyStateType::Dashing) {
        pOwner->velocity.x = dashDirX * dashSpeed;
        pOwner->velocity.y = dashDirY * dashSpeed;
        dashTimer += dt;
        
        if (dashTimer >= dashDuration) {
            // 대쉬 종료 -> 기본 추적 상태 복귀
            hasDashed = true;
            pEnemyState->SetMove(EnemyStateType::MoveDown);
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

    // 대쉬 발동 조건 검사 (Orc2 타입 && 거리 내 진입 && 안 씀)
    if (enemyType == 1 && !hasDashed && distance < dashRange) {
        dashTimer = 0.0f;
        pEnemyState->SetMove(EnemyStateType::DashPrep);
        return;
    }

    // [Gemini CLI] 우회 이동 로직 (Simple Obstacle Avoidance)
    float moveX = 0.0f;
    float moveY = 0.0f;

    if (distance > 0.001f) {
        float baseDirX = dx / distance;
        float baseDirY = dy / distance;

        // 이동할 방향 후보들 (기본 방향, +-45도, +-90도)
        float angles[] = { 0.0f, 0.785f, -0.785f, 1.57f, -1.57f };
        bool foundPath = false;

        for (float angle : angles) {
            float cs = std::cos(angle);
            float sn = std::sin(angle);
            float testDirX = baseDirX * cs - baseDirY * sn;
            float testDirY = baseDirX * sn + baseDirY * cs;

            // 해당 방향으로 조금 앞을 미리 체크
            float checkDist = 0.1f;
            float checkX = pOwner->position.x + testDirX * checkDist;
            float checkY = pOwner->position.y + testDirY * checkDist;

            if (pLayout == nullptr || !pLayout->IsPositionBlocked(checkX, checkY)) {
                moveX = testDirX * speed;
                moveY = testDirY * speed;
                foundPath = true;
                break;
            }
        }

        if (!foundPath) {
            moveX = 0.0f;
            moveY = 0.0f;
        }
    }

    pOwner->velocity.x = moveX;
    pOwner->velocity.y = moveY;

    // 속도 방향에 따라 애니메이션 상태 결정
    if (std::abs(pOwner->velocity.x) > std::abs(pOwner->velocity.y)) {
        if (pOwner->velocity.x > 0) pEnemyState->SetMove(EnemyStateType::MoveRight);
        else pEnemyState->SetMove(EnemyStateType::MoveLeft);
    }
    else if (std::abs(pOwner->velocity.y) > 0.001f) {
        if (pOwner->velocity.y > 0) pEnemyState->SetMove(EnemyStateType::MoveUp);
        else pEnemyState->SetMove(EnemyStateType::MoveDown);
    }
    else {
        // 이동하지 않을 때는 이전 상태 유지 혹은 기본 방향
    }
}
