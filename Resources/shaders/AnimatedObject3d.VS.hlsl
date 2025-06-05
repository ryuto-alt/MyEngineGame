#include "Object3d.hlsli"

struct VertexShaderInput {
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 weight : WEIGHT0;
    uint4 boneIndices : BONEINDICES0;
};

cbuffer TransformationMatrix : register(b1) {
    float4x4 WVP;
    float4x4 World;
};

cbuffer BoneMatrix : register(b4) {
    float4x4 bones[128];
};

VertexShaderOutput main(VertexShaderInput input) {
    VertexShaderOutput output;
    
    // スキニング行列の計算（頂点ブレンディング）
    // デバッグ: インデックスが範囲外になる問題を回避
    uint idx0 = min(input.boneIndices.x, 127);
    uint idx1 = min(input.boneIndices.y, 127);
    uint idx2 = min(input.boneIndices.z, 127);
    uint idx3 = min(input.boneIndices.w, 127);
    
    float4x4 skinMatrix = 
        bones[idx0] * input.weight.x +
        bones[idx1] * input.weight.y +
        bones[idx2] * input.weight.z +
        bones[idx3] * input.weight.w;
    
    // スキニングされた頂点位置
    float4 skinnedPos = mul(skinMatrix, input.position);
    output.position = mul(WVP, skinnedPos);
    output.texcoord = input.texcoord;
    
    // スキニングされた法線
    float3 skinnedNormal = mul((float3x3)skinMatrix, input.normal);
    output.normal = normalize(mul((float3x3)World, skinnedNormal));
    
    return output;
}