#include"object3d.hlsli"

cbuffer TransformationMatrix : register(b0)
{
    float4x4 WVP;
    float4x4 World;
};


struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, WVP);
    output.texcoord = input.texcoord;
    
    // 法線の計算を精密に行い、正規化を確実に行う
    float3 worldNormal = mul(input.normal, (float3x3) World);
    output.normal = normalize(worldNormal);
    
    return output;
}