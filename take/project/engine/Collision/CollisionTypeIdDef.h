#pragma once
#include <cstdint>

enum class CollisionTypeIdDef : uint32_t {
	kDefalut,
	kPlayer,
	kPlayerBullet,
	kEnemy,
	kEnemyBullet,
};