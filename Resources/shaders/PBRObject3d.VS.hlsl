#include "PBRObject3d.hlsli"

struct TransformationMatrix
{
    float32_t4x4 WVP;
    float32_t4x4 World;
    float32_t4x4 WorldInverseTranspose;
};
ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

struct VertexShaderInput
{
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
    float32_t4 tangent : TANGENT0; // w成分はハンドネス（右手系/左手系）
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    
    // 位置の変換
    output.position = mul(input.position, gTransformationMatrix.WVP);
    output.worldPosition = mul(input.position, gTransformationMatrix.World).xyz;
    
    // テクスチャ座標
    output.texcoord = input.texcoord;
    
    // 法線の変換（法線行列を使用）
    output.normal = normalize(mul(input.normal, (float32_t3x3)gTransformationMatrix.WorldInverseTranspose));
    
    // タンジェントの変換
    output.tangent = normalize(mul(input.tangent.xyz, (float32_t3x3)gTransformationMatrix.World));
    
    // バイタンジェントの計算
    // glTFではタンジェントのw成分がハンドネス（-1 or 1）を表す
    float32_t handedness = input.tangent.w;
    output.bitangent = normalize(cross(output.normal, output.tangent) * handedness);
    
    return output;
}