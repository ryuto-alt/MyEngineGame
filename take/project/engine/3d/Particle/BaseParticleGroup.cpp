#include "BaseParticleGroup.h"
#include "PipelineStateObject.h"
#include "ModelManager.h"
#include "MatrixMath.h"
#include "Vector3Math.h"
#include "Camera.h"
#include "ImGuiManager.h"
#include "SrvManager.h"
#include "ModelCommon.h"
#include "ParticleCommon.h"
#include "TextureManager.h"
#include "engine/base/TakeCFrameWork.h"
#include "camera/CameraManager.h"
#include <numbers>

void BaseParticleGroup::Draw() {

	// perViewResource
	particleCommon_->GetDirectXCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(
		particleCommon_->GetGraphicPSO()->GetGraphicBindResourceIndex("gPerView"), perViewResource_->GetGPUVirtualAddress());
	// particleResource
	particleCommon_->GetSrvManager()->SetGraphicsRootDescriptorTable(
		particleCommon_->GetGraphicPSO()->GetGraphicBindResourceIndex("gParticle"), particleSrvIndex_);
}

Particle BaseParticleGroup::MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate) {

	ParticleAttributes attributes = particlePreset_.attribute;
	//スケールをランダムに設定
	std::uniform_real_distribution<float> distScale(attributes.scaleRange.min, attributes.scaleRange.max);
	//回転をランダムに設定
	std::uniform_real_distribution<float> distRotate(attributes.rotateRange.min, attributes.rotateRange.max);
	//位置をランダムに設定
	std::uniform_real_distribution<float> distPosition(attributes.positionRange.min, attributes.positionRange.max);
	//速度をランダムに設定
	std::uniform_real_distribution<float> distVelocity(attributes.velocityRange.min, attributes.velocityRange.max);
	//色をランダムに設定
	std::uniform_real_distribution<float> distColor(attributes.colorRange.min, attributes.colorRange.max);
	//寿命をランダムに設定
	std::uniform_real_distribution<float> distTime(attributes.lifetimeRange.min, attributes.lifetimeRange.max);

	Particle particle;
	particle.transforms_.scale = { attributes.scale.x,attributes.scale.y,attributes.scale.z};
	particle.transforms_.rotate = { 0.0f,0.0f,distRotate(randomEngine)};

	Vector3 randomTranslate = { distPosition(randomEngine),distPosition(randomEngine),distPosition(randomEngine) };
	particle.transforms_.translate = translate + randomTranslate;
	particle.velocity_ = { distVelocity(randomEngine),distVelocity(randomEngine),distVelocity(randomEngine) };
	if (attributes.editColor) {
		particle.color_ = {
			attributes.color.x,
			attributes.color.y,
			attributes.color.z,
			1.0f };
	} else {
		particle.color_ = { distColor(randomEngine),distColor(randomEngine),distColor(randomEngine),1.0f };
	}

	particle.lifeTime_ = distTime(randomEngine);
	particle.currentTime_ = 0.0f;

	if (attributes.isBillboard == true) {
		perViewData_->isBillboard = true;
	}

	return particle;
}

//=============================================================================
// パーティクルの発生
//=============================================================================
std::list<Particle> BaseParticleGroup::Emit(const Vector3& emitterPos, uint32_t particleCount) {
	//ランダムエンジン
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());

	std::list<Particle> particles;
	for (uint32_t index = 0; index < particleCount; ++index) {
		particles.push_back(MakeNewParticle(randomEngine, emitterPos));
	}
	return particles;
}

//=============================================================================
// パーティクルの配列の結合処理
//=============================================================================
void BaseParticleGroup::SpliceParticles(std::list<Particle> particles) {
	particles_.splice(particles_.end(), particles);
}

void BaseParticleGroup::SetPreset(const ParticlePreset& preset) {
	particlePreset_ = preset;
	
}

void BaseParticleGroup::SetEmitterPosition(const Vector3& position) {
	emitterPos_ = position;
}

