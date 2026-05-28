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
    // 초기 기본 안전 데이터 셋업
    m_envData.time = 0.0f;
    m_envData.isBossStage = 0;
    m_envData.hitPosition = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

    Logger::Info("EnvironmentRenderer created. hasMesh=%d hasMaterial=%d",
        pFloorMesh != nullptr, pMaterial != nullptr);
}

void EnvironmentRenderer::Start()
{
    // 안전 장치 예외 처리 구문
    if (pOwner == nullptr) {
        Logger::Warning("EnvironmentRenderer: 부모 GameObject(Owner)가 null이므로 초기화를 건너뜁니다.");
        return;
    }
    if (pMaterial == nullptr || pFloorMesh == nullptr) {
        Logger::Warning("EnvironmentRenderer: 자원(Mesh/Material)이 불완전합니다. owner=%s", pOwner->name.c_str());
        return;
    }

    GraphicsContext* ctx = GraphicsContext::getInstance();
    ID3D11Device* pd3dDevice = ctx->getDevice();
    if (pd3dDevice == nullptr) {
        Logger::Error("EnvironmentRenderer: D3D11 Device가 null입니다. owner=%s", pOwner->name.c_str());
        return;
    }

    // 1. 월드/뷰/투영 행렬 상수 버퍼 생성 (b0 슬롯용)
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
        Logger::Error("EnvironmentRenderer: Matrix Buffer 생성 실패. hr=0x%08X", hr);
        return;
    }

    // 2. 지형 전역 연출 전용 상수 버퍼 생성 (b1 슬롯용)
    D3D11_BUFFER_DESC envBufferDesc = {};
    envBufferDesc.ByteWidth = sizeof(EnvironmentBufferType); // 16바이트 정렬 구조체 크기
    envBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    envBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    D3D11_SUBRESOURCE_DATA envInitData = {};
    envInitData.pSysMem = &m_envData;

    hr = pd3dDevice->CreateBuffer(&envBufferDesc, &envInitData, &pEnvBuffer);
    if (FAILED(hr)) {
        Logger::Error("EnvironmentRenderer: Environment Shader Buffer 생성 실패. hr=0x%08X", hr);
        if (pMatrixBuffer) pMatrixBuffer->Release();
        return;
    }

    isStarted = true;
    Logger::Info("EnvironmentRenderer started successfully. owner=%s", pOwner->name.c_str());
}

void EnvironmentRenderer::Render()
{
    // 유효성 검사 실패 시 드로우 렌더링을 즉시 건너뜁니다.
    if (!isStarted || pMaterial == nullptr || pFloorMesh == nullptr || pFloorMesh->pVertexBuffer == nullptr) {
        return;
    }

    GraphicsContext* ctx = GraphicsContext::getInstance();
    ID3D11DeviceContext* pImmediateContext = ctx->getDeviceContext();
    if (pImmediateContext == nullptr || pMatrixBuffer == nullptr || pEnvBuffer == nullptr) {
        return;
    }

    // 1. 바닥 전용 셰이더 및 텍스처 파이프라인 바인딩
    pMaterial->Bind();

    // 2. 바닥 변환 행렬 버퍼(b0) 데이터 업로드 (지형 배경은 고정 격자이므로 회전 없이 원점 배치)
    MatrixBufferType matrixData = {};
    matrixData.worldMatrix = DirectX::XMMatrixIdentity(); // 단위 행렬
    matrixData.viewMatrix = DirectX::XMMatrixIdentity();
    matrixData.projectionMatrix = DirectX::XMMatrixIdentity();

    pImmediateContext->UpdateSubresource(pMatrixBuffer, 0, nullptr, &matrixData, 0, 0);

    // 3. 지형 연출 버퍼(b1) 실시간 갱신 데이터 업로드 (Time 및 충돌 좌표 수치 반영)
    pImmediateContext->UpdateSubresource(pEnvBuffer, 0, nullptr, &m_envData, 0, 0);

    // 4. 입력 가속기(IA) 및 정점 셰이더 행렬 버퍼 세팅
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    pImmediateContext->IASetVertexBuffers(0, 1, &pFloorMesh->pVertexBuffer, &stride, &offset);
    pImmediateContext->VSSetConstantBuffers(0, 1, &pMatrixBuffer);

    // 5. ★ [핵심 파이프라인] 픽셀 셰이더(PS) b1 슬롯에 환경 연출 버퍼를 최종 동킹시킨다!
    // 이를 통해 HLSL 소스 안에서 시간 변수와 충돌 위치 데이터 연산이 가능해집니다.
    pImmediateContext->PSSetConstantBuffers(1, 1, &pEnvBuffer);

    // 6. 인스턴싱 최종 드로우 콜 명령 제출
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
    // HLSL float4 타입 변환 매핑 연산 (W 수치는 1.0f로 공간 좌표 설정)
    m_envData.hitPosition = DirectX::XMFLOAT4(hitPos.x, hitPos.y, hitPos.z, 1.0f);
}

void EnvironmentRenderer::DisableFlashEffect()
{
    m_envData.hitPosition = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
}

EnvironmentRenderer::~EnvironmentRenderer()
{
    // 내가 직접 뉴(CreateBuffer)해서 할당받은 GPU COM 자원들을 안전하게 릭 없이 해제한다.
    if (pMatrixBuffer != nullptr) {
        pMatrixBuffer->Release();
        pMatrixBuffer = nullptr;
    }
    if (pEnvBuffer != nullptr) {
        pEnvBuffer->Release();
        pEnvBuffer = nullptr;
    }
    Logger::Info("EnvironmentRenderer 자원이 정상 해제(Released) 되었습니다.");
}