#pragma once

/*
 * CollisionSystem.h
 * 게임 오브젝트 간의 단순 원형 충돌 검사와 충돌 반응을 담당하는 시스템이다.
 *
 * 현재 충돌은 각 GameObject의 position 사이 거리가 collisionDistance 이하인지로 판단한다.
 * 충돌한 오브젝트는 속도를 반사하고, 화면 경계 밖으로 나간 오브젝트는 경계 안으로 되돌린다.
 */

#include <vector>

class GameObject;

struct CollisionPair {
    // 충돌한 두 GameObject 포인터. 실제 소유권은 GameLoop/gameWorld 쪽에 있다.
    GameObject* first;
    GameObject* second;
};

class CollisionSystem {
public:
    explicit CollisionSystem(float distance = 0.2f);

    // 충돌 거리와 월드 경계를 외부에서 조정하기 위한 설정 함수들.
    void SetCollisionDistance(float distance);
    float GetCollisionDistance() const;
    void SetBounds(float minX, float maxX, float minY, float maxY);
    float GetMinX() const;
    float GetMaxX() const;
    float GetMinY() const;
    float GetMaxY() const;

    // 한 프레임의 전체 충돌 처리. 감지, 알림, 반응, 경계 보정을 순서대로 수행한다.
    void Update(const std::vector<GameObject*>& gameObjects);
    // 모든 오브젝트 쌍을 검사해 충돌 쌍 목록을 반환한다.
    std::vector<CollisionPair> Detect(const std::vector<GameObject*>& gameObjects);

    // 두 오브젝트 사이의 유클리드 거리 계산과 충돌 여부 판정 helper.
    static float CalculateL2Distance(const GameObject* first, const GameObject* second);
    static bool IsColliding(const GameObject* first, const GameObject* second, float distance);
    bool IsOutOfBounds(const GameObject* object) const;

private:
    // 오브젝트끼리 이 거리 이하로 가까워지면 충돌했다고 판단한다.
    float collisionDistance;
    // 오브젝트가 움직일 수 있는 화면/월드 경계.
    float minX;
    float maxX;
    float minY;
    float maxY;
    // Player와 Bullet 충돌 시 lose 메시지를 한 번만 출력하기 위한 상태.
    bool isLosePrinted;

    // 충돌 법선 방향을 기준으로 속도를 반사한다.
    static void ReflectVelocity(GameObject* object, float normalX, float normalY, float normalZ);
    // 오브젝트끼리 겹쳤을 때 서로 밀어내고 속도를 반사한다.
    void ResolveObjectCollision(const CollisionPair& pair);
    // 월드 경계를 벗어난 오브젝트를 경계 안으로 되돌리고 속도를 반사한다.
    void ResolveBounds(GameObject* object);
    // 게임 규칙에 필요한 충돌 알림을 처리한다.
    void NotifyCollision(const CollisionPair& pair);
};
