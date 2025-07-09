#include "PostEffect/Fullscreen.hlsli"

struct VignetteInfo {
	float scale;
	float power;
	bool enable;
};

Texture2D<float4> gInputTexture : register(t0);
RWTexture2D<float4> gOutputTexture : register(u0);
ConstantBuffer<VignetteInfo> gVignetteInfo : register(b0);

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID ){
	
	if (gVignetteInfo.enable == false) {
		gOutputTexture[DTid.xy] = gInputTexture[DTid.xy];
		return;
	}
	
	float4 color = gInputTexture[DTid.xy];
	float width, height;
	gOutputTexture.GetDimensions(width, height);
	float2 uv = DTid.xy / float2(width, height);
	float2 offset = uv - 0.5f;
	float distance = length(offset);
	float vignette = pow(1.0f - distance * gVignetteInfo.scale, gVignetteInfo.power);
	vignette = saturate(vignette);
	
	float4 resultColor = color * vignette;
	gOutputTexture[DTid.xy] = resultColor;
}