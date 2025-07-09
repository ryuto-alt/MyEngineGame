#pragma once
#include "3d/Particle/BaseParticleGroup.h"

//加速フィールド
struct AccelerationField {
	Vector3 acceleration_; //加速度
	Vector3 position_;     //位置
	AABB aabb_;            //当たり判定
};

class ParticleCommon;
class Particle3d : public BaseParticleGroup {
public:

	Particle3d() = default;
	~Particle3d();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(ParticleCommon* particleCommon,const std::string& filePath) override;

	/// <summary>
	/// 終了処理
	/// </summary>
	void Update() override;

	void UpdateImGui() override;

	/// <summary>
	/// 描画
	/// </summary>
	void Draw() override;

	void SetPreset(const ParticlePreset& preset) override { particlePreset_ = preset; }

	/// <summary>
	/// パーティクルの生成
	/// </summary>
	Particle MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate);

	/// <summary>
	/// パーティクルの発生
	/// </summary>
	std::list<Particle> Emit(const Vector3& emitterPos, uint32_t particleCount);

	void SpliceParticles(std::list<Particle> particles);

private:

	void SetModel(const std::string& filePath);

private:
	//モデル
	Model* model_ = nullptr;

private:

	void UpdateMovement(std::list<Particle>::iterator particleIterator);
};