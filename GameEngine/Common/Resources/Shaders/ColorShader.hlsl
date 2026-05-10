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

struct VS_INPUT
{
    float3 pos : POSITION;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
};

PS_INPUT VS(VS_INPUT input)
{
    PS_INPUT output;
    output.pos = mul(float4(input.pos, 1.0f), worldMatrix);
    return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
    return tintColor;
}
