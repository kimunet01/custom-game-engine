#pragma once

/*
 * EnemySpawner.h
 * 적(Enemy)을 주기적으로 월드에 생성하는 컴포넌트.
 *
 * 주기적으로 새로운 GameObject를 생성하고 필요한 State와 Component를 부착한 뒤
 * GameLoop에 등록한다.
 */

#include "Component.h"
#include <vector>

class GameLoop;
class Mesh;
class Material;
class GameObject;

class EnemySpawner : public Component
{
public:
    EnemySpawner(GameLoop* loop, Mesh* mesh, Material* material, GameObject* player, float enemySpeed = 0.03f);
    void Update(float dt) override;
    void Spawn();
    
    // 적이 비활성화될 때 풀에 다시 넣습니다.
    void ReturnToPool(GameObject* enemy);

private:
    void PreAllocate(int count);
    GameObject* CreateNewEnemyInstance();

    GameLoop* pLoop = nullptr;
    Mesh* pEnemyMesh = nullptr;
    Material* pEnemyMaterial = nullptr;
    GameObject* pPlayer = nullptr;

    float enemySpeed = 0.03f; // 적의 이동 속도 저장
    float timer = 0.0f;
    float interval = 5.0f;
    int enemyCount = 0;

    // 오브젝트 풀링을 위한 관리 목록
    std::vector<GameObject*> inactivePool;
    bool isInitialized = false;
    const int preAllocateCount = 30; // 크래시 방지를 위해 넉넉하게 사전 할당
    };
