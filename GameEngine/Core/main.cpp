/*
 * main.cpp
 * Entry point and sample scene assembly.
 */

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <vector>

#include "D3D11ResourceHandler.h"
#include "EngineTypes.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "MeshRenderer.h"
#include "PlayerControl.h"
#include "Resources/Materials/TextureMaterial.h"
#include "Resources/Mesh.h"
#include "SpriteAnimator.h"
#include "VelocityController.h"
#include "Win32Handler.h"

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

KeyState localKeyState;
VideoConfig videoConfig;

namespace {
std::vector<Vertex> CreateSpriteQuadMesh(float width, float height, float u0, float v0, float u1, float v1)
{
    const float halfWidth = width * 0.5f;
    const float halfHeight = height * 0.5f;

    return {
        { -halfWidth,  halfHeight, 0.5f, u0, v0 },
        {  halfWidth,  halfHeight, 0.5f, u1, v0 },
        {  halfWidth, -halfHeight, 0.5f, u1, v1 },

        { -halfWidth,  halfHeight, 0.5f, u0, v0 },
        {  halfWidth, -halfHeight, 0.5f, u1, v1 },
        { -halfWidth, -halfHeight, 0.5f, u0, v1 }
    };
}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    GraphicsContext* ctx = GraphicsContext::getInstance();

    D3D11_INPUT_ELEMENT_DESC textureIed[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ctx->createWindow(hInstance, nCmdShow, L"test", videoConfig.Width, videoConfig.Height);
    ctx->createDeviceAndSwapChainAndRTV(videoConfig.Width, videoConfig.Height);

    Mesh playerMesh(CreateSpriteQuadMesh(0.16f, 0.18f, 0.0f, 0.0f, 0.125f, 0.111111f));
    playerMesh.createVertexBuffer();

    const wchar_t* textureShaderPath = L"Common\\Resources\\Shaders\\TextureShader.hlsl";
    ShaderSet textureShaders = ctx->CompileAndCreate(textureShaderPath, 0, true, textureIed, 2);
    TextureMaterial* playerMaterial = new TextureMaterial(textureShaders, L"assets\\chmov.png");

    GameLoop loop;
    loop.collisionSystem.SetBounds(-0.85f, 0.85f, -0.65f, 0.65f);

    GameObject* player = new GameObject("Player");
    player->AddComponent(new PlayerControl(0));
    player->AddComponent(new VelocityController());
    SpriteAnimator* animator = new SpriteAnimator(&playerMesh);
    animator->AddClip("idle", 8, 9, 0, 8, 0.12f);
    animator->Play("idle");
    player->AddComponent(animator);
    player->AddComponent(new MeshRenderer({ &playerMesh }, playerMaterial));
    loop.AddGameObject(player);

    loop.Run();

    ctx->CleanUp();
    return 0;
}
