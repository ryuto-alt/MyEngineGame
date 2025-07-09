#include "PostEffect/FullScreen.hlsli"

struct RadialBlurInfo {
	float2 center; 
	float kBlurWidth;
	bool enable;
};

Texture2D<float4> gInputTexture : register(t0);
RWTexture2D<float4> gOutputTexture : register(u0);
SamplerState gSampler : register(s0);

ConstantBuffer<RadialBlurInfo> gBlurInfo : register(b0);

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID ) {
	float width, height;
	gInputTexture.GetDimensions(width, height);
	float2 uv = DTid.xy / float2(width, height);
	float3 resultColor = float3(0.0f, 0.0f, 0.0f);
	
	
	//サンプリング数。多いほど滑らかになるが重くなる
	const int kNumSamples = 16;
	
	if (gBlurInfo.enable == false) {
		gOutputTexture[DTid.xy] = gInputTexture[DTid.xy];
		return;
	};
	//中心から現在のUVに対しての方向を計算
	float2 direction = uv - gBlurInfo.center;
	
	for (int sampleIndex = 0; sampleIndex < kNumSamples; ++sampleIndex) {
		//サンプリングのUV座標を計算
		float2 sampleUV = uv + direction * gBlurInfo.kBlurWidth * float(sampleIndex);
		//色を加算
		resultColor += gInputTexture.Sample(gSampler, sampleUV).rgb;
	}
	
	//平均化
	resultColor *= rcp(float(kNumSamples));
	
	gOutputTexture[DTid.xy] = float4(resultColor, 1.0f);
}