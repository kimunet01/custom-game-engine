#pragma once

/*
 * MeshRenderer.h
 * GameObject에 부착되어 Mesh를 화면에 그리는 렌더링 컴포넌트다.
 *
 * MeshRenderer는 Mesh의 vertex buffer와 Material의 셰이더/파라미터를 사용해
 * Draw 호출을 수행한다. 위치와 회전은 pOwner(GameObject)의 transform 값을 읽어
 * MatrixBuffer로 Vertex Shader에 전달한다.
 */

#include <vector>

#include "Component.h"
#include "D3D11ResourceHandler.h"
#include "EngineTypes.h"
#include "Resources/Mesh.h"
#include "Resources/Material.h"

class MeshRenderer : public Component {
public:
    // 이 렌더러가 그릴 Mesh 목록. MeshRenderer는 Mesh를 소유하지 않고 참조만 한다.
    std::vector<Mesh*> meshes;
    // 렌더링에 사용할 셰이더와 머티리얼 파라미터. 소유권은 외부에 있다.
    Material* pMaterial;
    // 오브젝트별 world/view/projection 행렬을 Vertex Shader에 전달하는 상수 버퍼.
    ID3D11Buffer* pMatrixBuffer;
    // 오브젝트별 색상(tint)을 Pixel Shader에 전달하는 상수 버퍼.
    ID3D11Buffer* pColorBuffer;
    // 렌더링 시 적용할 색상 틴트 (기본값: 흰색)
    DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

    explicit MeshRenderer(std::vector<Mesh*> meshes, Material* mat);
    // 렌더러가 사용할 행렬 상수 버퍼를 GPU에 생성한다.
    void Start() override;
    // Material을 바인딩하고 Mesh별 vertex buffer를 그린다.
    void Render() override;
    // 이 컴포넌트가 생성한 행렬 상수 버퍼를 해제한다.
    ~MeshRenderer();
};
