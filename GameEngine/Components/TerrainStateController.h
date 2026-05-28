#pragma once

#include <string>
#include "Component.h"
#include "EngineTypes.h" 

// 엔진의 다른 부품들을 제어하기 위한 전방 선언
class LevelLayout;
class EnvironmentRenderer;
class GameObject;

class TerrainStateController : public Component {
public:
    explicit TerrainStateController();
    virtual ~TerrainStateController() = default;

    // Component 가상 함수 오버라이드
    void Start() override;
    void Update(float dt) override;

    // 외부(예: Spawner)에서 보스 등장 신호를 줄 때 호출할 함수
    void TriggerBossAppearance();

    // 외부(예: CollisionSystem)에서 탄환이 벽에 충돌했을 때 좌표를 접수할 함수
    void ReportWallCollision(const Vec3& hitPosition);

    // 상태 조회를 위한 Get 함수들
    bool IsBossStageActive() const { return m_isBossStage; }

private:
    // 지형 관제탑이 실시간으로 부려먹을 하부 인프라 부품 포인터
    LevelLayout* m_levelLayout = nullptr;
    EnvironmentRenderer* m_envRenderer = nullptr;

    // 지형의 전역 상태 변수들
    bool m_isBossStage = false;
    float m_bossStageTimer = 0.0f;
    float m_stageElapsedTime = 0.0f; // 게임 시작 후 흐른 전체 시간을 누적할 타이머

    // 탄환 충돌 연출용 제어 변수
    bool m_isFlashActive = false;
    float m_flashTimer = 0.0f;
    const float m_maxFlashDuration = 0.2f;
    Vec3 m_lastHitPosition;
};