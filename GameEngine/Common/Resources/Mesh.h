#pragma once

/*
 * Mesh.h
 * CPU 정점 데이터와 그 데이터를 GPU에 올린 vertex buffer를 함께 관리하는 리소스 타입이다.
 *
 * Mesh는 어떤 모양을 그릴지에 대한 기하 정보를 담고, MeshRenderer는 이 Mesh를 받아
 * 실제 렌더링 명령을 실행한다. 현재 Vertex는 위치 정보만 가지므로 색상/텍스처 표현은
 * Material과 Shader 쪽에서 결정된다.
 */

#include <vector>

#include "D3D11ResourceHandler.h"
#include "EngineTypes.h"

class Mesh {
public:
    // CPU 메모리에 보관하는 원본 정점 배열.
	std::vector<Vertex> mesh;
    // DirectX input assembler 단계에 바인딩할 GPU vertex buffer.
	ID3D11Buffer* pVertexBuffer;

    // mesh 벡터의 정점 데이터를 GPU vertex buffer로 업로드한다.
	void createVertexBuffer();
    void UpdateVertexBuffer();
    void SetUVRect(float u0, float v0, float u1, float v1);
    // 외부에서 만든 정점 배열을 받아 Mesh의 CPU 데이터로 소유한다.
	explicit Mesh(std::vector<Vertex> vertices);
    // 생성한 vertex buffer를 Release한다.
	~Mesh();
};
