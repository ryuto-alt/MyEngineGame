#include "Rifle.h"
#include "TakeCFrameWork.h"

//最大弾数の設定


Rifle::~Rifle() {}

void Rifle::Initialize(Object3dCommon* object3dCommon,BulletManager* bulletManager, const std::string& filePath) {

	bulletManager_ = bulletManager;

	//3dオブジェクトの初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, filePath);

	//武器の初期化
	weaponType_ = WeaponType::WEAPON_TYPE_RIFLE;
	attackPower_ = 10;
	attackInterval_ = kAttackInterval;
	bulletCount_ = 30;
	maxBulletCount_ = 30;
	bulletSpeed_ = 400.0f; // 弾のスピードを設定

	isChargeAttack_ = true; // ライフルはチャージ攻撃可能
	isMoveShootable_ = true; // ライフルは移動撃ち可能
	isStopShootOnly_ = false; // ライフルは停止撃ち専用ではない
}

void Rifle::Update() {

	//攻撃間隔の減少
	attackInterval_ -= TakeCFrameWork::GetDeltaTime();

	if (isBursting_) {
		burstInterval_ -= TakeCFrameWork::GetDeltaTime();
		if (burstInterval_ <= 0.0f && burstCount_ > 0) {
			// 弾発射
			if (ownerObject_->GetCharacterType() == CharacterType::PLAYER) {
				bulletManager_->ShootBullet(object3d_->GetCenterPosition(), targetPos_, bulletSpeed_, CharacterType::PLAYER_BULLET);
			} else if (ownerObject_->GetCharacterType() == CharacterType::ENEMY) {
				bulletManager_->ShootBullet(object3d_->GetCenterPosition(), targetPos_, bulletSpeed_, CharacterType::ENEMY_BULLET);
			}
			bulletCount_--;
			if (bulletCount_ <= 0) {
				bulletCount_ = maxBulletCount_;
			}

			burstCount_--;
			if (burstCount_ > 0) {
				burstInterval_ = kBurstAttackInterval; // 次の弾発射までの間隔を設定
			} else {
				isBursting_ = false; // 三連射終了
			}
		}
	}


	object3d_->Update();
}

void Rifle::UpdateImGui() {

	ImGui::SeparatorText("Rifle");
	ImGui::Text("Charge Time: %.2f", chargeTime_);
	ImGui::Text("Charge Max Time: %.2f", chargeMaxTime_);
	ImGui::Text("Is Charging: %s", isCharging_ ? "Yes" : "No");
	ImGui::SliderFloat("Bullet Speed", &bulletSpeed_, 100.0f, 1000.0f);
	ImGui::SliderInt("Max Bullet Count", &maxBulletCount_, 1, 100);
	ImGui::Separator();
	if(ImGui::TreeNode("Rifle Settings")) {
		ImGui::Text("Weapon Type: Rifle");
		ImGui::Text("Attack Power: %d", attackPower_);
		ImGui::Text("Attack Interval: %.2f", attackInterval_);
		ImGui::Text("Bullet Count: %d", bulletCount_);
		ImGui::Text("Max Bullet Count: %d", maxBulletCount_);
		ImGui::Text("Bullet Speed: %.2f", bulletSpeed_);
		ImGui::Text("Is Charge Attack: %s", isChargeAttack_ ? "Yes" : "No");
		ImGui::Text("Is Move Shootable: %s", isMoveShootable_ ? "Yes" : "No");
		ImGui::Text("Is Stop Shoot Only: %s", isStopShootOnly_ ? "Yes" : "No");
		ImGui::TreePop();
	}
}

void Rifle::Draw() {

	object3d_->Draw();
}

//=====================================================================================
// 攻撃処理
//=====================================================================================

void Rifle::Attack() {

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
	//攻撃力の設定
}

//=====================================================================================
// チャージ攻撃処理
//=====================================================================================

void Rifle::Charge(float deltaTime) {
	// チャージ攻撃フラグが立っていない場合、チャージを開始
	if (!isCharging_) {
		isCharging_ = true;
		chargeTime_ = 0.0f;
	}
	chargeTime_ += deltaTime;

	// チャージ時間が最大に達した場合
	if (chargeTime_ >= chargeMaxTime_) { 

		// チャージ時間を最大に制限
		chargeTime_ = chargeMaxTime_; 
	}

	//TODO: チャージ中のエフェクトやアニメーションをここで処理する

}

//=====================================================================================
// チャージ攻撃実行
//=====================================================================================

void Rifle::ChargeAttack() {
	// チャージ攻撃が開始されていない場合は何もしない
	if(!isCharging_) return;
	isCharging_ = false;

	if (chargeTime_ >= chargeMaxTime_) {
		// 最大チャージ時間に達した場合の処理
		//停止撃ちで三連射
		isBursting_ = true;
		burstCount_ = kMaxBurstCount; // 3連射のカウントを初期化
		burstInterval_ = 0.0f; // 3連射の間隔を初期化

	} else {
		// チャージ時間が最大に達していない場合の処理
		// 通常の攻撃を1回行う
		Attack();
	}

	
	//チャージ時間のリセット
	chargeTime_ = 0.0f;
}

void Rifle::SetOwnerObject(GameCharacter* owner) {
	BaseWeapon::SetOwnerObject(owner);
	object3d_->SetParent(owner->GetObject3d());
}

bool Rifle::IsChargeAttack() const {
	return isChargeAttack_;
}

bool Rifle::IsMoveShootable() const {
	return isMoveShootable_;
}

bool Rifle::IsStopShootOnly() const {
	return isStopShootOnly_;
}
