#pragma once
#include "3d/Particle/BaseParticleGroup.h"
#include "Primitive/PrimitiveType.h"


class PrimitiveParticle : public BaseParticleGroup {

public:
	PrimitiveParticle(PrimitiveType type);
	~PrimitiveParticle();

	void Initialize(ParticleCommon* particleCommon, const std::string& filePath) override;
	void Update() override;
	void UpdateImGui() override;
	void Draw() override;

	/// <summary>
	/// パーティクルの生成
	/// </summary>
	Particle MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate);

	/// <summary>
	/// パーティクルの発生
	/// </summary>
	std::list<Particle> Emit(const Vector3& emitterPos, uint32_t particleCount);

	void SpliceParticles(std::list<Particle> particles);

	void SetPreset(const ParticlePreset& preset) override;

	void SetPrimitiveHandle(uint32_t handle) { primitiveHandle_ = handle; }

private:
	uint32_t primitiveHandle_ = 0; // プリミティブのハンドル
private:

	void UpdateMovement(std::list<Particle>::iterator particleIterator);
};

