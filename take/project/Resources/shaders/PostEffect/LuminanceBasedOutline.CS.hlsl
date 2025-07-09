#include "PostEffect/Fullscreen.hlsli"
#include "PostEffect/Outline.hlsli"

Texture2D<float4> gInputTexture : register(t0);
RWTexture2D<float4> gOutputTexture : register(u0);
SamplerState gSampler : register(s0);

float Luminance(float3 v) {
	return dot(v, float3(0.2125f, 0.7154f, 0.0721f));
}

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID ) {
	
	float width, height;
	gInputTexture.GetDimensions(width, height);
	float2 uv = DTid.xy / float2(width, height);
	float2 uvStepSize = float2(rcp(width), rcp(height));
	float3 resultColor = float3(0.0f, 0.0f, 0.0f);
	
	float2 difference = { 0.0f, 0.0f };
	
	//畳み込み
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 3; y++) { // 3x3 kernel
			
			//現在のtextureのUV座標を取得
			float2 offset = kIndex3x3[x][y] * uvStepSize;
			float2 sampleUV = uv + offset;
			
			//色に1/9を掛ける
			float3 fetchColor = gInputTexture.Sample(gSampler, sampleUV).rgb;
			float luminance = Luminance(fetchColor);
			difference.x += luminance * kPrewittHorizontalKernel[x][y];
			difference.y += luminance * kPrewittVerticalKernel[x][y];
		}
	}
	
	float weight = length(difference);
	weight = saturate(weight * 6.0f);
	
	resultColor = (1.0f - weight) * gInputTexture.Sample(gSampler, uv).rgb;
	gOutputTexture[DTid.xy] = float4(resultColor, 1.0f);
}