#pragma once

/*
 * TerrainStateController.h
 * Drives stage-wide environment state on the StageTerrain GameObject.
 *
 * Responsibilities:
 *  - Tracks elapsed time and triggers the boss stage after a fixed delay
 *    (currently 5s — TODO: link to the actual boss GameObject's LifeState).
 *  - Forwards time to EnvironmentRenderer so the shader's flicker animates.
 *  - Exposes ReportWallCollision so future bullet code can flash the stage on impact.
 *    (No caller wires this yet — kept as a hook for future bullet/impact work.)
 */

#include <string>
#include "Component.h"
#include "EngineTypes.h"

class LevelLayout;
class EnvironmentRenderer;
class GameObject;

class TerrainStateController : public Component {
public:
    explicit TerrainStateController();
    virtual ~TerrainStateController() = default;

    void Start() override;
    void Update(float dt) override;

    // Manually switch the stage to its boss theme (otherwise auto-triggered at 5s).
    void TriggerBossAppearance();

    // Hook for future bullet/projectile code to flash the stage at a hit point.
    // Currently unused — no caller wired in main.cpp.
    void ReportWallCollision(const Vec3& hitPosition);

    bool IsBossStageActive() const { return m_isBossStage; }

private:
    // Sibling components on the same GameObject (StageTerrain).
    LevelLayout* m_levelLayout = nullptr;
    EnvironmentRenderer* m_envRenderer = nullptr;

    // Boss-stage state.
    bool m_isBossStage = false;
    float m_bossStageTimer = 0.0f;        // Seconds since boss stage started — drives the shader's flicker.
    float m_stageElapsedTime = 0.0f;      // Seconds since stage start — used to auto-trigger boss.

    // Hit-flash state (currently inert because ReportWallCollision has no caller).
    bool m_isFlashActive = false;
    float m_flashTimer = 0.0f;
    const float m_maxFlashDuration = 0.2f;
    Vec3 m_lastHitPosition;
};
