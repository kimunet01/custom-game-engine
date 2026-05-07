#include "GameLoop.h"
#include <thread>

GameLoop::GameLoop()
{
    Initialize();
}

// GameLoop owns the objects registered in the world.
GameLoop::~GameLoop()
{
    for (GameObject* object : gameWorld) {
        delete object;
    }
}

// Initialize base loop state.
void GameLoop::Initialize()
{
    isRunning = true;
    prevTime = std::chrono::high_resolution_clock::now();
    deltaTime = 0.0f;
}

// Register an object in the world.
void GameLoop::AddGameObject(GameObject* object)
{
    gameWorld.push_back(object);
}

// Input phase:
// 1. Drain the Win32 message queue.
// 2. Let components read the cached input state updated by WndProc.
void GameLoop::Input()
{
    MSG msg = {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            isRunning = false;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    for (GameObject* object : gameWorld) {
        for (auto component : object->components) {
            component->Input();
        }
    }
}

void GameLoop::Update()
{
    // 컴포넌트 Start
    for (GameObject* object : gameWorld) {
        for (auto component : object->components) {
            if (!component->isStarted) {
                component->Start();
            }
        }
    }

    // 컴포넌트 Update
    for (GameObject* object : gameWorld) {
        for (auto component : object->components) {
            component->Update(deltaTime);
        }
    }

    // 충돌 검사
    collisionSystem.Update(gameWorld);
}

// Render phase:
// Clear the back buffer, set shared pipeline state, then render each object.
void GameLoop::Render()
{
    GraphicsContext* ctx = GraphicsContext::getInstance();
    ID3D11DeviceContext* pImmediateContext = ctx->getDeviceContext();
    ID3D11RenderTargetView* pRenderTargetView = ctx->getRTV();
    IDXGISwapChain* pSwapChain = ctx->getSwapChain();

    float clearColor[] = { 0.1f, 0.2f, 0.3f, 1.0f };
    pImmediateContext->ClearRenderTargetView(pRenderTargetView, clearColor);

    pImmediateContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);

    //RECT clientRect = {};
    //GetClientRect(hWnd, &clientRect);

    // Reset the viewport to the configured window size.
    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<FLOAT>(videoConfig.Width);
    viewport.Height = static_cast<FLOAT>(videoConfig.Height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    pImmediateContext->RSSetViewports(1, &viewport);
    pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Let each object render its own components.
    for (GameObject* object : gameWorld) {
        for (auto component : object->components) {
            component->Render();
        }
    }

    pSwapChain->Present(1, 0);
}

// Main loop.
// Each frame runs Input -> Update -> Render after calculating deltaTime.
void GameLoop::Run()
{
    while (isRunning) {
        const auto currentTime = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<float> elapsed = currentTime - prevTime;
        deltaTime = elapsed.count();
        prevTime = currentTime;

        Input();
        Update();
        Render();
    }
}
