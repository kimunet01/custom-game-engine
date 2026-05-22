#pragma once

/*
 * EnemyController.h
 * 적(Enemy)의 AI 로직을 담당하는 컴포넌트.
 *
 * 플레이어와의 거리에 따라 Idle, Chase, Attack 상태를 전환하고
 * 적절한 속도(velocity)를 설정한다.
 */

#include "Component.h"
#include "EnemyState.h"
#include <vector>

class GameObject;
class EnemySpawner;

class EnemyController : public Component
{
public:
    ~EnemyController() override;
    void Start() override;
    void Update(float dt) override;

    void SetTarget(GameObject* target) { pTarget = target; }
    void SetSpawner(EnemySpawner* spawner) { pSpawner = spawner; }
    void SetSpeed(float s) { speed = s; }
    
    // 풀에서 재사용될 때 호출하여 상태를 초기화합니다.
    void Reset();

    // StateCallbacks에서 플래그를 제어할 수 있도록 노출
    bool isMovementLocked = false;
    bool isAttackLocked = false;

private:
    GameObject* pTarget = nullptr;
    EnemyState* pEnemyState = nullptr;
    EnemySpawner* pSpawner = nullptr;

    float speed = 0.03f;
    float attackRange = 0.05f;
    float chaseRange = 0.6f;

    // 공격 후 딜레이 등을 위한 타이머
    float attackTimer = 0.0f;
    float attackDuration = 0.5f;

    // 사망 연출 후 풀로 돌아가기 위한 타이머
    float deathTimer = 0.0f;
    float deathDuration = 2.0f;
};
