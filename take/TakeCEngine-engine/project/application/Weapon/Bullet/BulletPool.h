#pragma once
#include "Weapon/Bullet/Bullet.h"
#include <cstdint>
#include <vector>
#include <memory>

class BulletPool {
public:

	BulletPool() = default;
	~BulletPool() = default;

	void Initialize(size_t size);

	void Finalize();

	Bullet* GetBullet();

	//void ReleaseInstance(Bullet* instance);

	void UpdateAllBullet();

	void DrawAllBullet();

	void DrawAllCollider();

	std::vector<Bullet*> GetPool();

private:

	std::vector<std::unique_ptr<Bullet>> pool_;
};