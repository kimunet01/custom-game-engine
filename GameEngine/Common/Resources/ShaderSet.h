#pragma once
#include "EngineTypes.h"

class ShaderSet {
public:
	ID3D11VertexShader* vs = nullptr;
	ID3D11PixelShader* ps = nullptr;
	ID3D11InputLayout* layout = nullptr;

	void createVertexBuffer();
	explicit ShaderSet(ID3D11VertexShader* v, ID3D11PixelShader* p, ID3D11InputLayout* l);
	ShaderSet();
	ShaderSet(const ShaderSet& other);
	ShaderSet& operator=(const ShaderSet& other);
	ShaderSet(ShaderSet&& other) noexcept;
	ShaderSet& operator=(ShaderSet&& other) noexcept;
	~ShaderSet();

private:
	void AddRefResources();
	void ReleaseResources();
};
