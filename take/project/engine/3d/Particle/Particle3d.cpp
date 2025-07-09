#include "Particle3d.h"
#include "base/DirectXCommon.h"
#include "base/ModelManager.h"
#include "base/SrvManager.h"
#include "base/TextureManager.h"
#include "base/TakeCFrameWork.h"
#include "math/MatrixMath.h"
#include "math/Vector3Math.h"
#include "math/Easing.h"
#include "3d/Model.h"
#include "camera/CameraManager.h"
#include "ParticleCommon.h"
#include "Input.h"
#include <numbers>


Particle3d::~Particle3d() {
	model_ = nullptr;
	particleResource_.Reset();
	perViewResource_.Reset();
}

//=============================================================================
// 初期化
//=============================================================================

void Particle3d::Initialize(ParticleCommon* particleCommon, const std::string& filePath) {

	particleCommon_ = particleCommon;

	//instancing用のResource生成
	particleResource_ =
		DirectXCommon::CreateBufferResource(particleCommon_->GetDirectXCommon()->GetDevice(), sizeof(ParticleForGPU) * kNumMaxInstance_);
	particleResource_->SetName(L"Particle3d::particleResource_");

	//perViewResource生成
	perViewResource_ = DirectXCommon::CreateBufferResource(particleCommon_->GetDirectXCommon()->GetDevice(), sizeof(PerView));
	perViewResource_->Map(0, nullptr, reinterpret_cast<void**>(&perViewData_));
	perViewResource_->SetName(L"Particle3d::perViewResource_");

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
	perViewData_->isBillboard = true;

	//Mapping
	particleResource_->Map(0, nullptr, reinterpret_cast<void**>(&particleData_));

	ParticleAttributes& attributes = particlePreset_.attribute;

	//属性初期化
	attributes.scale = { 1.0f,1.0f,1.0f };
	attributes.positionRange = { 2.0f,2.0f };
	attributes.scaleRange = { 1.0f,1.0f };
	attributes.rotateRange = { -std::numbers::pi_v<float>, std::numbers::pi_v<float> };
	attributes.velocityRange = { 0.0f,1.0f };
	attributes.colorRange = { 0.0f,1.0f };
	attributes.lifetimeRange = { 5.0f,5.0f };
	attributes.editColor = true;
	attributes.color = { 0.9f,0.3f,0.9f };
	particlePreset_.textureFilePath = filePath;
	//モデルの読み込み
	SetModel(filePath);
	//TakeCFrameWork::GetPrimitiveDrawer()->CreateRingVertexData();

}

//=============================================================================
// 更新処理
//=============================================================================

void Particle3d::Update() {
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

//=============================================================================
// ImGuiの更新
//=============================================================================

void Particle3d::UpdateImGui() {
#ifdef _DEBUG

	ParticleAttributes& attributes = particlePreset_.attribute;

	ImGui::Text("Particle3d");
	ImGui::Text("ParticleCount:%d", numInstance_);
	particleCommon_->GetGraphicPSO()->UpdateImGui();
	ImGui::Separator();
	ImGui::DragFloat3("Scale", &attributes.scale.x, 0.01f);
	ImGui::DragFloat2("PositionRange", &attributes.positionRange.min, 0.01f);
	ImGui::DragFloat2("VelocityRange", &attributes.velocityRange.min, 0.01f);
	ImGui::DragFloat2("ColorRange", &attributes.colorRange.min, 0.01f);
	ImGui::DragFloat2("LifetimeRange", &attributes.lifetimeRange.min, 0.01f);
	ImGui::Checkbox("EditColor", &attributes.editColor);
	if (attributes.editColor) {
		ImGui::ColorEdit3("Color", &attributes.color.x);
	}

#endif // _DEBUG
}

//=============================================================================
// 描画処理
//=============================================================================

void Particle3d::Draw() {

	BaseParticleGroup::Draw();

	model_->DrawForParticle(numInstance_);
}



//=============================================================================
// パーティクルの生成
//=============================================================================

Particle Particle3d::MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate) {

	return BaseParticleGroup::MakeNewParticle(randomEngine, translate);
}

//=============================================================================
// パーティクルの発生
//=============================================================================

std::list<Particle> Particle3d::Emit(const Vector3& emitterPos, uint32_t particleCount) {

	return BaseParticleGroup::Emit(emitterPos, particleCount);
}

//=============================================================================
// パーティクルの配列の結合処理
//=============================================================================

void Particle3d::SpliceParticles(std::list<Particle> particles) {
	BaseParticleGroup::SpliceParticles(particles);
}

//=============================================================================
// モデルの設定
//=============================================================================

void Particle3d::SetModel(const std::string& filePath) {
	model_ = ModelManager::GetInstance()->FindModel(filePath);
}

void Particle3d::UpdateMovement(std::list<Particle>::iterator particleIterator) {
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

	if (attributes.scaleSetting_) {
		//スケールの更新
		(*particleIterator).transforms_.scale.x = Easing::Lerp(
			attributes.scaleRange.min,
			attributes.scaleRange.max,
			(*particleIterator).currentTime_ / (*particleIterator).lifeTime_);

		(*particleIterator).transforms_.scale.y = (*particleIterator).transforms_.scale.x;
		(*particleIterator).transforms_.scale.z = (*particleIterator).transforms_.scale.x;
	}

	(*particleIterator).currentTime_ += kDeltaTime_; //経過時間の更新
}
