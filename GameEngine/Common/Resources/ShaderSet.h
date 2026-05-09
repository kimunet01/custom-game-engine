#pragma once
/*
 * ShaderSet.h
 * 한 렌더링 패스에서 함께 사용되는 vertex shader, pixel shader, input layout을 묶는다.
 *
 * DirectX COM 객체는 참조 카운트 기반으로 관리되므로, 이 타입은 복사/이동/소멸 시
 * AddRef와 Release를 직접 호출해 리소스 수명을 맞춘다. Material은 ShaderSet을 보관하고
 * Bind 단계에서 실제 파이프라인에 바인딩한다.
 */

#include "EngineTypes.h"

class ShaderSet {
public:
    // 정점 변환을 담당하는 vertex shader.
	ID3D11VertexShader* vs = nullptr;
    // 픽셀 색상 출력을 담당하는 pixel shader.
	ID3D11PixelShader* ps = nullptr;
    // CPU Vertex 구조와 HLSL 입력 의미론을 연결하는 input layout.
	ID3D11InputLayout* layout = nullptr;

    // 현재 사용되지 않는 placeholder 선언이다. 실제 vertex buffer 생성은 Mesh가 담당한다.
	void createVertexBuffer();
	explicit ShaderSet(ID3D11VertexShader* v, ID3D11PixelShader* p, ID3D11InputLayout* l);
	ShaderSet();
    // 복사 시 같은 COM 리소스를 공유하므로 AddRef로 참조 카운트를 증가시킨다.
	ShaderSet(const ShaderSet& other);
	ShaderSet& operator=(const ShaderSet& other);
    // 이동 시 포인터 소유권을 넘기고 원본은 nullptr로 비운다.
	ShaderSet(ShaderSet&& other) noexcept;
	ShaderSet& operator=(ShaderSet&& other) noexcept;
	~ShaderSet();

private:
    // 보유 중인 COM 객체들의 참조 카운트를 증가시킨다.
	void AddRefResources();
    // 보유 중인 COM 객체들을 Release하고 포인터를 nullptr로 정리한다.
	void ReleaseResources();
};
