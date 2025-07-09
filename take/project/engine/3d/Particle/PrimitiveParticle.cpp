#include "PrimitiveParticle.h"
#include "base/DirectXCommon.h"
#include "base/ModelManager.h"
#include "base/SrvManager.h"
#include "base/TextureManager.h"
#include "base/TakeCFrameWork.h"
#include "math/MatrixMath.h"
#include "math/Vector3Math.h"
#include "math/Easing.h"
#include "camera/CameraManager.h"
#include "3d/Particle/ParticleCommon.h"
#include <numbers>

PrimitiveParticle::PrimitiveParticle(PrimitiveType type) {
	particlePreset_.primitiveType = type;
}

PrimitiveParticle::~PrimitiveParticle() {}

void PrimitiveParticle::Initialize(ParticleCommon* particleCommon, const std::string& filePath) {

	particleCommon_ = particleCommon;

	//ParticleResource生成
	particleResource_ =
		DirectXCommon::CreateBufferResource(particleCommon_->GetDirectXCommon()->GetDevice(), sizeof(ParticleForGPU) * kNumMaxInstance_);
	particleResource_->SetName(L"PrimitiveParticle::particleResource_");
	//perViewResource生成
	perViewResource_ = DirectXCommon::CreateBufferResource(particleCommon_->GetDirectXCommon()->GetDevice(), sizeof(PerView));
	perViewResource_->Map(0, nullptr, reinterpret_cast<void**>(&perViewData_));
	perViewResource_->SetName(L"PrimitiveParticle::perViewResource_");
	//SRVの生成
	particleSrvIndex_ = particleCommon_->GetSrvManager()->Allocate();
	particleCommon_->GetSrvManager()->CreateSRVforStructuredBuffer(
		kNumMaxInstance_,
		sizeof(ParticleForGPU),
		particleResource_.Get(),
		particleSrvIndex_
	);
	//perviewInit
	perViewData_->viewProjection = MatrixMath::MakeIdentity4x4();
	perViewData_->billboardMatrix = MatrixMath::MakeIdentity4x4();

	//Mapping
	particleResource_->Map(0, nullptr, reinterpret_cast<void**>(&particleData_));

	if (particlePreset_.primitiveType == PRIMITIVE_RING) {
		//プリミティブの初期化
		primitiveHandle_ = TakeCFrameWork::GetPrimitiveDrawer()->GenerateRing(1.0f, 0.5f, filePath);
	} else if (particlePreset_.primitiveType == PRIMITIVE_PLANE) {
		primitiveHandle_ = TakeCFrameWork::GetPrimitiveDrawer()->GeneratePlane(1.0f, 1.0f, filePath);
	} else if (particlePreset_.primitiveType == PRIMITIVE_SPHERE) {
		primitiveHandle_ = TakeCFrameWork::GetPrimitiveDrawer()->GenerateSphere(1.0f, filePath);
	} else {
		assert(0 && "未対応の PrimitiveType が指定されました");
	}

	particlePreset_.textureFilePath = filePath;
}

void PrimitiveParticle::Update() {

	// ランダムエンジンの初期化  
	std::random_device seedGenerator;
	std::mt19937 randomEngine(seedGenerator());

	numInstance_ = 0;
	for (std::list<Particle>::iterator particleIterator = particles_.begin();
		particleIterator != particles_.end(); ) {

		// MEMO: Particleの総出現数だけ繰り返す  
		if (numInstance_ < kNumMaxInstance_) {

			// 寿命が来たら削除  
			if ((*particleIterator).lifeTime_ <= (*particleIterator).currentTime_) {
				particleIterator = particles_.erase(particleIterator);
				continue;
			}

			// particle1つの位置更新  
			UpdateMovement(particleIterator);
			//alphaの計算
			float alpha = 1.0f - ((*particleIterator).currentTime_ / (*particleIterator).lifeTime_);

			particleData_[numInstance_].color = {
				(*particleIterator).color_.x,
				(*particleIterator).color_.y,
				(*particleIterator).color_.z,
				alpha
			};
			particleData_[numInstance_].scale = (*particleIterator).transforms_.scale;
			particleData_[numInstance_].rotate = (*particleIterator).transforms_.rotate;
			particleData_[numInstance_].translate = (*particleIterator).transforms_.translate;
			particleData_[numInstance_].velocity = (*particleIterator).velocity_;
			particleData_[numInstance_].lifeTime = (*particleIterator).lifeTime_;
			particleData_[numInstance_].currentTime = (*particleIterator).currentTime_;

			// データをGPUに転送  
			perViewData_->viewProjection = CameraManager::GetInstance()->GetActiveCamera()->GetViewProjectionMatrix();
			perViewData_->billboardMatrix = CameraManager::GetInstance()->GetActiveCamera()->GetRotationMatrix();

			++numInstance_; // 次のインスタンスに進める  
		}
		++particleIterator; // 次のイテレータに進める  
	}
}

void PrimitiveParticle::UpdateImGui() {
#ifdef _DEBUG
	ParticleAttributes& attributes = particlePreset_.attribute;

	ImGui::Begin("Particle");
	ImGui::Text("Instance Count : %d", numInstance_);
	ImGui::DragFloat3("RingScale", &attributes.scale.x, 0.01f);
	ImGui::DragFloat2("RotateRange", &attributes.rotateRange.min, 0.01f);
	ImGui::DragFloat2("PositionRange", &attributes.positionRange.min, 0.01f);
	ImGui::DragFloat2("VelocityRange", &attributes.velocityRange.min, 0.01f);
	ImGui::DragFloat2("ColorRange", &attributes.colorRange.min, 0.01f);
	ImGui::Checkbox("isBillboard", &attributes.isBillboard);
	ImGui::End();
#endif // _DEBUG
}

void PrimitiveParticle::Draw() {


	BaseParticleGroup::Draw();
	//プリミティブの描画
	TakeCFrameWork::GetPrimitiveDrawer()->DrawParticle(
		particleCommon_->GetGraphicPSO(), numInstance_, particlePreset_.primitiveType, primitiveHandle_);
}

Particle PrimitiveParticle::MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate) {

	return BaseParticleGroup::MakeNewParticle(randomEngine, translate);
}

std::list<Particle> PrimitiveParticle::Emit(const Vector3& emitterPos, uint32_t particleCount) {

	return BaseParticleGroup::Emit(emitterPos, particleCount);
}

void PrimitiveParticle::SpliceParticles(std::list<Particle> particles) {

	BaseParticleGroup::SpliceParticles(particles);
}

void PrimitiveParticle::SetPreset(const ParticlePreset& preset) {
	particlePreset_ = preset;
	//テクスチャファイルパスの設定
	if (particlePreset_.primitiveType == PRIMITIVE_RING) {
		auto& primitiveMaterial = TakeCFrameWork::GetPrimitiveDrawer()->GetRingData(primitiveHandle_)->material_;
		primitiveMaterial->SetTextureFilePath(preset.textureFilePath);
	}else if (particlePreset_.primitiveType == PRIMITIVE_PLANE) {
		auto& primitiveMaterial = TakeCFrameWork::GetPrimitiveDrawer()->GetPlaneData(primitiveHandle_)->material_;
		primitiveMaterial->SetTextureFilePath(preset.textureFilePath);
	} else if (particlePreset_.primitiveType == PRIMITIVE_SPHERE) {
		auto& primitiveMaterial = TakeCFrameWork::GetPrimitiveDrawer()->GetSphereData(primitiveHandle_)->material_;
		primitiveMaterial->SetTextureFilePath(preset.textureFilePath);
	} else {
		assert(0 && "未対応の PrimitiveType が指定されました");
	}
}

void PrimitiveParticle::UpdateMovement(std::list<Particle>::iterator particleIterator) {

	//particle1つの位置更新
	ParticleAttributes& attributes = particlePreset_.attribute;

	if (attributes.isTraslate_) {
		if (attributes.enableFollowEmitter_) {
			//エミッターに追従する場合
			(*particleIterator).transforms_.translate = emitterPos_;
		} else {
			(*particleIterator).transforms_.translate += (*particleIterator).velocity_ * kDeltaTime_;

		}
	}

	if (attributes.scaleSetting_ == static_cast<uint32_t>(ScaleSetting::ScaleUp)) {
		//スケールの更新(拡大)
		(*particleIterator).transforms_.scale.x = Easing::Lerp(
			attributes.scaleRange.min,
			attributes.scaleRange.max,
			(*particleIterator).currentTime_ / (*particleIterator).lifeTime_);

		(*particleIterator).transforms_.scale.y = Easing::Lerp(
			attributes.scaleRange.min,
			attributes.scaleRange.max,
			(*particleIterator).currentTime_ / (*particleIterator).lifeTime_);
		(*particleIterator).transforms_.scale.z = Easing::Lerp(
			attributes.scaleRange.min,
			attributes.scaleRange.max,
			(*particleIterator).currentTime_ / (*particleIterator).lifeTime_);

	} else if (attributes.scaleSetting_ == static_cast<uint32_t>(ScaleSetting::ScaleDown)) {
		//スケールの更新(縮小)
		(*particleIterator).transforms_.scale.x = Easing::Lerp(
			attributes.scaleRange.max,
			attributes.scaleRange.min,
			(*particleIterator).currentTime_ / (*particleIterator).lifeTime_);
		(*particleIterator).transforms_.scale.y = Easing::Lerp(
			attributes.scaleRange.max,
			attributes.scaleRange.min,
			(*particleIterator).currentTime_ / (*particleIterator).lifeTime_);
		(*particleIterator).transforms_.scale.z = Easing::Lerp(
			attributes.scaleRange.max,
			attributes.scaleRange.min,
			(*particleIterator).currentTime_ / (*particleIterator).lifeTime_);
	}

	(*particleIterator).currentTime_ += kDeltaTime_; //経過時間の更新

}
