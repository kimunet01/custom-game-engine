/*
 * main.cpp
 * 프로그램 진입점과 현재 샘플 씬 조립을 담당한다.
 *
 * 이 파일은 Win32 창 생성, DirectX 11 device/swap chain 초기화, 셰이더/머티리얼 생성,
 * Mesh 생성, GameObject 생성 및 컴포넌트 부착을 한 번에 수행한다.
 *
 * 실행 흐름 요약:
 * 1. WinMain에서 GraphicsContext를 가져온다.
 * 2. 창과 DirectX 렌더링 리소스를 생성한다.
 * 3. 셰이더와 머티리얼, Mesh를 준비한다.
 * 4. Player와 Bullet GameObject를 만들고 컴포넌트를 붙인다.
 * 5. GameLoop::Run으로 Input -> Update -> Render를 반복한다.
 */

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <chrono>
#include <cstring>
#include <random>
#include <string>
#include <vector>

#include "EngineTypes.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "Resources/Mesh.h"
#include "MeshRenderer.h"
#include "PlayerControl.h"
#include "VelocityController.h"
#include "Win32Handler.h"
#include "Resources/Materials/ColorMaterial.h"
#include "D3D11ResourceHandler.h"
#include <directxmath.h>

#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")


KeyState localKeyState;
VideoConfig videoConfig;
namespace {
    // 플레이어를 표현할 삼각형 Mesh 정점 데이터를 만든다.
    // 현재 type별 형태 차이는 없지만, 이후 플레이어 종류별 정점/크기/방향을 나눌 수 있는 자리다.
    std::vector<Vertex> CreatePlayerMesh(int type)
    {
        if (type == 0) {
            return {
                { 0.0f,   0.18f, 0.5f },
                { 0.16f, -0.10f, 0.5f },
                { -0.16f, -0.10f, 0.5f }
            };
        }

        return {
            { 0.0f,   0.18f, 0.5f },
            { 0.16f, -0.10f, 0.5f },
            { -0.16f, -0.10f, 0.5f }
        };
    }

    float CreateRandomVelocityComponent(std::mt19937& rng)
    {
        // 탄환이 시작할 때 임의의 방향과 속도를 갖도록 한다.
        // 크기는 0.18~0.42 사이, 부호는 -1 또는 +1 중 하나를 사용한다.
        std::uniform_real_distribution<float> distribution(0.18f, 0.42f);
        std::uniform_int_distribution<int> signDistribution(0, 1);
        const float sign = signDistribution(rng) == 0 ? -1.0f : 1.0f;
        return distribution(rng) * sign;
    }

    float CreateRandomPositionComponent(std::mt19937& rng, float minValue, float maxValue)
    {
        // 지정된 범위 안에서 한 축의 랜덤 위치 값을 만든다.
        std::uniform_real_distribution<float> distribution(minValue, maxValue);
        return distribution(rng);
    }

    float CalculateDistanceSquared(const Vec3& first, const Vec3& second)
    {
        // sqrt를 쓰지 않는 제곱 거리 계산.
        // 최소 거리 비교에서는 실제 거리보다 제곱 거리 비교가 더 가볍다.
        const float xDistance = first.x - second.x;
        const float yDistance = first.y - second.y;
        const float zDistance = first.z - second.z;
        return xDistance * xDistance + yDistance * yDistance + zDistance * zDistance;
    }

    bool IsFarEnoughFromOccupiedPositions(const Vec3& position, const std::vector<Vec3>& occupiedPositions, float minDistance)
    {
        // 새 spawn 위치가 기존 오브젝트들과 충분히 떨어져 있는지 검사한다.
        const float minDistanceSquared = minDistance * minDistance;
        for (const Vec3& occupiedPosition : occupiedPositions) {
            if (CalculateDistanceSquared(position, occupiedPosition) < minDistanceSquared) {
                return false;
            }
        }

        return true;
    }

    Vec3 CreateSeparatedRandomPosition(std::mt19937& rng, const std::vector<Vec3>& occupiedPositions)
    {
        // 탄환이 플레이어나 다른 탄환과 겹쳐 시작하지 않도록 분리된 위치를 찾는다.
        // 먼저 랜덤 시도를 하고, 실패하면 격자 탐색으로 가능한 위치를 찾는다.
        constexpr float minX = -0.75f;
        constexpr float maxX = 0.75f;
        constexpr float minY = -0.55f;
        constexpr float maxY = 0.55f;
        constexpr float minSpawnDistance = 0.24f;

        // 충분히 많은 랜덤 후보를 먼저 시도한다.
        for (int attempt = 0; attempt < 256; ++attempt) {
            Vec3 position;
            position.x = CreateRandomPositionComponent(rng, minX, maxX);
            position.y = CreateRandomPositionComponent(rng, minY, maxY);
            position.z = 0.0f;

            if (IsFarEnoughFromOccupiedPositions(position, occupiedPositions, minSpawnDistance)) {
                return position;
            }
        }

        // 랜덤 배치가 실패하면 결정적인 fallback으로 격자 위치를 순회한다.
        for (float y = minY; y <= maxY; y += minSpawnDistance) {
            for (float x = minX; x <= maxX; x += minSpawnDistance) {
                Vec3 position = { x, y, 0.0f };
                if (IsFarEnoughFromOccupiedPositions(position, occupiedPositions, minSpawnDistance)) {
                    return position;
                }
            }
        }

        return { minX, minY, 0.0f };
    }

    std::vector<Vertex> CreateBulletMesh()
    {
        // 탄환을 표현할 작은 삼각형 Mesh 정점 데이터.
        return {
            { 0.0f,   0.055f, 0.5f },
            { 0.05f, -0.035f, 0.5f },
            { -0.05f, -0.035f, 0.5f }
        };
    }
}

std::string shaderSource = R"(
cbuffer MatrixBuffer : register(b0)
{
    row_major matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
}

cbuffer ColorBuffer : register(b1)
{
    float4 tintColor;
}

struct VS_INPUT { float3 pos : POSITION; };
struct PS_INPUT { float4 pos : SV_POSITION; };

PS_INPUT VS(VS_INPUT input) {
    PS_INPUT output;
    output.pos = mul(float4(input.pos, 1.0f), worldMatrix);
    return output;
}

float4 PS(PS_INPUT input) : SV_Target {
    return tintColor;
}
)";

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    // 그래픽스 리소스의 중심 싱글톤을 가져온다.
    GraphicsContext* ctx = GraphicsContext::getInstance();
    // 현재 Vertex는 position만 가지므로 input layout도 POSITION 하나만 선언한다.
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    // Win32 창을 만들고 화면에 표시한다.
    ctx->createWindow(hInstance, nCmdShow, L"test", videoConfig.Width, videoConfig.Height);
    // DirectX device, swap chain, render target view를 생성한다.
    ctx->createDeviceAndSwapChainAndRTV(videoConfig.Width, videoConfig.Height);

    // 현재 main.cpp에서는 직접 사용하지 않지만, 초기화 성공 확인이나 추후 리소스 생성에 쓸 수 있다.
    ID3D11Device* pd3dDevice = ctx->getDevice();

    //========================================================================//
    //========================================================================//
    //========================================================================//
    {
        // CPU 정점 데이터로 Mesh를 만들고, 렌더링 전에 GPU vertex buffer로 업로드한다.
        Mesh playerMesh(CreatePlayerMesh(0));
        Mesh bulletMesh(CreateBulletMesh());
        playerMesh.createVertexBuffer();
        bulletMesh.createVertexBuffer();
        // 문자열 HLSL 소스를 컴파일해 vertex shader, pixel shader, input layout을 만든다.
        ShaderSet starShaders = ctx->CompileAndCreate(shaderSource.c_str(), shaderSource.length(), false, ied, 1);

        // 단색 머티리얼 생성. 현재 Player와 Bullet이 같은 머티리얼을 공유한다.
        ColorMaterial* goldMat = new ColorMaterial(starShaders, { 1, 0.8f, 0, 1 });

        // GameLoop는 오브젝트 목록, 프레임 실행, 충돌 시스템을 관리한다.
        GameLoop loop;
        loop.collisionSystem.SetCollisionDistance(0.18f);
        loop.collisionSystem.SetBounds(-0.85f, 0.85f, -0.65f, 0.65f);
        // 탄환의 시작 위치와 속도를 랜덤으로 만들기 위한 난수 생성기.
        std::random_device randomDevice;
        std::mt19937 rng(randomDevice());
        //========================================================================//
        //========================================================================//
        //========================================================================//

        // Player 오브젝트 구성:
        // 입력으로 velocity를 정하는 PlayerControl, velocity를 position에 반영하는 VelocityController,
        // Mesh와 Material을 그리는 MeshRenderer를 조합한다.
        GameObject* player = new GameObject("Player");
        player->AddComponent(new PlayerControl(0));
        player->AddComponent(new VelocityController());
        player->AddComponent(new MeshRenderer({ &playerMesh }, goldMat));
        loop.AddGameObject(player);

        std::vector<Vec3> occupiedPositions;
        occupiedPositions.push_back(player->position);

        // Bullet 오브젝트 10개를 생성한다.
        // 각 탄환은 랜덤 위치와 랜덤 속도를 갖고, VelocityController로 움직이며, MeshRenderer로 그려진다.
        for (int i = 0; i < 10; ++i) {
            GameObject* bullet = new GameObject("Bullet" + std::to_string(i));
            bullet->position = CreateSeparatedRandomPosition(rng, occupiedPositions);
            occupiedPositions.push_back(bullet->position);
            bullet->velocity.x = CreateRandomVelocityComponent(rng);
            bullet->velocity.y = CreateRandomVelocityComponent(rng);
            bullet->velocity.z = 0.0f;
            bullet->AddComponent(new VelocityController());
            bullet->AddComponent(new MeshRenderer({ &bulletMesh }, goldMat));
            loop.AddGameObject(bullet);
        }

        // 메인 게임 루프 실행. 이 호출은 WM_QUIT을 받을 때까지 반환되지 않는다.
        loop.Run();
    }

    // 종료 시 DirectX 자원 정리.
    ctx->CleanUp();
    return 0;
}
