#include "MeshRenderer.h"

#include <directxmath.h>
#include <utility>

#include "GameObject.h"
#include "Logger.h"

/*
 * MeshRenderer.cpp
 * MeshRenderer의 GPU 상수 버퍼 생성과 실제 렌더링 명령 제출을 구현한다.
 *
 * 이 컴포넌트는 Mesh와 Material을 소유하지 않는다. 따라서 Mesh/Material 객체는
 * MeshRenderer가 살아 있는 동안 유지되어야 한다.
 */

MeshRenderer::MeshRenderer(std::vector<Mesh*> meshes, Material* mat)
    : meshes(std::move(meshes))
    , pMatrixBuffer(nullptr)
    , pColorBuffer(nullptr)
    , pMaterial(mat)
{
    Logger::Info("MeshRenderer created. meshCount=%zu hasMaterial=%d", this->meshes.size(), pMaterial != nullptr);
}

void MeshRenderer::Start()
{
    if (pOwner == nullptr) {
        Logger::Warning("MeshRenderer start skipped because owner is null");
        return;
    }
    if (pMaterial == nullptr) {
        Logger::Warning("MeshRenderer start skipped because material is null. owner=%s", pOwner->name.c_str());
        return;
    }
    if (meshes.empty()) {
        Logger::Warning("MeshRenderer start skipped because mesh list is empty. owner=%s", pOwner->name.c_str());
        return;
    }

    GraphicsContext* ctx = GraphicsContext::getInstance();
    ID3D11Device* pd3dDevice = ctx->getDevice();
    if (pd3dDevice == nullptr) {
        Logger::Error("MeshRenderer cannot create buffers because D3D11 device is null. owner=%s", pOwner->name.c_str());
        return;
    }

    // MatrixBufferType은 HLSL의 MatrixBuffer(b0)와 맞춰 오브젝트 변환 행렬을 전달한다.
    D3D11_BUFFER_DESC matrixBufferDesc = {};
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    // 최초 생성 시에는 단위 행렬로 초기화해 안전한 기본 상태를 만든다.
    MatrixBufferType matrixData = {};
    matrixData.worldMatrix = DirectX::XMMatrixIdentity();
    matrixData.viewMatrix = DirectX::XMMatrixIdentity();
    matrixData.projectionMatrix = DirectX::XMMatrixIdentity();

    D3D11_SUBRESOURCE_DATA matrixInitData = {};
    matrixInitData.pSysMem = &matrixData;

    // CreateBuffer가 성공해야 Render에서 VS constant buffer로 바인딩할 수 있다.
    HRESULT hr = pd3dDevice->CreateBuffer(&matrixBufferDesc, &matrixInitData, &pMatrixBuffer);
    if (FAILED(hr) || pMatrixBuffer == nullptr) {
        Logger::Error("MeshRenderer failed to create matrix buffer. owner=%s hr=0x%08X", pOwner->name.c_str(), static_cast<unsigned int>(hr));
        return;
    }

    // ColorBuffer 생성 (PS b1 슬롯용)
    D3D11_BUFFER_DESC colorBufferDesc = {};
    colorBufferDesc.ByteWidth = sizeof(ColorBufferType);
    colorBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    colorBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    ColorBufferType colorData = {};
    colorData.tintColor = color;

    D3D11_SUBRESOURCE_DATA colorInitData = {};
    colorInitData.pSysMem = &colorData;

    hr = pd3dDevice->CreateBuffer(&colorBufferDesc, &colorInitData, &pColorBuffer);
    if (FAILED(hr) || pColorBuffer == nullptr) {
        Logger::Error("MeshRenderer failed to create color buffer. owner=%s hr=0x%08X", pOwner->name.c_str(), static_cast<unsigned int>(hr));
        return;
    }

    isStarted = true;
    Logger::Info("MeshRenderer started. owner=%s meshCount=%zu", pOwner->name.c_str(), meshes.size());
}

void MeshRenderer::Render()
{
    // 머티리얼이나 Mesh가 없으면 그릴 정보가 없으므로 렌더링을 건너뛴다.
    if (!pMaterial || meshes.size() == 0) return;

    GraphicsContext* ctx = GraphicsContext::getInstance();
    ID3D11DeviceContext* pImmediateContext = ctx->getDeviceContext();

    // render할 material(shader, color, texture...) 설정.
    // 여기서 input layout, VS/PS, 색상/텍스처 같은 머티리얼 상태가 GPU에 올라간다.
    pMaterial->Bind();

    if (pOwner == nullptr || pImmediateContext == nullptr || pMatrixBuffer == nullptr || pColorBuffer == nullptr) {
        return;
    }

    // 1. Matrix Buffer 갱신
    MatrixBufferType matrixData = {};
    matrixData.worldMatrix =
        DirectX::XMMatrixRotationZ(pOwner->rotation) *
        DirectX::XMMatrixTranslation(
            pOwner->position.x,
            pOwner->position.y,
            pOwner->position.z
        );
    matrixData.viewMatrix = DirectX::XMMatrixIdentity();
    matrixData.projectionMatrix = DirectX::XMMatrixIdentity();

    pImmediateContext->UpdateSubresource(pMatrixBuffer, 0, nullptr, &matrixData, 0, 0);

    // 2. Color Buffer 갱신
    ColorBufferType colorData = {};
    colorData.tintColor = color;
    pImmediateContext->UpdateSubresource(pColorBuffer, 0, nullptr, &colorData, 0, 0);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    for (auto pMesh : meshes) {
        if (pMesh == nullptr || pMesh->pVertexBuffer == nullptr) {
            continue;
        }

        // Mesh의 vertex buffer를 input assembler에 연결
        pImmediateContext->IASetVertexBuffers(0, 1, &pMesh->pVertexBuffer, &stride, &offset);
        
        // 상수 버퍼 바인딩: VS b0(행렬), PS b2(색상)
        // (PS b1은 EnvironmentBuffer가 사용하므로 b2로 옮김)
        pImmediateContext->VSSetConstantBuffers(0, 1, &pMatrixBuffer);
        pImmediateContext->PSSetConstantBuffers(2, 1, &pColorBuffer);
        
        pImmediateContext->Draw(static_cast<UINT>(pMesh->mesh.size()), 0);
    }
}


MeshRenderer::~MeshRenderer() {
    // 생성한 COM 객체들을 해제한다.
    if (pMatrixBuffer != nullptr) {
        pMatrixBuffer->Release();
    }
    if (pColorBuffer != nullptr) {
        pColorBuffer->Release();
    }
    Logger::Info("MeshRenderer destroyed");
}
