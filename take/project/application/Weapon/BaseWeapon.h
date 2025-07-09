#pragma once
#include "Object3d.h"
#include "Object3dCommon.h"
#include "Entity/GameCharacter.h"
#include "Weapon/Bullet/BulletManager.h"
#include "Weapon/WeaponType.h"
#include <cstdint>
#include <string>
#include <memory>

//武器インターフェース
class BaseWeapon {
public:

	BaseWeapon() = default;
	virtual ~BaseWeapon() = default;

	//武器の初期化
	virtual void Initialize(Object3dCommon* object3dCommon, BulletManager* bulletManager, const std::string& filePath) = 0;
	//武器の更新
	virtual void Update() = 0;
	//ImGuiの更新
	virtual void UpdateImGui() = 0;
	//武器の描画
	virtual void Draw() = 0;
	//武器の攻撃
	virtual void Attack() = 0;
	//武器のチャージ処理
	
	virtual void Charge([[maybe_unused]] float deltaTime) {};
	virtual void ChargeAttack() {};

	//武器タイプの取得
	virtual const WeaponType& GetWeaponType() const = 0;

	virtual float GetChargeTime() const { return chargeTime_; }
	virtual float GetAttackInterval() const { return attackInterval_; }
	//所有者の設定
	virtual void SetOwnerObject(GameCharacter* owener) { ownerObject_ = owener; }
	//攻撃対象の座標を設定
	virtual void SetTarget(const Vector3& targetPos) { targetPos_ = targetPos; }

	virtual bool IsCharging() const { return isCharging_; }
	//チャージ攻撃可能か
	virtual bool IsChargeAttack() const = 0;
	//移動撃ち可能か
	virtual bool IsMoveShootable() const = 0;
	//停止撃ち専用か
	virtual bool IsStopShootOnly() const = 0;

protected:
	// 弾管理クラス
	BulletManager* bulletManager_ = nullptr;

	//武器の3Dオブジェクト
	std::unique_ptr<Object3d> object3d_ = nullptr;

	//武器の所有者オブジェクト
	GameCharacter* ownerObject_ = nullptr;
	//攻撃対象の座標
	Vector3 targetPos_;

	//武器の種類
	WeaponType weaponType_ = WeaponType::WEAPON_TYPE_RIFLE;
	//武器の攻撃力
	int32_t attackPower_ = 0;
	//攻撃間隔
	float attackInterval_ = 0.0f;
	//弾数
	int32_t bulletCount_ = 0;
	//弾薬の最大数
	int32_t maxBulletCount_;
	//弾のスピード
	float bulletSpeed_ = 0.0f;

	// チャージ攻撃
	bool isCharging_ = false;  // チャージ攻撃フラグ
	float chargeTime_ = 0.0f;    // チャージ時間
	float chargeMaxTime_ = 2.0f; // 最大チャージ時間

	bool isChargeAttack_  = false; // チャージ攻撃フラグ
	bool isMoveShootable_ = false; // 移動撃ち可能か
	bool isStopShootOnly_ = false; // 停止撃ち専用か
};