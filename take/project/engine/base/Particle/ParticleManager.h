#pragma once
#include "ResourceDataStructure.h"
#include "3d/Particle/Particle3d.h"
#include "3d/Particle/PrimitiveParticle.h"
#include "3d/Particle/ParticleCommon.h"
#include "3d/Particle/ParticleEmitterAllocater.h"
#include <list>
#include <wrl.h>
#include <unordered_map>
#include <string>
#include <memory>
#include <random>



class ParticleManager {
public:
	ParticleManager() = default;
	~ParticleManager() = default;

	// 初期化
	void Initialize();
	// 更新処理
	void Update();
	// ImGuiの更新処理
	void UpdateImGui();
	// 描画処理
	void Draw();
	// 終了処理
	void Finalize();

	void UpdatePrimitiveType(const std::string& groupName, PrimitiveType type,const Vector3& param);

	/// <summary>
	/// パーティクルグループの生成
	/// </summary>
	/// <param name="particleCommon">パーティクル共通情報</param>
	/// <param name="name">グループ名(固有名)</param>
	/// <param name="filePath">objファイルパス</param>
	void CreateParticleGroup(ParticleCommon* particleCommon,const std::string& name,
		const std::string& filePath,PrimitiveType primitiveType = PRIMITIVE_PLANE);

	void CreateParticleGroup(ParticleCommon* particleCommon, const std::string& presetJson);

	void Emit(const std::string& name, const Vector3& emitPosition, uint32_t count);

	uint32_t EmitterAllocate();

	BaseParticleGroup* GetParticleGroup(const std::string& name);

	void SetPreset(const std::string& name, const ParticlePreset& preset);


private:

	std::unique_ptr<ParticleEmitterAllocater> emitterAllocater_;

	std::unordered_map<std::string, std::unique_ptr<PrimitiveParticle>> particleGroups_;

};

