#pragma once
#include "ResourceDataStructure.h"
#include "Transform.h"
#include "TransformMatrix.h"
#include "DirectXCommon.h"
#include "math/AABB.h"
#include "3d/Model.h"
#include "3d/Particle/ParticleAttribute.h"


#include <d3d12.h>
#include <wrl.h>
#include <random>
#include <list>

//Particle1個分のデータ
struct Particle {
	EulerTransform transforms_;  //位置
	Vector3 velocity_; 	    //速度
	Vector4 color_;         //色
	float lifeTime_;        //寿命
	float currentTime_;     //経過時間
};


class ParticleCommon;
class BaseParticleGroup {
public:
	BaseParticleGroup() = default;
	virtual ~BaseParticleGroup() = default;

	virtual void Initialize(ParticleCommon* particleCommon, const std::string& filePath) = 0;

	virtual void Update() = 0;

	virtual void UpdateImGui() = 0;

	virtual void Draw();

	/// <summary>
	/// パーティクルの生成
	/// </summary>
	Particle MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate);

	/// <summary>
	/// パーティクルの発生
	/// </summary>
	std::list<Particle> Emit(const Vector3& emitterPos, uint32_t particleCount);

	//bool IsCollision(const AABB& aabb, const Vector3& point);

	void SpliceParticles(std::list<Particle> particles);

	const ParticlePreset& GetPreset() const { return particlePreset_; }

	virtual void SetPreset(const ParticlePreset& preset);

	void SetEmitterPosition(const Vector3& position);

protected:

	//Particleの総数
	static const uint32_t kNumMaxInstance_ = 10240; 
	//1フレームの時間
	const float kDeltaTime_ = 1.0f / 60.0f;
	//描画するインスタンス数
	uint32_t numInstance_ = 0; 
	//Particleの配列
	std::list<Particle> particles_; 

	//加速フィールド
	//AccelerationField accelerationField_;
	//パーティクルの属性
	ParticlePreset particlePreset_;

protected:

	//ParticleCommon
	ParticleCommon* particleCommon_ = nullptr;

	//エミッターの座標
	Vector3 emitterPos_ = { 0.0f, 0.0f, 0.0f };

	//instancing用のデータ
	ParticleForGPU* particleData_ = nullptr;
	PerView* perViewData_ = nullptr;
	uint32_t particleSrvIndex_ = 0;
	ComPtr<ID3D12Resource> particleResource_;
	ComPtr<ID3D12Resource> perViewResource_;
};