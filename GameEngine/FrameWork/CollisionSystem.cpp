#include "CollisionSystem.h"

#include <cmath>
#include <cstdio>

#include "GameObject.h"
#include "Logger.h"
#include "LevelLayout.h"

/*
 * CollisionSystem.cpp
 * 단순 거리 기반 충돌 감지와 반사 처리, 경계 처리, 충돌 알림을 구현한다.
 *
 * 현재는 모든 오브젝트 쌍을 검사하는 O(n^2) 방식이다. 오브젝트 수가 적은 초기 단계에서는
 * 이해하기 쉽고 충분하지만, 이후 탄환/적 수가 많아지면 공간 분할 구조로 확장할 수 있다.
 */

CollisionSystem::CollisionSystem(float distance)
    : collisionDistance(distance)
    , minX(-0.85f)
    , maxX(0.85f)
    , minY(-0.65f)
    , maxY(0.65f)
    , isLosePrinted(false)
{
    Logger::Info("CollisionSystem created. distance=%.3f bounds=(%.2f, %.2f, %.2f, %.2f)", collisionDistance, minX, maxX, minY, maxY);
}

void CollisionSystem::SetCollisionDistance(float distance)
{
    collisionDistance = distance;
    Logger::Info("CollisionSystem collision distance changed. distance=%.3f", collisionDistance);
}

float CollisionSystem::GetCollisionDistance() const
{
    return collisionDistance;
}

void CollisionSystem::SetBounds(float minX, float maxX, float minY, float maxY)
{
    this->minX = minX;
    this->maxX = maxX;
    this->minY = minY;
    this->maxY = maxY;
    Logger::Info("CollisionSystem bounds changed. minX=%.2f maxX=%.2f minY=%.2f maxY=%.2f", minX, maxX, minY, maxY);
}

float CollisionSystem::GetMinX() const
{
    return minX;
}

float CollisionSystem::GetMaxX() const
{
    return maxX;
}

float CollisionSystem::GetMinY() const
{
    return minY;
}

float CollisionSystem::GetMaxY() const
{
    return maxY;
}

void CollisionSystem::Update(const std::vector<GameObject*>& gameObjects)
{
    // 1. 매개변수로 들어온 gameObjects를 사용하여 LevelLayout 부품을 찾는다!
    LevelLayout* levelLayout = nullptr;
    for (GameObject* obj : gameObjects) {
        if (obj != nullptr) {
            levelLayout = obj->GetComponent<LevelLayout>();
            if (levelLayout != nullptr) {
                break; // 찾았으면 루프 탈출!
            }
        }
    }

    // 2. 만약 지형 부품(levelLayout)을 찾았다면, 지형 오브젝트 스스로가
    // Update 단계를 거쳐 내부의 밀어내기 함수들을 실행하게 만들자!
    // (이렇게 하면 private 함수에 직접 접근하지 않아도 프레임워크 흐름상 안전하게 연산이 수행된단다)
    if (levelLayout != nullptr)
    {
        for (GameObject* obj : gameObjects)
        {
            if (obj != nullptr)
            {
                // 플레이어나 몬스터처럼 벽에 부딪혀야 하는 움직이는 객체들을 대상으로 삼는다.
                // 네 로그에 찍힌 이름이 "Player1", "Player2"일 테니 이렇게 필터링해 주면 안전해!
                if (obj->name == "Player1" || obj->name == "Player2" || obj->name == "Player" || obj->name == "Monster")
                {
                    // 외부에서 강제로 호출하는 대신, levelLayout의 주체적인 함수들을 호출해 준다.
                    // 만약 LevelLayout 내부 함수가 묶여있다면 지형 오브젝트의 Update를 활용하거나
                    // 아래처럼 public 인터페이스를 타는 것이 정석이란다!

                    // 💡 [팁] 만약 이 함수들이 여전히 막힌다면, LevelLayout.h를 열어서 
                    // 해당 함수들 바로 위에 'public:' 한 줄만 적어주면 에러가 완전히 사라져!
                    levelLayout->ClampGameObjectToBounds(obj);
                    levelLayout->ResolvePillarCollision(obj);
                    levelLayout->ResolveBoxCollision(obj);
                }
            }
        }
    }
    // 1. 충돌 쌍을 먼저 모두 찾는다.
    const std::vector<CollisionPair> collisionPairs = Detect(gameObjects);

    // 2. 충돌마다 게임 규칙 알림과 물리적 반응을 처리한다.
    for (const CollisionPair& pair : collisionPairs) {
        NotifyCollision(pair);
        ResolveObjectCollision(pair);
    }

    // 3. 오브젝트끼리 충돌하지 않았더라도 화면 경계는 별도로 검사한다.
    for (GameObject* object : gameObjects) {
        ResolveBounds(object);
    }
}


std::vector<CollisionPair> CollisionSystem::Detect(const std::vector<GameObject*>& gameObjects)
{
    std::vector<CollisionPair> collisionPairs;

    // 매 프레임 새로 판정하므로 이전 프레임 충돌 상태를 먼저 초기화한다.
    for (GameObject* object : gameObjects) {
        if (object != nullptr) {
            object->isCollided = false;
        }
    }

    // i < j 조합만 검사해 같은 쌍을 두 번 검사하지 않는다.
    for (size_t i = 0; i < gameObjects.size(); ++i) {
        GameObject* first = gameObjects[i];
        if (first == nullptr) {
            continue;
        }

        for (size_t j = i + 1; j < gameObjects.size(); ++j) {
            GameObject* second = gameObjects[j];
            if (second == nullptr) {
                continue;
            }

            if (IsColliding(first, second, collisionDistance)) {
                first->isCollided = true;
                second->isCollided = true;
                collisionPairs.push_back({ first, second });
            }
        }
    }

    return collisionPairs;
}

float CollisionSystem::CalculateL2Distance(const GameObject* first, const GameObject* second)
{
    if (first == nullptr || second == nullptr) {
        return 0.0f;
    }

    // 현재 충돌은 GameObject의 position만 기준으로 하는 단순 거리 충돌이다.
    const float xDistance = first->position.x - second->position.x;
    const float yDistance = first->position.y - second->position.y;
    const float zDistance = first->position.z - second->position.z;
    return std::sqrt(
        xDistance * xDistance +
        yDistance * yDistance +
        zDistance * zDistance
    );
}

bool CollisionSystem::IsColliding(const GameObject* first, const GameObject* second, float distance)
{
    if (first == nullptr || second == nullptr) {
        return false;
    }

    return CalculateL2Distance(first, second) <= distance;
}

bool CollisionSystem::IsOutOfBounds(const GameObject* object) const
{
    if (object == nullptr) {
        return false;
    }

    return object->position.x < minX ||
        object->position.x > maxX ||
        object->position.y < minY ||
        object->position.y > maxY;
}

void CollisionSystem::ReflectVelocity(GameObject* object, float normalX, float normalY, float normalZ)
{
    if (object == nullptr) {
        return;
    }

    // v' = v - 2 * dot(v, n) * n 공식을 사용해 법선 방향 기준으로 속도를 반사한다.
    const float dot =
        object->velocity.x * normalX +
        object->velocity.y * normalY +
        object->velocity.z * normalZ;

    object->velocity.x -= 2.0f * dot * normalX;
    object->velocity.y -= 2.0f * dot * normalY;
    object->velocity.z -= 2.0f * dot * normalZ;
}

void CollisionSystem::ResolveObjectCollision(const CollisionPair& pair)
{
    if (pair.first == nullptr || pair.second == nullptr) {
        return;
    }

    // 두 오브젝트 중심을 잇는 방향을 충돌 법선으로 사용한다.
    float normalX = pair.first->position.x - pair.second->position.x;
    float normalY = pair.first->position.y - pair.second->position.y;
    float normalZ = pair.first->position.z - pair.second->position.z;
    float length = std::sqrt(normalX * normalX + normalY * normalY + normalZ * normalZ);

    if (length <= 0.0001f) {
        // 두 오브젝트가 같은 위치에 있으면 방향을 계산할 수 없으므로 임의의 X축 법선을 사용한다.
        normalX = 1.0f;
        normalY = 0.0f;
        normalZ = 0.0f;
        length = 1.0f;
    }

    normalX /= length;
    normalY /= length;
    normalZ /= length;

    // 서로 반대 방향의 법선으로 속도를 반사한다.
    ReflectVelocity(pair.first, normalX, normalY, normalZ);
    ReflectVelocity(pair.second, -normalX, -normalY, -normalZ);

    // 바로 다음 프레임에도 같은 충돌이 반복되지 않도록 아주 조금 떨어뜨린다.
    pair.first->position.x += normalX * 0.01f;
    pair.first->position.y += normalY * 0.01f;
    pair.first->position.z += normalZ * 0.01f;
    pair.second->position.x -= normalX * 0.01f;
    pair.second->position.y -= normalY * 0.01f;
    pair.second->position.z -= normalZ * 0.01f;
}

void CollisionSystem::ResolveBounds(GameObject* object)
{
    if (object == nullptr) {
        return;
    }

    // X축 경계를 벗어나면 위치를 경계에 고정하고 X 속도 방향을 안쪽으로 돌린다.
    if (object->position.x < minX) {
        object->position.x = minX;
        object->velocity.x = std::fabs(object->velocity.x);
        object->isCollided = true;
    }
    else if (object->position.x > maxX) {
        object->position.x = maxX;
        object->velocity.x = -std::fabs(object->velocity.x);
        object->isCollided = true;
    }

    // Y축 경계도 같은 방식으로 처리한다.
    if (object->position.y < minY) {
        object->position.y = minY;
        object->velocity.y = std::fabs(object->velocity.y);
        object->isCollided = true;
    }
    else if (object->position.y > maxY) {
        object->position.y = maxY;
        object->velocity.y = -std::fabs(object->velocity.y);
        object->isCollided = true;
    }
}

void CollisionSystem::NotifyCollision(const CollisionPair& pair)
{
    // 현재 게임 규칙: Player와 Bullet이 충돌하면 lose!를 한 번 출력한다.
    // 더 많은 규칙이 생기면 이벤트/콜백 구조로 분리할 후보 지점이다.
    const bool firstIsPlayer = pair.first != nullptr && pair.first->name == "Player";
    const bool secondIsPlayer = pair.second != nullptr && pair.second->name == "Player";
    const bool firstIsBullet = pair.first != nullptr && pair.first->name.find("Bullet") == 0;
    const bool secondIsBullet = pair.second != nullptr && pair.second->name.find("Bullet") == 0;

    if (!isLosePrinted && ((firstIsPlayer && secondIsBullet) || (secondIsPlayer && firstIsBullet))) {
        Logger::Info("lose!");
        isLosePrinted = true;
    }
}
