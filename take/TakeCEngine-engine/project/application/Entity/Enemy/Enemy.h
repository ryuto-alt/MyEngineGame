#pragma once
#include "Entity/GameCharacter.h"
#include "Weapon/BaseWeapon.h"
#include "camera/Camera.h"
#include "3d/Particle/ParticleEmitter.h"
#include <optional>

class Enemy : public GameCharacter {

public:
	Enemy() = default;
	~Enemy() override;
	void Initialize(Object3dCommon* object3dCommon, const std::string& filePath) override;
	void Update() override;
	void UpdateImGui();
	void Draw() override;
	void DrawCollider() override;
	void OnCollisionAction(GameCharacter* other) override;

	void WeaponInitialize(Object3dCommon* object3dCommon,BulletManager* bulletManager, const std::string& weaponFilePath);

private:

	enum class Behavior {
		IDLE,
		RUNNING,
		JUMP,
		DASH,
		CHARGEATTACK, //
		HEAVYDAMAGE,
	};

	void InitRunning();
	void InitJump();
	void InitDash();
	void InitHeavyDamage();

	void UpdateRunning();
	void UpdateAttack();
	void UpdateDamage();
	void UpdateJump();
	void UpdateDash();
	void UpdateHeavyDamage();

private:

	//状態遷移リクエスト
	std::optional<Behavior> behaviorRequest_ = std::nullopt;
	//プレイヤーの状態
	Behavior behavior_ = Behavior::IDLE;
	//プレイヤーの武器
	std::unique_ptr<BaseWeapon> weapon_ = nullptr;
	//particleEmitter
	std::vector<std::unique_ptr<ParticleEmitter>> particleEmitter_;

	//標的
	Vector3 targetPos_ = { 0.0f,0.0f,0.0f };
	//移動ベクトル
	Vector3 velocity_ = { 0.0f,0.0f,0.0f };
	//減速率
	float deceleration_ = 1.1f;
	//移動方向
	Vector3 moveDirection_ = { 0.0f,0.0f,1.0f };

	QuaternionTransform transform_ = { {1.0f,1.0f,1.0f}, { 0.0f,0.0f,0.0f,1.0f }, {0.0f,0.0f,0.0f} };

	const float moveSpeed_ = 200.0f;
	const float kMaxMoveSpeed_ = 50.0f;

	float deltaTime_ = 0.0f;
	uint32_t hitPoint_ = 100; //体力

	//ダメージを受けた時のエフェクト適用時間
	float damageEffectTime_ = 0.0f;

	bool isJumping_ = false;
	bool isDashing_ = false;
	bool onGround_ = false;
	bool isDamaged_ = false; //ダメージを受けたかどうか
	
};

