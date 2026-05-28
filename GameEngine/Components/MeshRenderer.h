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
    // per-instance tint(r,g,b,a)를 Pixel Shader b1 슬롯에 전달하는 상수 버퍼.
    // Material(텍스처/셰이더)은 인스턴스 사이 공유될 수 있어야 하므로 tint는 MeshRenderer 측에 둔다.
    ID3D11Buffer* pTintBuffer;
    // 현재 인스턴스의 tint 값. HitReactionController 같은 외부 컴포넌트가 SetTint로 수정한다.
    // 기본 (1,1,1,1)은 텍스처 색상 그대로를 의미한다.
    Vec4 tint;

    explicit MeshRenderer(std::vector<Mesh*> meshes, Material* mat);
    // 렌더러가 사용할 행렬/tint 상수 버퍼를 GPU에 생성한다.
    void Start() override;
    // Material을 바인딩하고 Mesh별 vertex buffer를 그린다.
    void Render() override;
    // 외부에서 tint를 설정한다. 값은 다음 Render에서 GPU에 반영된다.
    void SetTint(float r, float g, float b, float a);
    // 이 컴포넌트가 생성한 상수 버퍼들을 해제한다.
    ~MeshRenderer();
};
