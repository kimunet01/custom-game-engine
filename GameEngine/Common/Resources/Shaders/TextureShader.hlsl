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
    return diffuseTexture.Sample(diffuseSampler, input.uv) * tintColor;
}
