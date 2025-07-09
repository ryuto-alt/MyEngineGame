#pragma once
#include "ParticleEmitter.h"
#include "ParticleCommon.h"
#include "DirectXCommon.h"
#include "SrvManager.h"

class ParticleEmitterAllocater {
public:
	ParticleEmitterAllocater() = default;
	~ParticleEmitterAllocater() = default;

	uint32_t Allocate();

	void Clear();

public:

	static const uint32_t kMaxEmitterCount_ = 512; //最大emitter数

private:

	uint32_t emitterCount_ = 0;
};

