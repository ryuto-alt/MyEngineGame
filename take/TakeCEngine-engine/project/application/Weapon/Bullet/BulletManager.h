#pragma once
#include "Weapon/Bullet/Bullet.h"
#include "Weapon/Bullet/BulletPool.h"
#include "3d/Object3dCommon.h"
#include <string>

class BulletManager {
public:

	BulletManager() = default;
	~BulletManager() = default;

	void Initialize(Object3dCommon* object3dCommon,size_t size);
	void Finalize();
	void UpdateBullet();
	void DrawBullet();
	void DrawCollider();

	void ShootBullet(const Vector3& weaponPos, const Vector3& targetPos, const float& speed, CharacterType type);

	std::vector<Bullet*> GetAllBullets();

private:

	Object3dCommon* object3dCommon_ = nullptr;
	std::string bulletFilePath_;
	std::unique_ptr<BulletPool> bulletPool_;
};