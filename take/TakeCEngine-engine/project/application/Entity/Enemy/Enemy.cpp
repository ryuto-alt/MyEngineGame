#include "Enemy.h"
#include "Collision/BoxCollider.h"
#include "Collision/SphereCollider.h"
#include "3d/Object3d.h"
#include "3d/Object3dCommon.h"
#include "Model.h"
#include "Input.h"
#include "camera/CameraManager.h"
#include "engine/base/TakeCFrameWork.h"
#include "Vector3Math.h"
#include "math/Easing.h"

#include "Weapon/Rifle.h"

Enemy::~Enemy() {
	object3d_.reset();
	collider_.reset();
}

//========================================================================================================
//　初期化
//========================================================================================================


void Enemy::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {


	//キャラクタータイプ設定
	characterType_ = CharacterType::ENEMY;

	behaviorRequest_ = Behavior::RUNNING;
	//オブジェクト初期化
	object3d_ = std::make_unique<Object3d>();
	object3d_->Initialize(object3dCommon, filePath);
	//コライダー初期化
	collider_ = std::make_unique<BoxCollider>();
	collider_->Initialize(object3dCommon->GetDirectXCommon(), object3d_.get());
	collider_->SetHalfSize({ 1.0f, 2.0f, 1.0f }); // コライダーの半径を設定

	//emiiter設定
	//emitter0
	particleEmitter_.resize(3);
	particleEmitter_[0] = std::make_unique<ParticleEmitter>();
	particleEmitter_[0]->Initialize("EnemyEmitter0",{ {1.0f,1.0f,1.0f}, { 0.0f,0.0f,0.0f }, transform_.translate }, 5, 1.0f);
	particleEmitter_[0]->SetParticleName("DamageSpark");
	//emitter1
	particleEmitter_[1] = std::make_unique<ParticleEmitter>();
	particleEmitter_[1]->Initialize("EnemyEmitter1", { {1.0f,1.0f,1.0f}, { 0.0f,0.0f,0.0f }, transform_.translate }, 10, 1.0f);
	particleEmitter_[1]->SetParticleName("CrossEffect");
	//emitter2
	particleEmitter_[2] = std::make_unique<ParticleEmitter>();
	particleEmitter_[2]->Initialize("EnemyEmitter2", { {1.0f,1.0f,1.0f}, { 0.0f,0.0f,0.0f }, transform_.translate }, 10, 1.0f);
	particleEmitter_[2]->SetParticleName("SparkExplosion");


	deltaTime_ = TakeCFrameWork::GetDeltaTime();
}

void Enemy::WeaponInitialize(Object3dCommon* object3dCommon, BulletManager* bulletManager, const std::string& weaponFilePath) {

	//武器の初期化
	weapon_ = std::make_unique<Rifle>();
	weapon_->Initialize(object3dCommon, bulletManager, weaponFilePath);
	weapon_->SetOwnerObject(this);
}

//========================================================================================================
// 更新処理
//========================================================================================================

void Enemy::Update() {

	if (behaviorRequest_) {

		behavior_ = behaviorRequest_.value();

		switch (behavior_) {
		case Behavior::IDLE:
			break;
		case Behavior::RUNNING:
			InitRunning();
			break;
		case Behavior::JUMP:
			InitJump();
			break;
		case Behavior::DASH:
			InitDash();
			break;
		}

		behaviorRequest_ = std::nullopt;
	}

	switch (behavior_) {
	case Enemy::Behavior::IDLE:
		break;
	case Enemy::Behavior::RUNNING:
		UpdateRunning();
		break;
	case Enemy::Behavior::JUMP:
		UpdateJump();
		break;
	case Enemy::Behavior::DASH:
		UpdateDash();
		break;
	case Enemy::Behavior::CHARGEATTACK:
		//UpdateAttack();
		break;
	case Enemy::Behavior::HEAVYDAMAGE:

	default:
		break;
	}

	//ダメージエフェクトの更新
	if (isDamaged_) {
		damageEffectTime_ -= deltaTime_;
		particleEmitter_[0]->Emit();
		if (damageEffectTime_ <= 0.0f) {
			isDamaged_ = false;
		}
	}

	//Quaternionからオイラー角に変換
	Vector3 eulerRotate = QuaternionMath::toEuler(transform_.rotate);
	//カメラの設定
	//camera_->SetTargetPos(transform_.translate);
	//camera_->SetTargetRot(eulerRotate);

	object3d_->SetTranslate(transform_.translate);
	object3d_->SetRotation(eulerRotate);
	object3d_->Update();
	collider_->Update(object3d_.get());

	weapon_->Update();

	//パーティクルエミッターの更新
	for (auto& emitter : particleEmitter_) {
		emitter->SetTranslate(transform_.translate);
		emitter->Update();
	}
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("DamageSpark")->SetEmitterPosition(transform_.translate);
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("SmokeEffect")->SetEmitterPosition(transform_.translate);
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("SparkExplosion")->SetEmitterPosition(transform_.translate);
}

void Enemy::UpdateImGui() {

#ifdef _DEBUG
	ImGui::Begin("Enemy");
	ImGui::DragFloat3("Translate", &transform_.translate.x, 0.01f);
	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f);
	ImGui::DragFloat4("Rotate", &transform_.rotate.x, 0.01f);
	object3d_->UpdateImGui("Enemy");
	ImGui::End();
#endif // _DEBUG
}

//========================================================================================================
// 描画処理
//========================================================================================================


void Enemy::Draw() {

	object3d_->Draw();
	weapon_->Draw();
}

void Enemy::DrawCollider() {

#ifdef _DEBUG
	collider_->DrawCollider();
#endif // _DEBUG

}

//========================================================================================================
// 当たり判定の処理
//========================================================================================================

void Enemy::OnCollisionAction(GameCharacter* other) {

	if (other->GetCharacterType() == CharacterType::PLAYER) {
		//衝突時の処理

		//particleEmitter_->Emit();
		hitPoint_--;
	}

	if (other->GetCharacterType()  == CharacterType::PLAYER_BULLET) {
		//プレイヤーの弾に当たった場合の処理
		particleEmitter_[1]->Emit();
		particleEmitter_[2]->Emit();
		isDamaged_ = true;
		damageEffectTime_ = 0.5f;
		hitPoint_--;
	}
}

void Enemy::InitRunning() {

	transform_.translate = { 0.0f,0.0f,0.0f };
}

void Enemy::InitJump() {}

void Enemy::InitDash() {}

void Enemy::InitHeavyDamage() {}

void Enemy::UpdateRunning() {


}

void Enemy::UpdateAttack() {}

void Enemy::UpdateDamage() {}

void Enemy::UpdateJump() {}

void Enemy::UpdateDash() {}

void Enemy::UpdateHeavyDamage() {

}
