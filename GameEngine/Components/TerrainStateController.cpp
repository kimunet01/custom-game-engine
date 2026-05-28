#include "TerrainStateController.h"
#include "GameObject.h"
#include "LevelLayout.h"
#include "EnvironmentRenderer.h"
#include "Logger.h"

TerrainStateController::TerrainStateController()
    : m_isBossStage(false)
    , m_bossStageTimer(0.0f)
    , m_isFlashActive(false)
    , m_flashTimer(0.0f)
    , m_stageElapsedTime(0.0f)
{
}

void TerrainStateController::Start()
{
    Component::Start();

    if (pOwner != nullptr)
    {
        m_levelLayout = pOwner->GetComponent<LevelLayout>();
        m_envRenderer = pOwner->GetComponent<EnvironmentRenderer>();

        if (m_levelLayout == nullptr) {
            Logger::Error("TerrainStateController: LevelLayout sibling not found on owner=%s",
                          pOwner->name.c_str());
        }
        if (m_envRenderer == nullptr) {
            Logger::Error("TerrainStateController: EnvironmentRenderer sibling not found on owner=%s",
                          pOwner->name.c_str());
        }
    }
}

void TerrainStateController::Update(float dt)
{
    // 1) Accumulate stage time.
    m_stageElapsedTime += dt;

    // 2) Auto-trigger the boss stage after 5 seconds.
    //    TODO: replace with a LifeState/spawn-event hook so the tone change
    //    aligns with the actual Boss GameObject lifecycle.
    if (m_stageElapsedTime >= 5.0f && !m_isBossStage)
    {
        TriggerBossAppearance();
    }

    // 3) While in the boss stage, keep feeding time to the shader so the
    //    initial flicker fades out (g_time-driven sin wave in TextureShader.hlsl).
    if (m_isBossStage)
    {
        m_bossStageTimer += dt;
        if (m_envRenderer != nullptr)
        {
            m_envRenderer->UpdateShaderTime(m_bossStageTimer);
        }
    }

    // 4) Hit-flash countdown (no-op until ReportWallCollision has callers).
    if (m_isFlashActive)
    {
        m_flashTimer += dt;
        if (m_flashTimer >= m_maxFlashDuration)
        {
            m_isFlashActive = false;
            m_flashTimer = 0.0f;
            if (m_envRenderer != nullptr)
            {
                m_envRenderer->DisableFlashEffect();
            }
        }
    }
}

void TerrainStateController::TriggerBossAppearance()
{
    if (m_isBossStage) return; // Already active.

    m_isBossStage = true;
    m_bossStageTimer = 0.0f;
    Logger::Info("TerrainStateController: boss stage started — switching tone");

    if (m_envRenderer != nullptr)
    {
        m_envRenderer->SetBossThemeActive(true);
    }
}

void TerrainStateController::ReportWallCollision(const Vec3& hitPosition)
{
    m_isFlashActive = true;
    m_flashTimer = 0.0f;
    m_lastHitPosition = hitPosition;

    if (m_envRenderer != nullptr)
    {
        m_envRenderer->EnableFlashEffect(m_lastHitPosition);
    }
}
