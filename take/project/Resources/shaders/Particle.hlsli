struct VertexShaderOutput {
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
	float3 normal : NORMAL0;
	float4 color : COLOR0;
};

struct ParticleForGPU {
	float3 translate;
	float3 rotate;
	float3 scale;
	float3 velocity;
	float4 color;
	float lifetime;
	float currentTime;
};