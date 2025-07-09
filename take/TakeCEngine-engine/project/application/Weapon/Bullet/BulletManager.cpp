#include "BulletManager.h"

//========================================================================================================
//　初期化
//========================================================================================================

void BulletManager::Initialize(Object3dCommon* object3dCommon,size_t size) {
	bulletPool_ = std::make_unique<BulletPool>();
	bulletPool_->Initialize(size);
	object3dCommon_ = object3dCommon;
	bulletFilePath_ = "cube.obj";
}

//========================================================================================================
// 終了処理
//========================================================================================================

void BulletManager::Finalize() {

	bulletPool_->Finalize();
	object3dCommon_ = nullptr;
}

//========================================================================================================
// 全弾の更新処理
//========================================================================================================

void BulletManager::UpdateBullet() {
	bulletPool_->UpdateAllBullet();
}

//========================================================================================================
// 全弾の描画処理
//========================================================================================================

void BulletManager::DrawBullet() {
	bulletPool_->DrawAllBullet();
}

void BulletManager::DrawCollider() {

	bulletPool_->DrawAllCollider();
}

//========================================================================================================
// 弾の初期化
//========================================================================================================

void BulletManager::ShootBullet(const Vector3& weaponPos,const Vector3& targetPos,const float& speed,CharacterType type) {

	Bullet* bullet = bulletPool_->GetBullet();
	bullet->Initialize(object3dCommon_, bulletFilePath_);
	bullet->BulletInitialize(weaponPos, targetPos,speed,type);
	//bullet->EmitterInitialize(10, 0.1f); // 10個のパーティクルを0.1秒間隔で発生させる
}

std::vector<Bullet*> BulletManager::GetAllBullets() {
	
	std::vector<Bullet*> bullets;
	for (const auto& bullet : bulletPool_->GetPool()) {
		if (bullet->GetIsActive()) {
			bullets.push_back(bullet);
		}
	}
	return bullets;
}
