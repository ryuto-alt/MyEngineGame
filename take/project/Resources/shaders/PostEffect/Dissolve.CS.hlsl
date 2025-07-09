#include "PostEffect/FullScreen.hlsli"

struct DissolveInfo {
	float threshold; //閾値
	bool isDissolve;
};

Texture2D<float4> gInputTexture : register(t0);
Texture2D<float4> gMaskTexture  : register(t1);
RWTexture2D<float4> gOutputTexture : register(u0);
SamplerState gSampler : register(s0);
ConstantBuffer<DissolveInfo> gDissolveInfo : register(b0);

[numthreads(2, 2, 1)]
void main( uint3 DTid : SV_DispatchThreadID ) {
	
	float width, height;
	gInputTexture.GetDimensions(width, height);
	float2 uv = DTid.xy / float2(width, height);
	float3 resultColor = float3(0.0f, 0.0f, 0.0f);
	
	if (gDissolveInfo.isDissolve == false) {
		gOutputTexture[DTid.xy] = gInputTexture[DTid.xy];
		return;
	}
	float mask = gMaskTexture.Sample(gSampler, uv).r;
	if ( mask <= 0.0f ) {
		gOutputTexture[DTid.xy] = gInputTexture[DTid.xy];
		return;
	}
	if (mask <= gDissolveInfo.threshold) {
		gOutputTexture[DTid.xy] = float4(0.0f, 1.0f, 0.0f, 1.0f);
		return;
	}
	
	resultColor = gInputTexture.Sample(gSampler, uv).rgb;
	
	//簡単なEdge検出
	float edge = 1.0f - smoothstep(gDissolveInfo.threshold, gDissolveInfo.threshold + 0.03f, mask);
	resultColor += edge * float3(1.0f, 0.0f, 0.0f);
	
	gOutputTexture[DTid.xy] = float4(resultColor, 1.0f);
}