#include "Object3d.hlsli"

struct VertexShaderInput {
	float4 position : POSITION0;
	float2 texcoord : TEXCOORD0;
	float3 normal : NORMAL0;
};

//スキニング結果を受け取るための構造体
struct Skinned {
	float4 position;
	float2 texcoord;
	float3 normal;
};

struct TransformationMatrix {
	float4x4 WVP;
	float4x4 World;
	float4x4 WorldInverseTranspose;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);

VertexShaderOutput main(VertexShaderInput input) {
	VertexShaderOutput output;
	
	//Skinning結果を使って位置を変換
	output.position = mul(input.position, gTransformationMatrix.WVP);
	output.worldPosition = mul(input.position, gTransformationMatrix.World).xyz;
	output.texcoord = input.texcoord;
	output.normal = normalize(mul(input.normal, (float3x3) gTransformationMatrix.WorldInverseTranspose));
	return output;
}