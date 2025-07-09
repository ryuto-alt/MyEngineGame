#include "GPUParticle.hlsli"
#include "RandomGenerator.hlsli"

RWStructuredBuffer<ParticleForCS> gParticles : register(u0);
RWStructuredBuffer<int> gFreeListIndex : register(u1);
RWStructuredBuffer<uint> gFreeList : register(u2);
ConstantBuffer<PerFrame> gPerFrame : register(b0);

[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID ){
	uint particleIndex = DTid.x;
	
	//particleIndexが最大数を超えた場合は何もしない
	//if (particleIndex > kMaxParticles) {
	//	return;
	//}

	gParticles[particleIndex].translate += gParticles[particleIndex].velocity;
	gParticles[particleIndex].scale += gPerFrame.deltaTime;
	gParticles[particleIndex].currentTime += gPerFrame.deltaTime;
	
	float alpha = 1.0f - (gParticles[particleIndex].currentTime / gParticles[particleIndex].lifetime);
	gParticles[particleIndex].color.a = saturate(alpha);
	
	//alphaが0なのでFreeとする
	if ( gParticles[particleIndex].color.a == 0 ) {
		//VSの出力で棄却されるようにする
		gParticles[particleIndex].scale = float3(0.0f, 0.0f, 0.0f);
		
		
		int freeListIndex;
		InterlockedAdd(gFreeListIndex[0], 1, freeListIndex);
		
		if ( (freeListIndex + 1) < kMaxParticles ) {
			gFreeList[freeListIndex + 1] = particleIndex;
		}
		else {
			InterlockedAdd(gFreeListIndex[0], -1, freeListIndex);
		}
	}
}