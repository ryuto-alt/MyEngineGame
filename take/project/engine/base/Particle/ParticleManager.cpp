#include "ParticleManager.h"
#include "base/ImGuiManager.h"
#include "base/TakeCFrameWork.h"
#include <cassert>

//================================================================================================
// 初期化
//================================================================================================

void ParticleManager::Initialize() {
	emitterAllocater_ = std::make_unique<ParticleEmitterAllocater>();
}

//================================================================================================
// 更新処理
//================================================================================================


void ParticleManager::Update() {
	for (auto& [name, particleGroup] : particleGroups_) {
		particleGroup->Update();
	}
}

//================================================================================================
// ImGuiの更新処理
//================================================================================================

void ParticleManager::UpdateImGui() {

#ifdef _DEBUG
	ImGui::Begin("ParticleManager");
	ImGui::Text("ParticleGroup Count : %d", particleGroups_.size());
	ImGui::Separator();
	for (const auto& [name, particleGroup] : particleGroups_) {

		if (ImGui::TreeNode(name.c_str())) {
			particleGroup->UpdateImGui();
			ImGui::TreePop();
		}
	}

	ImGui::End();
#endif // _DEBUG
}

//================================================================================================
// 描画処理
//================================================================================================

void ParticleManager::Draw() {
	for (auto& [name, particleGroup] : particleGroups_) {
		particleGroup->Draw();
	}
}

//================================================================================================
// 終了処理
//================================================================================================

void ParticleManager::Finalize() {
	particleGroups_.clear();
}

void ParticleManager::UpdatePrimitiveType(const std::string& groupName, PrimitiveType type,const Vector3& param) {

	if (!particleGroups_.contains(groupName)) {
		assert(false && "ParticleGroup not found! Please check the name.");
		return;
	}

	uint32_t newHandle = 0;
	//指定されたグループのプリミティブタイプを更新
	switch (type) {
	case PRIMITIVE_RING:
		newHandle = TakeCFrameWork::GetPrimitiveDrawer()->GenerateRing(
			param.x,param.y,particleGroups_.at(groupName)->GetPreset().textureFilePath);
		break;
	case PRIMITIVE_PLANE:
		newHandle = TakeCFrameWork::GetPrimitiveDrawer()->GeneratePlane(
			param.x, param.y, particleGroups_.at(groupName)->GetPreset().textureFilePath);
		break;
	case PRIMITIVE_SPHERE:
		newHandle = TakeCFrameWork::GetPrimitiveDrawer()->GenerateSphere(
			param.x, particleGroups_.at(groupName)->GetPreset().textureFilePath);
		break;
	default:
		break;
	}
	particleGroups_.at(groupName)->SetPrimitiveHandle(newHandle);
}

//================================================================================================
// パーティクルグループの生成
//================================================================================================

void ParticleManager::CreateParticleGroup(ParticleCommon* particleCommon, const std::string& name,
		const std::string& filePath,PrimitiveType primitiveType) {

	if (particleGroups_.contains(name)) {
		//既に同名のparticleGroupが存在する場合は生成しない
		return;
	}

	//particleGroupの生成
	std::unique_ptr<PrimitiveParticle> particleGroup = std::make_unique<PrimitiveParticle>(primitiveType);
	particleGroup->Initialize(particleCommon, filePath);
	particleGroups_.insert(std::make_pair(name, std::move(particleGroup)));
}

void ParticleManager::CreateParticleGroup(ParticleCommon* particleCommon, const std::string& presetJson) {
	ParticlePreset preset = TakeCFrameWork::GetJsonLoader()->LoadParticlePreset(presetJson);
	if(particleGroups_.contains(preset.presetName)) {
		//既に同名のparticleGroupが存在する場合は生成しない
		return;
	}

	//particleGroupの生成
	std::unique_ptr<PrimitiveParticle> particleGroup = std::make_unique<PrimitiveParticle>(preset.primitiveType);
	particleGroup->Initialize(particleCommon, preset.textureFilePath);
	particleGroup->SetPreset(preset);
	particleGroups_.insert(std::make_pair(preset.presetName, std::move(particleGroup)));
}


//================================================================================================
// パーティクルの発生
//================================================================================================

void ParticleManager::Emit(const std::string& name, const Vector3& emitPosition, uint32_t count) {

	//存在しない場合は処理しない
	if (!particleGroups_.contains(name)) {
		return;
	}

	//particleGroupsに発生させたパーティクルを登録させる
	particleGroups_.at(name)->SpliceParticles(particleGroups_.at(name)->Emit(emitPosition, count));
}

uint32_t ParticleManager::EmitterAllocate() {
	return emitterAllocater_->Allocate();
}

BaseParticleGroup* ParticleManager::GetParticleGroup(const std::string& name) {

	if (particleGroups_.contains(name)) {
		return particleGroups_.at(name).get();
	}

	assert(false && "ParticleGroup not found! Please check the name.");
	return nullptr;
}

void ParticleManager::SetPreset(const std::string& name, const ParticlePreset& preset) {

	//存在しない場合は処理しない
	if (!particleGroups_.contains(name)) {
		return;
	}
	//particleGroupsに発生させたパーティクルを登録させる
	particleGroups_.at(name)->SetPreset(preset);
}
