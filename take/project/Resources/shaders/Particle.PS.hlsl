#include "Particle.hlsli"

struct Material {
	float4 color; //カラー
	float4x4 uvTransform;
	int enableLighting; //Lightingを有効にするフラグ
	float shininess;
};

//struct DirectionalLight {
//	float4 color; //ライトのカラー	
//	float3 direction; //ライトの向き
//	float intensity; //輝度
//};

//マテリアル
ConstantBuffer<Material> gMaterial : register(b0);
//テクスチャ
Texture2D<float4> gTexture : register(t0);
//サンプラー
SamplerState gSampler : register(s0);

struct PixelShaderOutPut {
	float4 color : SV_TARGET0;
};

PixelShaderOutPut main(VertexShaderOutput input) {
	PixelShaderOutPut output;

	float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
	float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
	output.color = gMaterial.color * textureColor * input.color;

	if (output.color.a == 0.0f) {
		discard;
	}
	
	return output;
}