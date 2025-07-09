#include "Bullet.h"
#include "Collision/SphereCollider.h"
#include "camera/CameraManager.h"
#include "base/TakeCFrameWork.h"
#include "math/Vector3Math.h"
#include "math/Easing.h"

//========================================================================================================
// object3d,colliderの初期化
//========================================================================================================

void Bullet::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {

	//オブジェクト初期化
	if (!object3d_) {
		object3d_ = std::make_unique<Object3d>();
	}
	object3d_->Initialize(object3dCommon, filePath);
	//コライダー初期化
	if (!collider_) {
		collider_ = std::make_unique<SphereCollider>();
		collider_->Initialize(object3dCommon->GetDirectXCommon(), object3d_.get());
	}
	collider_->SetRadius(bulletradius_); // 半径を設定
	//emiiter設定
	//emitter0
	particleEmitter_.resize(2);
	particleEmitter_[0] = std::make_unique<ParticleEmitter>();
	particleEmitter_[0]->Initialize("EnemyEmitter0",{ {1.0f,1.0f,1.0f}, { 0.0f,0.0f,0.0f }, transform_.translate }, 5, 0.001f);
	particleEmitter_[0]->SetParticleName("BulletLight");
	//emitter1
	particleEmitter_[1] = std::make_unique<ParticleEmitter>();
	particleEmitter_[1]->Initialize("EnemyEmitter1", { {1.0f,1.0f,1.0f}, { 0.0f,0.0f,0.0f }, transform_.translate }, 8, 0.001f);
	particleEmitter_[1]->SetParticleName("SmokeEffect");

	deltaTime_ = TakeCFrameWork::GetDeltaTime();

	transform_.translate = { 0.0f, 100.0f, 0.0f };
}

//========================================================================================================
// 更新処理
//========================================================================================================

void Bullet::Update() {

	//移動処理
	transform_.translate += velocity_ * deltaTime_;

	//ライフタイムの減少
	lifeTime_ -= deltaTime_;
	if (lifeTime_ <= 0.0f) {
		isActive_ = false;
	}

	object3d_->SetTranslate(transform_.translate);
	object3d_->Update();
	collider_->Update(object3d_.get());

	//パーティクルエミッターの更新
	particleEmitter_[0]->SetTranslate(transform_.translate);
	particleEmitter_[0]->Update();

	particleEmitter_[1]->SetTranslate(transform_.translate);
	particleEmitter_[1]->Update();
	//MEMO: パーティクルの毎フレーム発生
	particleEmitter_[1]->Emit();
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("SmokeEffect")->SetEmitterPosition(transform_.translate);
	TakeCFrameWork::GetParticleManager()->GetParticleGroup("BulletLight")->SetEmitterPosition(transform_.translate);

}

void Bullet::UpdateImGui() {
	
}

//========================================================================================================
// 描画処理
//========================================================================================================

void Bullet::Draw() {

	object3d_->Draw();
}

//========================================================================================================
// 衝突判定の描画
//========================================================================================================

void Bullet::DrawCollider() {

	collider_->DrawCollider();
}

//========================================================================================================
// 衝突判定の処理
//========================================================================================================

void Bullet::OnCollisionAction(GameCharacter* other) {

	if (characterType_ == CharacterType::PLAYER_BULLET) {
		if (other->GetCharacterType() == CharacterType::ENEMY) {
			//敵に当たった場合の処理
			//敵のHPを減らす
			//enemy_->TakeDamage(damage_);

			//パーティクル射出
			//particleEmitter_->Emit();
			isActive_ = false; //弾を無効化
		}
	} else if (characterType_ == CharacterType::ENEMY_BULLET) {
		//敵の弾の場合の処理
		if (other->GetCharacterType() == CharacterType::PLAYER) {
			//プレイヤーに当たった場合の処理

			//player_->TakeDamage(damage_);
			isActive_ = false; //弾を無効化
		}
	}

}

//========================================================================================================
// 弾の座標、速度の初期化
//========================================================================================================

void Bullet::BulletInitialize(const Vector3& weaponPos, const Vector3& targetPos,const float& speed, CharacterType type) {

	transform_.translate = weaponPos;
	characterType_ = type;
	speed_ = speed;

	//ターゲットまでの方向を求める
	Vector3 direction = targetPos - transform_.translate;
	direction = Vector3Math::Normalize(direction);

	//速度の設定
	velocity_ = direction * speed_;
	lifeTime_ = 2.0f;
	isActive_ = true;
}

//void Bullet::EmitterInitialize(uint32_t count, float frequency) {
//
//	std::string name = "BulletEmitter_" + std::to_string(reinterpret_cast<std::uintptr_t>(this));
//	particleEmitter_->Initialize(name, transform_, count, frequency);
//	particleEmitter_->SetParticleName("HitEffect2");
//}
