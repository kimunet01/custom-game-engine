// 1. 행렬 상수 버퍼 (b0 슬롯)
cbuffer MatrixBuffer : register(b0)
{
    row_major matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
}

// 2. 텍스처 및 샘플러 정의
Texture2D diffuseTexture : register(t0);
SamplerState diffuseSampler : register(s0);

// 3. 환경 및 색상 상수 버퍼 (b1 슬롯 공유)
// 주의: 픽셀 셰이더 b1 슬롯을 EnvironmentBuffer와 ColorBuffer가 공유해야 할 수도 있음.
// 팀원들이 환경 효과를 위해 b1을 사용하므로, 틴트 색상을 EnvironmentBuffer에 통합하거나 b2로 옮겨야 함.
// 여기서는 안전하게 b2 슬롯으로 TintColor를 옮깁니다. (Renderer 코드 수정 필요)
cbuffer ColorBuffer : register(b2)
{
    float4 tintColor;
}

cbuffer EnvironmentBuffer : register(b1)
{
    float g_time; 
    int g_isBossStage; 
    float2 g_padding; 
    float4 g_hitPosition;
};

// 정점 셰이더 입력 구조체
struct VS_INPUT
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};

// 픽셀 셰이더 입력 구조체
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

// 정점 셰이더 함수 (Vertex Shader)
PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = mul(float4(input.pos, 1.0f), worldMatrix);
    output.uv = input.uv;
    return output;
}

// 픽셀 셰이더 함수 (Pixel Shader)
float4 PS(PS_INPUT input) : SV_Target
{
    // [1] 텍스처 샘플링
    float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);

    // [2] 팀원들의 보스 스테이지 환경 효과 적용
    if (g_isBossStage == 1)
    {
        float3 darkBase = textureColor.rgb * float3(0.6f, 0.55f, 0.65f);
        float3 bloodyTone = textureColor.rgb * float3(1.4f, 0.2f, 0.2f);
        textureColor.rgb = lerp(darkBase, bloodyTone, 0.4f);
       
        float flashIntensity = saturate(2.0f - g_time); 
        if (flashIntensity > 0.0f)
        {
            float wave = sin(g_time * 6.0f);
            float flashWeight = abs(wave) * flashIntensity;
            textureColor.rgb += float3(0.4f, 0.05f, 0.05f) * flashWeight;
        }
    }

    // [3] 제가 추가한 캐릭터 틴트 효과 적용 (곱하기)
    return textureColor * tintColor;
}
