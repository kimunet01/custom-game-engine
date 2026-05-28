// b0: per-object transforms.
cbuffer MatrixBuffer : register(b0)
{
    row_major matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
}

// b1: stage-wide environment effects (boss stage tone, flash, time).
// Written by EnvironmentRenderer for the StageTerrain object.
cbuffer EnvironmentBuffer : register(b1)
{
    float g_time;
    int g_isBossStage;
    float2 g_padding;
    float4 g_hitPosition;
};

// b2: per-instance tint color (used for hit-flash on characters).
// Written by MeshRenderer for each character. Default (1,1,1,1) = pass-through.
cbuffer TintBuffer : register(b2)
{
    float4 tint;
};

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
    // [1] Sample base color from the diffuse texture.
    float4 textureColor = diffuseTexture.Sample(diffuseSampler, input.uv);

    // [2] Apply boss-stage environment effect (only when g_isBossStage == 1).
    // This makes the stage texture take on a darker, bloody tone with a periodic flash
    // early in the boss stage.
    if (g_isBossStage == 1)
    {
        // Dark base: subdued, slightly purple-shifted version of the texture.
        float3 darkBase = textureColor.rgb * float3(0.6f, 0.55f, 0.65f);

        // Bloody tone: amplify red, suppress green/blue.
        float3 bloodyTone = textureColor.rgb * float3(1.4f, 0.2f, 0.2f);
        textureColor.rgb = lerp(darkBase, bloodyTone, 0.4f);

        // Initial flash that fades out over ~2 seconds since boss-stage start.
        float flashIntensity = saturate(2.0f - g_time);
        if (flashIntensity > 0.0f)
        {
            float wave = sin(g_time * 6.0f);
            float flashWeight = abs(wave) * flashIntensity;
            textureColor.rgb += float3(0.4f, 0.05f, 0.05f) * flashWeight;
        }
    }

    // [3] Apply per-instance tint last. Default (1,1,1,1) leaves color unchanged.
    // HitReactionController sets this to red for a short moment after a character takes damage.
    return float4(textureColor.rgb * tint.rgb, textureColor.a * tint.a);
}
