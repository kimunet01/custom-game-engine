#include "MeshRenderer.h"

#include <directxmath.h>
#include <utility>

#include "GameObject.h"

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
    , pMaterial(mat)
{
}

void MeshRenderer::Start()
{
    GraphicsContext* ctx = GraphicsContext::getInstance();
    ID3D11Device* pd3dDevice = ctx->getDevice();
    if (pd3dDevice == nullptr) {
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
    const HRESULT matrixHr = pd3dDevice->CreateBuffer(&matrixBufferDesc, &matrixInitData, &pMatrixBuffer);
    if (FAILED(matrixHr) || pMatrixBuffer == nullptr) {
        return;
    }

    isStarted = true;
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

    if (pOwner == nullptr || pImmediateContext == nullptr || pMatrixBuffer == nullptr) {
        return;
    }

    // 현재 GameObject의 회전과 위치를 world matrix로 변환한다.
    // 지금 구조에서는 scale이 없으므로 rotation과 translation만 적용한다.
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

    // 매 프레임 변하는 오브젝트 행렬을 constant buffer에 갱신한다.
    pImmediateContext->UpdateSubresource(
        pMatrixBuffer,
        0,
        nullptr,
        &matrixData,
        0,
        0
    );

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    for (auto pMesh : meshes) {
        if (pMesh == nullptr || pMesh->pVertexBuffer == nullptr) {
            continue;
        }

        // Mesh의 vertex buffer를 input assembler에 연결하고, 행렬 버퍼를 VS b0 슬롯에 연결한다.
        pImmediateContext->IASetVertexBuffers(0, 1, &pMesh->pVertexBuffer, &stride, &offset);
        pImmediateContext->VSSetConstantBuffers(0, 1, &pMatrixBuffer);
        pImmediateContext->Draw(static_cast<UINT>(pMesh->mesh.size()), 0);
    }
}


MeshRenderer::~MeshRenderer() {
    // pMatrixBuffer는 MeshRenderer가 직접 만든 COM 객체이므로 여기서 Release한다.
    if (pMatrixBuffer != nullptr) {
        pMatrixBuffer->Release();
    }
}
