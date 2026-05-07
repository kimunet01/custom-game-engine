#pragma once

#include <vector>

class GameObject;

struct CollisionPair {
    GameObject* first;
    GameObject* second;
};

class CollisionSystem {
public:
    explicit CollisionSystem(float distance = 0.2f);

    void SetCollisionDistance(float distance);
    float GetCollisionDistance() const;
    void SetBounds(float minX, float maxX, float minY, float maxY);
    float GetMinX() const;
    float GetMaxX() const;
    float GetMinY() const;
    float GetMaxY() const;

    void Update(const std::vector<GameObject*>& gameObjects);
    std::vector<CollisionPair> Detect(const std::vector<GameObject*>& gameObjects);

    static float CalculateL2Distance(const GameObject* first, const GameObject* second);
    static bool IsColliding(const GameObject* first, const GameObject* second, float distance);
    bool IsOutOfBounds(const GameObject* object) const;

private:
    float collisionDistance;
    float minX;
    float maxX;
    float minY;
    float maxY;
    bool isLosePrinted;

    static void ReflectVelocity(GameObject* object, float normalX, float normalY, float normalZ);
    void ResolveObjectCollision(const CollisionPair& pair);
    void ResolveBounds(GameObject* object);
    void NotifyCollision(const CollisionPair& pair);
};
