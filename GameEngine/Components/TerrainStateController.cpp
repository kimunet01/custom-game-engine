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

    // GameObject(예: 지형 오브젝트)로부터 하부 부품들을 가져온다.
    if (pOwner != nullptr)
    {
        m_levelLayout = pOwner->GetComponent<LevelLayout>();
        m_envRenderer = pOwner->GetComponent<EnvironmentRenderer>();

        if (m_levelLayout == nullptr) {
            Logger::Error("TerrainStateController: LevelLayout 부품을 찾을 수 없습니다!");
        }
        if (m_envRenderer == nullptr) {
            Logger::Error("TerrainStateController: EnvironmentRenderer 부품을 찾을 수 없습니다!");
        }
    }
}

void TerrainStateController::Update(float dt)
{
    // 1. 게임 시작 후 흐른 실시간(DeltaTime)을 매 프레임 누적
    m_stageElapsedTime += dt;

    // 2. [조건 체크] 5초가 지났고, 아직 보스 스테이지 상태가 아니라면?
    if (m_stageElapsedTime >= 5.0f && !m_isBossStage)
    {
        // 이미 기가 막히게 만들어둔 보스 등장 함수를 실행한다!
        // 이 안에서 m_isBossStage = true로 바뀌고, 렌더러에게 SetBossThemeActive(true)(붉은색) 신호를 보낸단다.
        TriggerBossAppearance();
    }

    // 3. 보스 등장 상태라면 시간을 누적하여 렌더러에게 전달 (기존 코드 유지)
    if (m_isBossStage)
    {
        m_bossStageTimer += dt;
        if (m_envRenderer != nullptr)
        {
            // 렌더러에게 누적 시간을 계속 주입하여 셰이더 상수 버퍼가 울렁거리게 만듦
            m_envRenderer->UpdateShaderTime(m_bossStageTimer);
        }
    }

    // 4. 탄환 충돌 섬광 타이머 제어 (기존 코드 유지)
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
    if (m_isBossStage == true) return; // 이미 켜져있다면 무시

    m_isBossStage = true;
    m_bossStageTimer = 0.0f;
    Logger::Info("TerrainStateController: 보스 등장 상태 감지! 맵 테마 전환");

    // 환경 렌더러에게 배경색을 붉은색으로 바꾸라고 지시(Write)
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

    // 환경 렌더러에게 이 좌표에 탄환 박혔으니까 노랗게 바꾸라고 충돌 좌표 전달(Write)
    if (m_envRenderer != nullptr)
    {
        m_envRenderer->EnableFlashEffect(m_lastHitPosition);
    }
}