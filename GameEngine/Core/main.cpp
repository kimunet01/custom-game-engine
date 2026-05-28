/*
 * main.cpp
 * Entry point and sample scene assembly.
 */

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <vector>

#include "D3D11ResourceHandler.h"
#include "AttackController.h"
#include "AttackState.h"
#include "MovementState.h"
#include "EngineTypes.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "LifeState.h"
#include "Logger.h"
#include "MeshRenderer.h"
#include "PlayerControl.h"
#include "EnemySpawner.h"
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
    Logger::Info("Application started");
    GraphicsContext* ctx = GraphicsContext::getInstance();

    D3D11_INPUT_ELEMENT_DESC textureIed[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    ctx->createWindow(hInstance, nCmdShow, L"test", videoConfig.Width, videoConfig.Height);
    ctx->createDeviceAndSwapChainAndRTV(videoConfig.Width, videoConfig.Height);

    // 1. 플레이어 자원 설정
    Mesh playerMesh(CreateSpriteQuadMesh(0.16f, 0.18f, 0.0f, 0.3f, 0.1f, 0.4f));
    playerMesh.createVertexBuffer();

    const wchar_t* textureShaderPath = L"Common\\Resources\\Shaders\\TextureShader.hlsl";
    ShaderSet textureShaders = ctx->CompileAndCreate(textureShaderPath, 0, true, textureIed, 2);
    TextureMaterial* playerMaterial = new TextureMaterial(textureShaders, L"assets\\chmov.png");

    // 2. 적(Enemy) 자원 설정
    // 적의 크기를 여기서 조절합니다 (예: 0.15f)
    Mesh enemyMesh(CreateSpriteQuadMesh(0.15f, 0.18f, 0.0f, 0.0f, 1.0f, 1.0f));
    enemyMesh.createVertexBuffer();
    TextureMaterial* enemyMaterial = new TextureMaterial(textureShaders, L"assets\\orc1_run_full.png");
    TextureMaterial* enemyMaterialOrc2 = new TextureMaterial(textureShaders, L"assets\\orc2_run_full.png");

    GameLoop loop;
    loop.collisionSystem.SetBounds(-0.85f, 0.85f, -0.65f, 0.65f);
    // 충돌 반경을 0.06f로 줄여서 더 정밀한 충돌 판정을 적용합니다.
    loop.collisionSystem.SetCollisionDistance(0.06f);

    // 3. 플레이어 생성
    GameObject* player = new GameObject("Player");
    // State는 Component가 아닌 데이터 단위. GameObject의 states 컬렉션에 등록한다.
    // 콜백을 구독하는 PlayerControl/SpriteAnimator보다 먼저 등록되어야 Start() 시점에 GetState로 발견된다.
    player->AddState(new AttackState());
    player->AddState(new LifeState());
    player->AddState(new MovementState());
    // AttackController는 AttackState를 조작하므로 State보다 뒤, 그리고 자신을 참조하는 PlayerControl보다 앞에 등록한다.
    player->AddComponent(new AttackController());
    player->AddComponent(new PlayerControl(0));
    player->AddComponent(new VelocityController());
    SpriteAnimator* animator = new SpriteAnimator(&playerMesh);
    animator->AddClip("stand_left", 10, 10, 0, 1, 0.12f, false);
    animator->AddClip("stand_right", 10, 10, 10, 1, 0.12f, false);
    animator->AddClip("stand_up", 10, 10, 20, 1, 0.12f, false);
    animator->AddClip("stand_down", 10, 10, 30, 1, 0.12f, false);
    animator->AddClip("walk_left", 10, 10, 0, 8, 0.10f);
    animator->AddClip("walk_right", 10, 10, 10, 8, 0.10f);
    animator->AddClip("walk_up", 10, 10, 20, 8, 0.10f);
    animator->AddClip("walk_down", 10, 10, 30, 8, 0.10f);
    animator->AddClip("sword_attack_down", 10, 10, 40, 5, 0.08f, false);
    animator->AddClip("sword_attack_up", 10, 10, 50, 5, 0.08f, false);
    animator->AddClip("sword_attack_right", 10, 10, 60, 6, 0.08f, false);
    animator->AddClip("sword_attack_left", 10, 10, 70, 6, 0.08f, false);
    animator->AddClip("dead", 10, 10, 81, 1, 0.12f, false);
    player->AddComponent(animator);
    player->AddComponent(new MeshRenderer({ &playerMesh }, playerMaterial));
    loop.AddGameObject(player);

    // 4. 에너미 스포너 생성 (기본형 - Orc1)
    GameObject* spawnerObj1 = new GameObject("EnemySpawner1");
    // 타입 0 (기본), 속도 0.04f
    EnemySpawner* spawner1 = new EnemySpawner(&loop, &enemyMesh, enemyMaterial, player, 0.04f, 0);
    spawnerObj1->AddComponent(spawner1);
    loop.AddGameObject(spawnerObj1);
    // [Crash 방지] 루프 시작 전 미리 풀을 생성합니다.
    spawner1->PreAllocate(30);

    // 5. 에너미 스포너 생성 (돌진형 - Orc2)
    GameObject* spawnerObj2 = new GameObject("EnemySpawner2");
    // 타입 1 (돌진형 탑재), 기본 속도는 조금 느린 0.03f
    EnemySpawner* dashSpawner = new EnemySpawner(&loop, &enemyMesh, enemyMaterialOrc2, player, 0.03f, 1);
    
    // [사용자 커스터마이징 영역]
    // 엔진 좌표계(-1.0 ~ 1.0)에 맞게 환산된 수치입니다. 자유롭게 조절하세요.
    dashSpawner->dashRange = 0.3f;     // 기획의 300.0f에 해당하는 화면 내 적정 거리 (화면의 약 절반)
    dashSpawner->dashSpeed = 0.4f;    // 기획의 10.0f에 해당하는 빠른 속도
    dashSpawner->dashPrepTime = 0.5f;  // 돌진 전 제자리 정지 시간 (0.5초)
    dashSpawner->dashDuration = 0.5f;  // 돌진이 유지되는 시간
    
    spawnerObj2->AddComponent(dashSpawner);
    loop.AddGameObject(spawnerObj2);
    // [Crash 방지] 루프 시작 전 미리 풀을 생성합니다.
    dashSpawner->PreAllocate(30);

    loop.Run();

    Logger::Info("Application shutting down");
    ctx->CleanUp();
    return 0;
}
