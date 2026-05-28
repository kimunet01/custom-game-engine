#pragma once

#include "Component.h"
#include "EngineTypes.h" 
#include <vector>

class GameObject;

// 1. 원형 충돌 구조체
struct PillarObstacle {
    Vec3 position;
    float radius;
};

// 2. 사각형 충돌 구조체
struct LayoutRectObstacle {
    float minX;
    float maxX;
    float minY;
    float maxY;
};

class LevelLayout : public Component {
public:
    explicit LevelLayout();
    virtual ~LevelLayout() = default;

    void Start() override;
    void Update(float dt) override;

    float GetMinX() const { return m_minX; }
    float GetMaxX() const { return m_maxX; }
    float GetMinY() const { return m_minY; }
    float GetMaxY() const { return m_maxY; }

    const std::vector<PillarObstacle>& GetPillars() const { return m_pillars; }
    const std::vector<LayoutRectObstacle>& GetWallBoxes() const { return m_wallBoxes; }

    void ClampGameObjectToBounds(GameObject* obj);
    void ResolvePillarCollision(GameObject* obj);
    void ResolveBoxCollision(GameObject* obj);
private:
    float m_minX;
    float m_maxX;
    float m_minY;
    float m_maxY;

    std::vector<PillarObstacle> m_pillars;
    std::vector<LayoutRectObstacle> m_wallBoxes;

};