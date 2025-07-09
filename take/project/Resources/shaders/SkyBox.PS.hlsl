#include "SkyBox.hlsli"

struct Material {
	float4 color; //カラー
	float4x4 uvTransform;
	int enableLighting; //Lightingを有効にするフラグ
	float shininess;
};

//マテリアル
ConstantBuffer<Material> gMaterial : register(b0);
//テクスチャ
TextureCube<float4> gTexture : register(t0);
//サンプラー
SamplerState gSampler : register(s0);

struct PixelShaderOutPut {
	float4 color : SV_TARGET0;
};

PixelShaderOutPut main(VertexShaderOutput input){
	PixelShaderOutPut output;
	float4 textureColor = gTexture.Sample(gSampler,input.texcoord);
	output.color = textureColor * gMaterial.color;
	return output;
}