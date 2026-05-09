#include "Mesh.h"

#include <utility>

/*
 * Mesh.cpp
 * Mesh의 생성, GPU vertex buffer 생성, 리소스 해제를 구현한다.
 *
 * MeshRenderer는 Render 단계에서 pVertexBuffer를 직접 사용하므로, Mesh::createVertexBuffer는
 * 렌더링 전에 한 번 호출되어 있어야 한다.
 */

Mesh::Mesh(std::vector<Vertex> vertices) {
    // 전달받은 정점 배열을 복사하지 않고 move하여 Mesh가 소유한다.
	mesh = std::move(vertices);
	pVertexBuffer = nullptr;
}

Mesh::~Mesh() {
    // DirectX COM 객체는 new/delete가 아니라 Release로 참조 카운트를 줄인다.
	if (pVertexBuffer != nullptr) {
		pVertexBuffer->Release();
	}
}

void Mesh::createVertexBuffer() {
	GraphicsContext* ctx = GraphicsContext::getInstance();
	ID3D11Device* pd3dDevice = ctx->getDevice();
	if (pd3dDevice == nullptr || mesh.empty()) {
		return;
	}

    // 정점 배열 전체 크기만큼 GPU 버퍼를 만들고 vertex buffer 용도로 지정한다.
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * mesh.size());
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    // 초기 데이터는 CPU에 있는 mesh 벡터의 연속 메모리를 그대로 사용한다.
	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = mesh.data();

    // 생성에 실패하면 pVertexBuffer는 nullptr 상태로 남고, MeshRenderer가 렌더링을 건너뛴다.
	const HRESULT hr = pd3dDevice->CreateBuffer(&bufferDesc, &initData, &pVertexBuffer);
	if (FAILED(hr) || pVertexBuffer == nullptr) {
		return;
	}
}
