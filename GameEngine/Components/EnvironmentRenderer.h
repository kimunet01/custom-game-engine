#pragma once

/*
 * EnvironmentRenderer.h
 * Renders the stage floor and pushes per-frame "environment" data
 * (time / isBossStage / hitPosition) to PS register b1 in TextureShader.hlsl.
 *
 * Used by the StageTerrain GameObject. The mesh and material are owned by
 * main.cpp; this component only manages the GPU constant buffers it creates.
 */

#include <vector>
#include "Component.h"
#include "D3D11ResourceHandler.h"
#include "EngineTypes.h"
#include "Resources/Mesh.h"
#include "Resources/Material.h"

// Layout of the b1 (PS) constant buffer used by TextureShader.hlsl for the
// boss-stage tone and bullet-hit flash effects. Must stay 16-byte aligned for
// D3D11, hence the explicit padding.
struct EnvironmentBufferType {
    float time;                    // Seconds since the boss stage started (drives sin-based flicker)
    int isBossStage;               // 0 = normal, 1 = boss-stage tone enabled
    DirectX::XMFLOAT2 padding;     // 8-byte pad to keep the next field 16-byte aligned
    DirectX::XMFLOAT4 hitPosition; // World-space hit position (currently unused)
};

class EnvironmentRenderer : public Component {
public:
    // External resources — owned by main.cpp, not by this component.
    Mesh* pFloorMesh = nullptr;
    Material* pMaterial = nullptr;

    // GPU constant buffers — owned and released by this component.
    ID3D11Buffer* pMatrixBuffer = nullptr; // b0: world/view/proj matrix
    ID3D11Buffer* pEnvBuffer = nullptr;    // b1: per-stage environment data

    explicit EnvironmentRenderer(Mesh* mesh, Material* mat);
    virtual ~EnvironmentRenderer();

    void Start() override;
    void Render() override;

    // Driven by TerrainStateController (Update / TriggerBossAppearance / ReportWallCollision).
    void UpdateShaderTime(float time);
    void SetBossThemeActive(bool active);
    void EnableFlashEffect(const Vec3& hitPos);
    void DisableFlashEffect();

private:
    // CPU mirror of the b1 buffer. Mutated by the Set/Update/Enable/Disable
    // calls above and uploaded to GPU every Render().
    EnvironmentBufferType m_envData;
};
