#pragma once

/*
 * D3D11ResourceHandler.h
 * DirectX 11의 핵심 그래픽스 리소스를 생성하고 공유하는 GraphicsContext를 선언한다.
 *
 * 이 클래스는 Win32 창 핸들, D3D11 device/context, swap chain, render target view를
 * 엔진 전체에서 접근할 수 있게 관리한다. 또한 HLSL 셰이더를 컴파일하고 ShaderSet으로
 * 묶어 반환하는 편의 함수를 제공한다.
 */

#include "EngineTypes.h"
#include "Resources/ShaderSet.h"
#include <cstddef>

// 단일 셰이더 엔트리 포인트를 컴파일하는 낮은 수준의 helper 함수.
// 현재는 GraphicsContext::CompileAndCreate가 더 높은 수준의 생성 흐름을 담당한다.
HRESULT compileShader(const void* pSrc, bool isFile, LPCSTR szEntry, LPCSTR szTarget, ID3DBlob** ppBlob);

class GraphicsContext {
private:
    // 엔진 전역에서 하나의 그래픽스 컨텍스트만 사용하기 위한 싱글톤 인스턴스.
    static GraphicsContext* pInstance;
    GraphicsContext();

    // DirectX 리소스 소유권이 복제되면 Release 타이밍이 꼬일 수 있으므로 복사를 금지한다.
    GraphicsContext(const GraphicsContext&) = delete;
    GraphicsContext& operator=(const GraphicsContext&) = delete;

    // Win32 창과 DirectX 11 렌더링에 필요한 핵심 핸들/인터페이스.
    HWND hWnd;
    ID3D11Device* pd3dDevice;
    ID3D11DeviceContext* pImmediateContext;
    IDXGISwapChain* pSwapChain;
    ID3D11RenderTargetView* pRenderTargetView;

public:
    // 싱글톤 인스턴스를 가져온다. 최초 호출 시 내부에서 생성된다.
    static GraphicsContext* getInstance();
    // 싱글톤 인스턴스 자체를 파괴한다. 보통 프로그램 종료 시점에서만 사용한다.
    static void Release();

    // 외부 시스템이 DirectX 리소스에 접근할 때 사용하는 getter 모음.
    HWND getHWND();
    ID3D11Device* getDevice();
    ID3D11DeviceContext* getDeviceContext();
    IDXGISwapChain* getSwapChain();
    ID3D11RenderTargetView* getRTV();

    // 현재 HWND를 기준으로 D3D11 device, swap chain, render target view를 생성한다.
    void createDeviceAndSwapChainAndRTV(int width, int height);
    // Win32 창 클래스를 등록하고 실제 렌더링 창을 생성한다.
    void createWindow(HINSTANCE hInstance, int nCmdShow, const wchar_t* winClassName, int width, int height);
    // 해상도 변경 시 back buffer와 render target view를 새 크기에 맞춰 재생성한다.
    void RebuildVideoResource();
    // Vertex/Pixel Shader를 컴파일하고 input layout까지 묶어 ShaderSet으로 반환한다.
    ShaderSet CompileAndCreate(const void* source, std::size_t length, bool isFile, D3D11_INPUT_ELEMENT_DESC* ied, UINT iedCount);
    // GraphicsContext가 직접 보유한 DirectX 리소스를 Release한다.
    void CleanUp();

    ~GraphicsContext();
};
