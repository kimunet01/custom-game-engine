#pragma once

#include "D3D11ResourceHandler.h"
#include "../Material.h"

struct ColorBuffer
{
    DirectX::XMFLOAT4 tintColor;
};

class ColorMaterial : public Material {

public:

    DirectX::XMFLOAT4 color;
    ID3D11Buffer* pColorBuffer = nullptr; // 색상 전송용 상수 버퍼


    ColorMaterial(const ShaderSet& s, DirectX::XMFLOAT4 col)
        : Material(s), color(col)
    {
        GraphicsContext* ctx = GraphicsContext::getInstance();
        ID3D11Device* pDevice = ctx->getDevice();
        // 색상 정보를 담을 전용 상수 버퍼 생성 (b1 슬롯용)
        D3D11_BUFFER_DESC cbd = { 0 };
        cbd.Usage = D3D11_USAGE_DEFAULT;
        cbd.ByteWidth = sizeof(ColorBuffer);
        cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

        pDevice->CreateBuffer(&cbd, nullptr, &pColorBuffer);
    }

    virtual ~ColorMaterial()
    {
        if (pColorBuffer) pColorBuffer->Release();
    }

    // 색상을 실시간으로 바꿀 수 있게 제공 (애니메이션용)
    void SetColor(DirectX::XMFLOAT4 col) { color = col; }

    void Bind() override
    {
        GraphicsContext* ctx = GraphicsContext::getInstance();
        ID3D11DeviceContext* pImmediateContext = ctx->getDeviceContext();
        // 1. 셰이더 및 레이아웃 바인딩 (공통)
        pImmediateContext->IASetInputLayout(shaders.layout);
        pImmediateContext->VSSetShader(shaders.vs, nullptr, 0);
        pImmediateContext->PSSetShader(shaders.ps, nullptr, 0);

        // 2. 머티리얼 고유의 색상 데이터 업데이트 (b1 슬롯에 꽂기)
        ColorBuffer cb = { color };
        pImmediateContext->UpdateSubresource(pColorBuffer, 0, nullptr, &cb, 0, 0);

        // Pixel Shader의 1번 슬롯(b1)에 색상 버퍼를 꽂음
        pImmediateContext->PSSetConstantBuffers(1, 1, &pColorBuffer);
    }
};
