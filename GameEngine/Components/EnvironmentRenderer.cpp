#include "EnvironmentRenderer.h"
#include <directxmath.h>
#include "GameObject.h"
#include "Logger.h"

EnvironmentRenderer::EnvironmentRenderer(Mesh* mesh, Material* mat)
    : pFloorMesh(mesh)
    , pMaterial(mat)
    , pMatrixBuffer(nullptr)
    , pEnvBuffer(nullptr)
    , m_envData{}
{
    // Default state: no boss tone, no flash.
    m_envData.time = 0.0f;
    m_envData.isBossStage = 0;
    m_envData.hitPosition = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

    Logger::Info("EnvironmentRenderer created. hasMesh=%d hasMaterial=%d",
                 pFloorMesh != nullptr, pMaterial != nullptr);
}

void EnvironmentRenderer::Start()
{
    if (pOwner == nullptr) {
        Logger::Warning("EnvironmentRenderer: owner is null, skipping init");
        return;
    }
    if (pMaterial == nullptr || pFloorMesh == nullptr) {
        Logger::Warning("EnvironmentRenderer: missing mesh or material. owner=%s", pOwner->name.c_str());
        return;
    }

    GraphicsContext* ctx = GraphicsContext::getInstance();
    ID3D11Device* pd3dDevice = ctx->getDevice();
    if (pd3dDevice == nullptr) {
        Logger::Error("EnvironmentRenderer: D3D11 device is null. owner=%s", pOwner->name.c_str());
        return;
    }

    // 1) b0 matrix constant buffer (world/view/proj).
    D3D11_BUFFER_DESC matrixBufferDesc = {};
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    MatrixBufferType matrixData = {};
    matrixData.worldMatrix = DirectX::XMMatrixIdentity();
    matrixData.viewMatrix = DirectX::XMMatrixIdentity();
    matrixData.projectionMatrix = DirectX::XMMatrixIdentity();

    D3D11_SUBRESOURCE_DATA matrixInitData = {};
    matrixInitData.pSysMem = &matrixData;

    HRESULT hr = pd3dDevice->CreateBuffer(&matrixBufferDesc, &matrixInitData, &pMatrixBuffer);
    if (FAILED(hr)) {
        Logger::Error("EnvironmentRenderer: CreateBuffer(matrix) failed. hr=0x%08X", hr);
        return;
    }

    // 2) b1 environment constant buffer (time / boss flag / hit position).
    D3D11_BUFFER_DESC envBufferDesc = {};
    envBufferDesc.ByteWidth = sizeof(EnvironmentBufferType);
    envBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    envBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    D3D11_SUBRESOURCE_DATA envInitData = {};
    envInitData.pSysMem = &m_envData;

    hr = pd3dDevice->CreateBuffer(&envBufferDesc, &envInitData, &pEnvBuffer);
    if (FAILED(hr)) {
        Logger::Error("EnvironmentRenderer: CreateBuffer(env) failed. hr=0x%08X", hr);
        if (pMatrixBuffer) pMatrixBuffer->Release();
        return;
    }

    isStarted = true;
    Logger::Info("EnvironmentRenderer started. owner=%s", pOwner->name.c_str());
}

void EnvironmentRenderer::Render()
{
    if (!isStarted || pMaterial == nullptr || pFloorMesh == nullptr || pFloorMesh->pVertexBuffer == nullptr) {
        return;
    }

    GraphicsContext* ctx = GraphicsContext::getInstance();
    ID3D11DeviceContext* pImmediateContext = ctx->getDeviceContext();
    if (pImmediateContext == nullptr || pMatrixBuffer == nullptr || pEnvBuffer == nullptr) {
        return;
    }

    // 1) Bind shader/texture/sampler.
    pMaterial->Bind();

    // 2) Upload b0 matrix (identity world for a screen-filling stage quad).
    MatrixBufferType matrixData = {};
    matrixData.worldMatrix = DirectX::XMMatrixIdentity();
    matrixData.viewMatrix = DirectX::XMMatrixIdentity();
    matrixData.projectionMatrix = DirectX::XMMatrixIdentity();
    pImmediateContext->UpdateSubresource(pMatrixBuffer, 0, nullptr, &matrixData, 0, 0);

    // 3) Upload b1 environment data (time / boss tone / hit position).
    pImmediateContext->UpdateSubresource(pEnvBuffer, 0, nullptr, &m_envData, 0, 0);

    // 4) Bind input assembler and VS constant buffer.
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    pImmediateContext->IASetVertexBuffers(0, 1, &pFloorMesh->pVertexBuffer, &stride, &offset);
    pImmediateContext->VSSetConstantBuffers(0, 1, &pMatrixBuffer);

    // 5) Bind PS b1 to the environment buffer. HLSL reads g_time / g_isBossStage / g_hitPosition.
    pImmediateContext->PSSetConstantBuffers(1, 1, &pEnvBuffer);

    // 6) Draw the stage quad.
    pImmediateContext->Draw(static_cast<UINT>(pFloorMesh->mesh.size()), 0);
}

void EnvironmentRenderer::UpdateShaderTime(float time)
{
    m_envData.time = time;
}

void EnvironmentRenderer::SetBossThemeActive(bool active)
{
    m_envData.isBossStage = active ? 1 : 0;
}

void EnvironmentRenderer::EnableFlashEffect(const Vec3& hitPos)
{
    // W=1 marks a valid hit position to the shader (currently unused there).
    m_envData.hitPosition = DirectX::XMFLOAT4(hitPos.x, hitPos.y, hitPos.z, 1.0f);
}

void EnvironmentRenderer::DisableFlashEffect()
{
    m_envData.hitPosition = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
}

EnvironmentRenderer::~EnvironmentRenderer()
{
    if (pMatrixBuffer != nullptr) {
        pMatrixBuffer->Release();
        pMatrixBuffer = nullptr;
    }
    if (pEnvBuffer != nullptr) {
        pEnvBuffer->Release();
        pEnvBuffer = nullptr;
    }
    Logger::Info("EnvironmentRenderer destroyed (GPU buffers released).");
}
