#include "GPUParticle.hlsli"
#include "ParticleEmitter.hlsli"
#include "RandomGenerator.hlsli"

static const uint kMaxParticleEmitters = 2;

StructuredBuffer<EmitterSphere> gEmitterSphere : register(t0);
RWStructuredBuffer<ParticleForCS> gParticles : register(u0);
RWStructuredBuffer<int> gFreeListIndex : register(u1);
RWStructuredBuffer<uint> gFreeList : register(u2);
ConstantBuffer<PerFrame> gPerFrame : register(b0);

//MEMO:複数のemitterを扱う場合は、適宜スレッド数を増やす
[numthreads(2, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
	
	//乱数ジェネレーターを初期化
	RamdomGenerator randomGenerator;
	randomGenerator.seed = (DTid + gPerFrame.time) * gPerFrame.time;
	
	//EmitterのIndexを取得
	uint emitterIndex = DTid.x;

	//Emitterが有効かどうかをチェック
	if ( gEmitterSphere[emitterIndex].isEmit != 0 ) {
		
		//EmitterのParticle数分ループ
		for ( uint countIndex = 0; countIndex < gEmitterSphere[emitterIndex].particleCount; ++countIndex ) {
			
			int freeListIndex;
			//gFreeListのIndexを1つ前に設定して、現在のIndexを取得
			InterlockedAdd(gFreeListIndex[0], -1, freeListIndex);
			
			//最大数よりもparticleが少ない場合射出可能
			if(0 <= freeListIndex && freeListIndex < kMaxParticles ) {
				//射出するParticleのIndexを取得
				int particleIndex = gFreeList[freeListIndex];
				//particle初期化
				gParticles[particleIndex].translate = randomGenerator.Generate3d();
				gParticles[particleIndex].scale = float3(1.0f, 1.0f, 1.0f);
				gParticles[particleIndex].color.rgb = randomGenerator.Generate3d();
				gParticles[particleIndex].color.a = 1.0f;
				gParticles[particleIndex].velocity = randomGenerator.Generate3d() * 3.0f - 1.0f;
				gParticles[particleIndex].lifetime = randomGenerator.Generate1d() * 2.0f + 1.0f;
				gParticles[particleIndex].currentTime = 0.0f;
				
			} else {
				//射出できない場合は、gFreeListIndexを減らした分戻す
				InterlockedAdd(gFreeListIndex[0], 1);
				break;
			}
		}
	}
}