#pragma once

#include <vector>
#include "Component.h"
#include "D3D11ResourceHandler.h"
#include "EngineTypes.h"
#include "Resources/Mesh.h"
#include "Resources/Material.h"

// 픽셀 셰이더 b1 슬롯에 매핑할 지형 연출 전용 상수 버퍼 구조체
// DirectX11 구조상 16바이트 정렬 크기를 만족해야 하므로 패딩을 추가합니다.
struct EnvironmentBufferType {
    float time;                    // 바닥 일렁거림용 누적 시간
    int isBossStage;               // 보스 스테이지 활성화 여부
    DirectX::XMFLOAT2 padding;     // 16바이트 정렬을 위한 패딩 (8바이트)
    DirectX::XMFLOAT4 hitPosition; // 탄환이 벽에 충돌한 월드 정밀 좌표 (16바이트)
};

class EnvironmentRenderer : public Component {
public:
    // 지형 렌더러가 참조할 메쉬 및 재질 자원 (소유권은 외부 main.cpp 측에 있음)
    Mesh* pFloorMesh = nullptr;
    Material* pMaterial = nullptr;

    // GPU 자원 관리를 위한 상수 버퍼 포인터들
    ID3D11Buffer* pMatrixBuffer = nullptr; // b0: 월드 변환 행렬 버퍼
    ID3D11Buffer* pEnvBuffer = nullptr;    // b1: 지형 환경 연출 데이터 버퍼

    explicit EnvironmentRenderer(Mesh* mesh, Material* mat);
    virtual ~EnvironmentRenderer();

    // Component 기저 클래스 수명주기 가상 함수 오버라이드
    void Start() override;
    void Render() override;

    // TerrainStateController가 프레임 루프(Update/Trigger/Report)에서 실시간으로 호출할 제어 인터페이스
    void UpdateShaderTime(float time);
    void SetBossThemeActive(bool active);
    void EnableFlashEffect(const Vec3& hitPos);
    void DisableFlashEffect();

private:
    // 실시간으로 값이 변경되어 GPU 상수 버퍼로 전송될 호스트 측 데이터 버퍼
    EnvironmentBufferType m_envData;
};