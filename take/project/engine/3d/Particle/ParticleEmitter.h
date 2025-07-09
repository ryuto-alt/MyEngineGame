#pragma once
#include "Transform.h"
#include "PipelineStateObject.h"
#include "ResourceDataStructure.h"
#include <cstdint>
#include <memory>
#include <string>

struct EmitterSphereInfo {
	Vector3 translate; //エミッターの位置
	float radius; //射出半径
	float frequency; //発生頻度
	float frequencyTime; //経過時間
	uint32_t particleCount; //発生するParticleの数
	uint32_t isEmit; //発生許可
};

class DirectXCommon;
class SrvManager;
class GPUParticle;
class ParticleEmitter {
public:

	ParticleEmitter() = default;
	~ParticleEmitter();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="emitterName">emitter名</param>
	/// <param name="transforms">SRT</param>
	/// <param name="count">発生させるパーティクルの数</param>
	/// <param name="frequency">発生頻度</param>
	void Initialize(const std::string& emitterName, EulerTransform transforms,uint32_t count, float frequency);

	void InitializeEmitterSphere(DirectXCommon* dxCommon, SrvManager* srvManager);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	void UpdateForGPU();

	/// <summary>
	/// ImGuiの更新
	/// </summary>
	void UpdateImGui();

	void DrawWireFrame();

	/// <summary>
	/// パーティクルの発生
	/// </summary>
	void Emit();

	void EmitParticle(GPUParticle* gpuParticle);

public:

	const std::string& GetEmitterName() const { return emitterName_; }
	const bool IsEmit() const { return isEmit_; }
	const EulerTransform& GetTransforms() const { return transforms_; }
	const uint32_t GetParticleCount() const { return particleCount_; }
	const float GetFrequency() const { return frequency_; }

public:

	void SetParticleName(const std::string& particleName);

	void SetTranslate(const Vector3& translate) {transforms_.translate = translate; }
	void SetRotate(const Vector3& rotate) { transforms_.rotate = rotate; }
	void SetScale(const Vector3& scale) { transforms_.scale = scale; }

	void SetIsEmit(bool isEmit) { isEmit_ = isEmit; }
	void SetFrequency(float frequency) { frequency_ = frequency; }
	void SetParticleCount(uint32_t count) { particleCount_ = count; }

private:
	//Particleの総数
	static const uint32_t kNumMaxInstance_ = 1024;

	//DirectXCommon
	DirectXCommon* dxCommon_;
	//SrvManager
	SrvManager* srvManager_;
	//PSO
	std::unique_ptr<PSO> emitParticlePso_;
	//RootSignature
	ComPtr<ID3D12RootSignature> emitParticleRootSignature_;

	//球状エミッター情報
	EmitterSphereInfo* emitterSphereInfo_ = nullptr;
	uint32_t emitterSphereSrvIndex_ = 0;
	//フレーム時間の情報
	PerFrame* perFrameData_ = nullptr;


	ComPtr<ID3D12Resource> emitterSphereResource_;
	ComPtr<ID3D12Resource> perFrameResource_;

	bool isEmit_; //発生フラグ
	EulerTransform transforms_;   //エミッターの位置
	uint32_t particleCount_; //発生するParticleの数
	float frequency_;        //発生頻度
	float frequencyTime_;    //経過時間
	std::string emitterName_; //emitterの名前
	std::string particleName_; //発生させるParticleの名前

};

