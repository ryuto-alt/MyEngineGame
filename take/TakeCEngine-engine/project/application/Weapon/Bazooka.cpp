#include "Bazooka.h"
#include "TakeCFrameWork.h"

void Bazooka::Initialize(Object3dCommon* object3dCommon, BulletManager* bulletManager, const std::string& filePath) {

	//弾薬マネージャの設定
	bulletManager_ = bulletManager;
	//3Dオブジェクトの初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, filePath);
	//武器の初期化
	weaponType_ = WeaponType::WEAPON_TYPE_BAZOOKA;
	attackPower_ = 50; // 攻撃力を設定
	attackInterval_ = kAttackInterval; // 攻撃間隔を設定
	bulletCount_ = 5; // 初期弾数を設定
	maxBulletCount_ = 5; // 最大弾数を設定
	bulletSpeed_ = 500.0f; // 弾のスピードを設定

	isChargeAttack_ = false; // バズーカはチャージ攻撃不可
	isMoveShootable_ = false; // バズーカは移動撃ち不可
	isStopShootOnly_ = true; // バズーカは停止撃ち専用
}

void Bazooka::Update() {

	//攻撃間隔の減少
	attackInterval_ -= TakeCFrameWork::GetDeltaTime();
	//3Dオブジェクトの更新
	object3d_->Update();
}

void Bazooka::UpdateImGui() {
	ImGui::SeparatorText("Bazooka Settings");
	ImGui::Text("Weapon Type: Bazooka");
	ImGui::Text("Attack Power: %d", attackPower_);
	ImGui::Text("Attack Interval: %.2f", attackInterval_);
	ImGui::Text("Bullet Count: %d", bulletCount_);
	ImGui::Text("Max Bullet Count: %d", maxBulletCount_);
	ImGui::Text("Bullet Speed: %.2f", bulletSpeed_);
}

void Bazooka::Draw() {

	object3d_->Draw();
}

void Bazooka::Attack() {

	//攻撃間隔が経過している場合
	if (attackInterval_ >= 0.0f) {
		return;
	}

	if (ownerObject_->GetCharacterType() == CharacterType::PLAYER){
		bulletManager_->ShootBullet(object3d_->GetCenterPosition(), targetPos_,bulletSpeed_, CharacterType::PLAYER_BULLET);
	} else if (ownerObject_->GetCharacterType() == CharacterType::ENEMY) {
		bulletManager_->ShootBullet(object3d_->GetCenterPosition(), targetPos_,bulletSpeed_, CharacterType::ENEMY_BULLET);
	} else {
		return; // キャラクタータイプが不明な場合は攻撃しない
	}

	//弾数の減少
	bulletCount_--;
	if (bulletCount_ <= 0) {
		bulletCount_ = maxBulletCount_;
	}
	//攻撃間隔のリセット
	attackInterval_ = kAttackInterval;
}

void Bazooka::SetOwnerObject(GameCharacter* owner) {

	ownerObject_ = owner;
	// 所有者の3Dオブジェクトを設定
	if (ownerObject_) {
		object3d_->SetParent(ownerObject_->GetObject3d());
	}
}

bool Bazooka::IsChargeAttack() const {
	// バズーカはチャージ攻撃不可
	return isChargeAttack_;
}

bool Bazooka::IsMoveShootable() const {
	// バズーカは移動撃ち不可
	return isMoveShootable_;
}

bool Bazooka::IsStopShootOnly() const {
	// バズーカは停止撃ち専用
	return isStopShootOnly_;
}
