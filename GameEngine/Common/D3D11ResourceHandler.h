#pragma once

#include "EngineTypes.h"
#include "Resources/ShaderSet.h"
#include <cstddef>

HRESULT compileShader(const void* pSrc, bool isFile, LPCSTR szEntry, LPCSTR szTarget, ID3DBlob** ppBlob);

class GraphicsContext {
private:
    // singleton management
    static GraphicsContext* pInstance;
    GraphicsContext();

    GraphicsContext(const GraphicsContext&) = delete;
    GraphicsContext& operator=(const GraphicsContext&) = delete;

    HWND hWnd;
    ID3D11Device* pd3dDevice;
    ID3D11DeviceContext* pImmediateContext;
    IDXGISwapChain* pSwapChain;
    ID3D11RenderTargetView* pRenderTargetView;

public:
    static GraphicsContext* getInstance();
    static void Release();

    HWND getHWND();
    ID3D11Device* getDevice();
    ID3D11DeviceContext* getDeviceContext();
    IDXGISwapChain* getSwapChain();
    ID3D11RenderTargetView* getRTV();

    void createDeviceAndSwapChainAndRTV(int width, int height);
    void createWindow(HINSTANCE hInstance, int nCmdShow, const wchar_t* winClassName, int width, int height);
    void RebuildVideoResource();
    ShaderSet CompileAndCreate(const void* source, std::size_t length, bool isFile, D3D11_INPUT_ELEMENT_DESC* ied, UINT iedCount);
    void CleanUp();

    ~GraphicsContext();
};
