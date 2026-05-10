#include "TextureMaterial.h"

#include <cstdint>
#include <vector>
#include <wincodec.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "windowscodecs.lib")

#include "Utils.h"

TextureMaterial::TextureMaterial(const ShaderSet& s, const wchar_t* texturePath)
    : Material(s)
{
    LoadTextureFromFile(texturePath);

    GraphicsContext* ctx = GraphicsContext::getInstance();
    ID3D11Device* pDevice = ctx->getDevice();
    if (pDevice == nullptr) {
        return;
    }

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    pDevice->CreateSamplerState(&samplerDesc, &pSamplerState);

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    pDevice->CreateBlendState(&blendDesc, &pBlendState);
}

TextureMaterial::~TextureMaterial()
{
    SafeRelease(pBlendState);
    SafeRelease(pSamplerState);
    SafeRelease(pTextureView);
}

void TextureMaterial::Bind()
{
    GraphicsContext* ctx = GraphicsContext::getInstance();
    ID3D11DeviceContext* pImmediateContext = ctx->getDeviceContext();
    if (pImmediateContext == nullptr) {
        return;
    }

    pImmediateContext->IASetInputLayout(shaders.layout);
    pImmediateContext->VSSetShader(shaders.vs, nullptr, 0);
    pImmediateContext->PSSetShader(shaders.ps, nullptr, 0);
    pImmediateContext->PSSetShaderResources(0, 1, &pTextureView);
    pImmediateContext->PSSetSamplers(0, 1, &pSamplerState);
    pImmediateContext->OMSetBlendState(pBlendState, nullptr, 0xffffffff);
}

bool TextureMaterial::LoadTextureFromFile(const wchar_t* texturePath)
{
    HRESULT coHr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const bool shouldUninitialize = SUCCEEDED(coHr);
    if (FAILED(coHr) && coHr != RPC_E_CHANGED_MODE) {
        return false;
    }

    IWICImagingFactory* pFactory = nullptr;
    IWICBitmapDecoder* pDecoder = nullptr;
    IWICBitmapFrameDecode* pFrame = nullptr;
    IWICFormatConverter* pConverter = nullptr;
    ID3D11Texture2D* pTexture = nullptr;

    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pFactory)
    );

    if (SUCCEEDED(hr)) {
        hr = pFactory->CreateDecoderFromFilename(
            texturePath,
            nullptr,
            GENERIC_READ,
            WICDecodeMetadataCacheOnLoad,
            &pDecoder
        );
    }

    if (SUCCEEDED(hr)) {
        hr = pDecoder->GetFrame(0, &pFrame);
    }

    UINT width = 0;
    UINT height = 0;
    if (SUCCEEDED(hr)) {
        hr = pFrame->GetSize(&width, &height);
    }

    if (SUCCEEDED(hr)) {
        hr = pFactory->CreateFormatConverter(&pConverter);
    }

    if (SUCCEEDED(hr)) {
        hr = pConverter->Initialize(
            pFrame,
            GUID_WICPixelFormat32bppRGBA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.0,
            WICBitmapPaletteTypeCustom
        );
    }

    std::vector<std::uint8_t> pixels;
    if (SUCCEEDED(hr)) {
        pixels.resize(static_cast<size_t>(width) * height * 4);
        hr = pConverter->CopyPixels(
            nullptr,
            width * 4,
            static_cast<UINT>(pixels.size()),
            pixels.data()
        );
    }

    GraphicsContext* ctx = GraphicsContext::getInstance();
    ID3D11Device* pDevice = ctx->getDevice();
    if (SUCCEEDED(hr) && pDevice == nullptr) {
        hr = E_FAIL;
    }

    if (SUCCEEDED(hr)) {
        D3D11_TEXTURE2D_DESC textureDesc = {};
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA textureData = {};
        textureData.pSysMem = pixels.data();
        textureData.SysMemPitch = width * 4;

        hr = pDevice->CreateTexture2D(&textureDesc, &textureData, &pTexture);
    }

    if (SUCCEEDED(hr)) {
        hr = pDevice->CreateShaderResourceView(pTexture, nullptr, &pTextureView);
    }

    SafeRelease(pTexture);
    SafeRelease(pConverter);
    SafeRelease(pFrame);
    SafeRelease(pDecoder);
    SafeRelease(pFactory);

    if (shouldUninitialize) {
        CoUninitialize();
    }

    return SUCCEEDED(hr) && pTextureView != nullptr;
}
