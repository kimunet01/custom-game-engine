cbuffer MatrixBuffer : register(b0)
{
    row_major matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
}

// per-instance tint color. MeshRenderer가 매 프레임 b1 슬롯에 갱신한다.
// HitReactionController가 피격 시 (1,0,0,1)로 잠깐 바꿨다가 (1,1,1,1)로 복원한다.
cbuffer TintBuffer : register(b1)
{
    float4 tint;
}

Texture2D diffuseTexture : register(t0);
SamplerState diffuseSampler : register(s0);

struct VS_INPUT
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = mul(float4(input.pos, 1.0f), worldMatrix);
    output.uv = input.uv;
    return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
    float4 sampled = diffuseTexture.Sample(diffuseSampler, input.uv);
    // RGB는 tint와 곱하여 색상 변조, 알파는 텍스처 알파 × tint 알파.
    return float4(sampled.rgb * tint.rgb, sampled.a * tint.a);
}
