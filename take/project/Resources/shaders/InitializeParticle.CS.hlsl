#include "GPUParticle.hlsli"

RWStructuredBuffer<ParticleForCS> gParticles : register(u0);
RWStructuredBuffer<int> gFreeListIndex : register(u1);
RWStructuredBuffer<uint> gFreeList : register(u2);

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
	uint particleIndex = DTid.x;
	

	if ( particleIndex < kMaxParticles ) {
		
		//particle初期化
		gParticles[particleIndex] = (ParticleForCS)0;
		gFreeList[particleIndex] = particleIndex;
	}
	
	//Indexが末尾を指すようにする
	if ( particleIndex == 0 ) {
		gFreeListIndex[0] = kMaxParticles - 1;
	}
}