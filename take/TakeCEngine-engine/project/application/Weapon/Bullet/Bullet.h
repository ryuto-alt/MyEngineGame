#pragma once
#include "3d/Object3d.h"
#include "3d/Object3dCommon.h"
#include "Entity/GameCharacter.h"
#include "3d/Particle/ParticleEmitter.h"
#include <cstdint>
#include <string>
#include <memory>

class Bullet : public GameCharacter {
public:
	Bullet() = default;
	~Bullet() = default;
	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath)override;
	void Update() override;
	void UpdateImGui();
	void Draw() override;
	void DrawCollider() override;
	void OnCollisionAction(GameCharacter* other) override;

	void BulletInitialize(const Vector3& weaponPos,const Vector3& targetPos,const float& speed,CharacterType type);

	//void EmitterInitialize(uint32_t count, float frequency);

public:

	EulerTransform GetTransform() const { return transform_; }
	bool GetIsActive() { return isActive_; }

public:

	void SetIsActive(bool isActive) { isActive_ = isActive; }
	void SetVelocity(const Vector3& velocity) { velocity_ = velocity; }
	void SetSpeed(float speed) { speed_ = speed; }
	void SetLifeTime(float lifeTime) { lifeTime_ = lifeTime; }
	void SetTransform(const EulerTransform& transform) { transform_ = transform; }

private:
	enum class BulletType {
		NONE,
		BULLET,
		EXPLOSION,
	};

private:

	EulerTransform transform_{};
	float deltaTime_ = 0.0f;
	Vector3 velocity_ = { 0.0f,0.0f,0.0f };
	float speed_ = 0.0f;
	bool isActive_ = false;
	float lifeTime_ = 0.0f;
	float bulletradius_ = 1.0f; //弾の半径

	std::vector<std::unique_ptr<ParticleEmitter>> particleEmitter_;
};

