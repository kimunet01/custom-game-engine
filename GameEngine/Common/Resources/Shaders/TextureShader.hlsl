// 1. 행렬 상수 버퍼 (b0 슬롯)
cbuffer MatrixBuffer : register(b0)
{
    row_major matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
}

// 2. 텍스처 및 샘플러 선언
Texture2D diffuseTexture : register(t0);
SamplerState diffuseSampler : register(s0);

// 3. 지형 환경 연출 상수 버퍼 (b1 슬롯)
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

float4 PS(PS_INPUT input) : SV_Target
{
    // [1] dungeon2.png 이미지로부터 원래의 픽셀 색상을 추출한다.
    float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);

    // [2] 보스스테이지 쉐이더 연출
    if (g_isBossStage == 1)
    {
        // [어두운 분위기 조성] 기존 맵 색상과 디테일을 유지하되, 
        // 전체적인 밝기를 살짝 다운(0.6배) 시키고 푸르스름하고 무거운 암청색 톤을 섞는다.
        float3 darkBase = textureColor.rgb * float3(0.6f, 0.55f, 0.65f);
        
        // 어두운 붉은색 톤(보스방 전용 분위기 컬러)
        float3 bloodyTone = textureColor.rgb * float3(1.4f, 0.2f, 0.2f);
        textureColor.rgb = lerp(darkBase, bloodyTone, 0.4f);
       
        // 보스 Stage 진입 초기(g_time이 0초~1.5초 사이일 때)에만 적용
        float flashIntensity = saturate(2.0f - g_time); // 2초가 지나면 0이 되어 번쩍임이 멈춘다.
        
        if (flashIntensity > 0.0f)
        {
            // sin 함수를 이용해 두 번 번쩍이게 주파수를 조율하고(g_time * 6.0f), 
            // 뒤로 갈수록 스르륵 사라지도록 flashIntensity를 곱한다.
            float wave = sin(g_time * 6.0f);
            float flashWeight = abs(wave) * flashIntensity;
            
            // 번쩍일 때는 순간적으로 밝고 강렬한 광원 효과를 더한다.
            textureColor.rgb += float3(0.4f, 0.05f, 0.05f) * flashWeight;
        }
    }

    return textureColor;
}