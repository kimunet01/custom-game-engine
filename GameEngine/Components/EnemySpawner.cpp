#include "EnemySpawner.h"
#include "EnemyController.h"
#include "EnemyState.h"
#include "AttackState.h"
#include "LifeState.h"
#include "MovementState.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "Logger.h"
#include "MeshRenderer.h"
#include "SpriteAnimator.h"
#include "VelocityController.h"
#include "Resources/Mesh.h"
#include "LevelLayout.h"

EnemySpawner::EnemySpawner(GameLoop* loop, Mesh* mesh, Material* material, GameObject* player, float speed, int type)
    : pLoop(loop), pEnemyMesh(mesh), pEnemyMaterial(material), pPlayer(player), enemySpeed(speed), enemyType(type)
{
}

void EnemySpawner::Start()
{
    if (pOwner) {
        // 스포너 자체도 Z축 뒤로 숨깁니다 (CollisionSystem의 방해 방지)
        pOwner->position = { 0.0f, 0.0f, 10.0f };
    }
    isStarted = true;
}

void EnemySpawner::Update(float dt)
{
    // [보호] 첫 프레임 폭주 방지: deltaTime이 너무 크면 업데이트를 건너뜜니다.
    if (dt > 0.5f) return;

    timer += dt;
    if (timer >= interval) {
        timer = 0.0f;
        Spawn();
    }
}

void EnemySpawner::PreAllocate(int count)
{
    for (int i = 0; i < count; ++i) {
        GameObject* enemy = CreateNewEnemyInstance();
        inactivePool.push_back(enemy);
        pLoop->AddGameObject(enemy);
    }
    Logger::Info("EnemySpawner: Pre-allocated %d enemies", count);
}

GameObject* EnemySpawner::CreateNewEnemyInstance()
{
    std::string name = "Enemy_" + std::to_string(++enemyCount);
    GameObject* enemy = new GameObject(name);

    // [초기 위치 은닉] 생성 직후 화면에 나타나는 것을 방지하기 위해 카메라 뒤쪽으로 보냅니다.
    // CollisionSystem은 X, Y 축만 감시하여 경계 안으로 끌어오므로, 감시하지 않는 Z축을 10.0f로 설정하여 숨깁니다.
    enemy->position = { 0.0f, 0.0f, 10.0f };
    enemy->velocity = { 0.0f, 0.0f, 0.0f };

    // 1. 적 전용 State만 등록
    enemy->AddState(new EnemyState());

    // 2. Component 등록
    EnemyController* controller = new EnemyController();
    controller->SetTarget(pPlayer);
    controller->SetSpawner(this);
    controller->SetSpeed(enemySpeed);
    
    // 특수 스킬(Dash) 값들 주입
    controller->enemyType = this->enemyType;
    controller->dashRange = this->dashRange;
    controller->dashPrepTime = this->dashPrepTime;
    controller->dashSpeed = this->dashSpeed;
    controller->dashDuration = this->dashDuration;
    
    enemy->AddComponent(controller);

    enemy->AddComponent(new VelocityController());

    Mesh* enemyMesh = new Mesh(pEnemyMesh->mesh);
    enemyMesh->createVertexBuffer();

    SpriteAnimator* animator = new SpriteAnimator(enemyMesh);
    // orc1_run_full.png 격자 구조: 8열 4행
    // 사용자 지정 순서: Down(0행), Up(1행), Left(2행), Right(3행)
    animator->AddClip("move_down",  8, 4, 0,  8, 0.10f);
    animator->AddClip("move_up",    8, 4, 8,  8, 0.10f);
    animator->AddClip("move_left",  8, 4, 16, 8, 0.10f);
    animator->AddClip("move_right", 8, 4, 24, 8, 0.10f);
    
    // Dash 스킬용 클립 연동 (에셋에 별도 동작이 없으므로 기본 move_down 활용)
    animator->AddClip("dash_prep",  8, 4, 0,  1, 0.10f); // 첫 프레임 정지
    animator->AddClip("dashing",    8, 4, 0,  8, 0.05f); // 빠르게 재생
    
    animator->AddClip("dead",       8, 4, 0,  1, 0.50f, false);
    animator->AddClip("disabled",   8, 4, 0,  1, 1.0f);

    enemy->AddComponent(animator);
    enemy->AddComponent(new MeshRenderer({ enemyMesh }, pEnemyMaterial));

    // 풀에 들어갈 때는 항상 Disabled 상태
    EnemyState* state = enemy->GetState<EnemyState>();
    if (state) state->SetDisabled();

    return enemy;
}

void EnemySpawner::Spawn()
{
    if (pLoop == nullptr || pEnemyMesh == nullptr || pEnemyMaterial == nullptr) return;

    GameObject* enemy = nullptr;
    if (!inactivePool.empty()) {
        enemy = inactivePool.back();
        inactivePool.pop_back();
    }
    else {
        // [Crash 방지] 풀이 비어있으면 게임 도중 강제로 추가 생성하지 않고 스폰을 한 턴 쉴니다.
        Logger::Warning("EnemySpawner: Pool is empty! Skipping spawn to prevent crash.");
        return;
    }

    // 활성화 시 위치 설정 (Z를 0.0f로 맞춰서 플레이어와 동일 평면에서 충돌 판정이 일어나게 함)
    // [Gemini CLI 수정] 벽 위에 스폰되지 않도록 LevelLayout을 찾아 위치를 검증합니다.
    LevelLayout* layout = nullptr;
    if (pLoop) {
        for (auto obj : pLoop->gameWorld) {
            if (obj) {
                layout = obj->GetComponent<LevelLayout>();
                if (layout) break;
            }
        }
    }

    float spawnX = 0.0f;
    float spawnY = 0.0f;
    bool validPosition = false;
    int retryCount = 0;

    while (!validPosition && retryCount < 10) {
        spawnX = static_cast<float>(rand() % 200 - 100) / 100.0f;
        spawnY = static_cast<float>(rand() % 200 - 100) / 100.0f;
        
        if (layout) {
            if (!layout->IsPositionBlocked(spawnX, spawnY)) {
                validPosition = true;
            }
        } else {
            validPosition = true; // 레이아웃을 못 찾으면 어쩔 수 없이 스폰
        }
        retryCount++;
    }

    enemy->position.x = spawnX;
    enemy->position.y = spawnY;
    enemy->position.z = 0.0f; 
    enemy->velocity = { 0, 0, 0 };

    EnemyController* controller = enemy->GetComponent<EnemyController>();
    if (controller) controller->Reset();

    EnemyState* state = enemy->GetState<EnemyState>();
    if (state) state->SetMove(EnemyStateType::MoveDown); // 소환 즉시 기본 방향(아래)으로 시작
}

void EnemySpawner::ReturnToPool(GameObject* enemy)
{
    if (enemy == nullptr) return;
    // 비활성화: CollisionSystem의 경계 보정 로직을 피하기 위해 Z축 멀리 숨깁니다.
    enemy->position.z = 10.0f;
    inactivePool.push_back(enemy);
    Logger::Info("EnemySpawner: Enemy returned to pool: %s", enemy->name.c_str());
}
