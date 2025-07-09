#include "BulletPool.h"
#include <cassert>

void BulletPool::Initialize(size_t size) {

	for (int i = 0; i < size; i++) {
		pool_.emplace_back(std::make_unique<Bullet>());
	}
}

void BulletPool::Finalize() {

	pool_.clear();
}

Bullet* BulletPool::GetBullet() {
	for (const auto& bullet : pool_) {
		if (!bullet->GetIsActive()) {
			bullet->SetIsActive(true);
			return bullet.get();
		}
	}

	return nullptr;
}

void BulletPool::UpdateAllBullet() {

	for (const auto& bullet : pool_) {
		if (bullet->GetIsActive()) {
			bullet->Update();
		}
	}
}

void BulletPool::DrawAllBullet() {

	for (const auto& bullet : pool_) {
		if (bullet->GetIsActive()) {
			bullet->Draw();
		}
	}
}

void BulletPool::DrawAllCollider() {

	for (const auto& bullet : pool_) {
		if (bullet->GetIsActive()) {
			bullet->DrawCollider();
		}
	}
}

std::vector<Bullet*> BulletPool::GetPool() {
	
	std::vector<Bullet*> bullets;
	for (const auto& bullet : pool_) {
		bullets.push_back(bullet.get());
	}
	return bullets;
}
