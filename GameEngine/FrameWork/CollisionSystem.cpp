#include "CollisionSystem.h"

#include <cmath>
#include <cstdio>

#include "GameObject.h"

CollisionSystem::CollisionSystem(float distance)
    : collisionDistance(distance)
    , minX(-0.85f)
    , maxX(0.85f)
    , minY(-0.65f)
    , maxY(0.65f)
    , isLosePrinted(false)
{
}

void CollisionSystem::SetCollisionDistance(float distance)
{
    collisionDistance = distance;
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
    const std::vector<CollisionPair> collisionPairs = Detect(gameObjects);

    for (const CollisionPair& pair : collisionPairs) {
        NotifyCollision(pair);
        ResolveObjectCollision(pair);
    }

    for (GameObject* object : gameObjects) {
        ResolveBounds(object);
    }
}

std::vector<CollisionPair> CollisionSystem::Detect(const std::vector<GameObject*>& gameObjects)
{
    std::vector<CollisionPair> collisionPairs;

    for (GameObject* object : gameObjects) {
        if (object != nullptr) {
            object->isCollided = false;
        }
    }

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

    float normalX = pair.first->position.x - pair.second->position.x;
    float normalY = pair.first->position.y - pair.second->position.y;
    float normalZ = pair.first->position.z - pair.second->position.z;
    float length = std::sqrt(normalX * normalX + normalY * normalY + normalZ * normalZ);

    if (length <= 0.0001f) {
        normalX = 1.0f;
        normalY = 0.0f;
        normalZ = 0.0f;
        length = 1.0f;
    }

    normalX /= length;
    normalY /= length;
    normalZ /= length;

    ReflectVelocity(pair.first, normalX, normalY, normalZ);
    ReflectVelocity(pair.second, -normalX, -normalY, -normalZ);

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
    const bool firstIsPlayer = pair.first != nullptr && pair.first->name == "Player";
    const bool secondIsPlayer = pair.second != nullptr && pair.second->name == "Player";
    const bool firstIsBullet = pair.first != nullptr && pair.first->name.find("Bullet") == 0;
    const bool secondIsBullet = pair.second != nullptr && pair.second->name.find("Bullet") == 0;

    if (!isLosePrinted && ((firstIsPlayer && secondIsBullet) || (secondIsPlayer && firstIsBullet))) {
        std::printf("lose!\n");
        isLosePrinted = true;
    }
}
