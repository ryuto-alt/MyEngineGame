#pragma once
#include "Collision/Collider.h"

class BoxCollider;
class SphereCollider : public Collider {
public:

	//初期化
	void Initialize(DirectXCommon* dxCommon, Object3d* collisionObject) override;

	void Update(Object3d* collisionObject) override;

	//衝突判定
	bool CheckCollision(Collider* other) override;

	//当たり判定範囲の描画
	void DrawCollider() override;

	Vector3 GetWorldPos() override;

	//半径の取得
	float GetRadius() const { return radius_; }

	void SetRadius(const float& radius) override { radius_ = radius; }

	//OBBとの衝突判定
	bool CheckCollisionOBB(BoxCollider* otherBox);

	//Sphereとの衝突判定
	bool CheckCollisionSphere(SphereCollider* sphere);

private:

	float radius_;
};

