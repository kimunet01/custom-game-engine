#pragma once

/*
 * EnvironmentRenderer.h
 * Renders the stage floor and pushes per-frame "environment" data
 * (time / isBossStage / hitPosition) to PS register b1 in TextureShader.hlsl.
 *
 * Used by the StageTerrain GameObject. The mesh and material are owned by
 * main.cpp; this component only manages the GPU constant buffers it creates.
 *
 * Policy: lifecycle(Start/Render) + public data only. External actions
 * (toggling boss tone, setting hit position) are performed by directly
 * mutating `envData` from a free function in StateCallbacks.
 */

#include <vector>
#include "Component.h"
#include "D3D11ResourceHandler.h"
#include "EngineTypes.h"
#include "Resources/Mesh.h"
#include "Resources/Material.h"

// b1 (PS) 상수 버퍼 레이아웃. TextureShader.hlsl의 EnvironmentBuffer와 16바이트 정렬을 맞춘다.
struct EnvironmentBufferType {
    float time;                    // BossStage 진입 후 경과 시간 (sin 깜빡임 입력)
    int isBossStage;               // 0 = 일반, 1 = 보스 톤 활성
    DirectX::XMFLOAT2 padding;     // 16바이트 정렬용 패딩
    DirectX::XMFLOAT4 hitPosition; // 월드 히트 위치 (현재 셰이더에서 미사용)
};

class EnvironmentRenderer : public Component {
public:
    // ── 외부 자원 (소유권은 main) ──
    Mesh* pFloorMesh = nullptr;
    Material* pMaterial = nullptr;

    // ── 본 컴포넌트가 만들고 관리하는 GPU 상수 버퍼 ──
    ID3D11Buffer* pMatrixBuffer = nullptr; // b0
    ID3D11Buffer* pEnvBuffer = nullptr;    // b1

    // ── 콜백이 직접 대입하는 환경 데이터 ──
    // 매 Render에서 pEnvBuffer로 GPU에 업로드된다.
    EnvironmentBufferType envData{};

    explicit EnvironmentRenderer(Mesh* mesh, Material* mat);
    virtual ~EnvironmentRenderer();

    void Start() override;
    void Render() override;
};
