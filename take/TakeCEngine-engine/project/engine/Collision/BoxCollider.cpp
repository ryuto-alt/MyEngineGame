#include "BoxCollider.h"
#include "3d/Object3dCommon.h"
#include "MatrixMath.h"
#include "SphereCollider.h"
#include "Model.h"
#include "ModelManager.h"
#include "CameraManager.h"
#include "DirectXCommon.h"
#include "application/Entity/GameCharacter.h"
#include "TakeCFrameWork.h"

#include <cmath>

//=============================================================================
// 初期化
//=============================================================================

void BoxCollider::Initialize(DirectXCommon* dxCommon, Object3d* collisionObject) {

	dxCommon_ = dxCommon;

	rotateMatrix_ = MatrixMath::MakeRotateMatrix(collisionObject->GetRotation());

	obb_.center = collisionObject->GetCenterPosition();
	obb_.axis[0].x = rotateMatrix_.m[0][0];
	obb_.axis[0].y = rotateMatrix_.m[0][1];
	obb_.axis[0].z = rotateMatrix_.m[0][2];

	obb_.axis[1].x = rotateMatrix_.m[1][0];
	obb_.axis[1].y = rotateMatrix_.m[1][1];
	obb_.axis[1].z = rotateMatrix_.m[1][2];

	obb_.axis[2].x = rotateMatrix_.m[2][0];
	obb_.axis[2].y = rotateMatrix_.m[2][1];
	obb_.axis[2].z = rotateMatrix_.m[2][2];

	obb_.halfSize = halfSize_;

	//CPUで動かす用のTransform
	transform_ = {
		collisionObject->GetScale(),
		collisionObject->GetRotation(),
		obb_.center
	};

	//アフィン行列
	worldMatrix_ = MatrixMath::MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	//カメラのセット
	camera_ = CameraManager::GetInstance()->GetActiveCamera();
}

void BoxCollider::Update(Object3d* collisionObject) {

	transform_ = collisionObject->GetTransform();
	obb_.center = collisionObject->GetCenterPosition();

	Matrix4x4 scaleMat = MatrixMath::MakeScaleMatrix(transform_.scale);

	rotateMatrix_ = MatrixMath::MakeRotateMatrix(transform_.rotate);

	obb_.axis[0].x = rotateMatrix_.m[0][0];
	obb_.axis[0].y = rotateMatrix_.m[0][1];
	obb_.axis[0].z = rotateMatrix_.m[0][2];

	obb_.axis[1].x = rotateMatrix_.m[1][0];
	obb_.axis[1].y = rotateMatrix_.m[1][1];
	obb_.axis[1].z = rotateMatrix_.m[1][2];

	obb_.axis[2].x = rotateMatrix_.m[2][0];
	obb_.axis[2].y = rotateMatrix_.m[2][1];
	obb_.axis[2].z = rotateMatrix_.m[2][2];

	color_ = { 1.0f,1.0f,1.0f,1.0f };

	Matrix4x4 translateMat = MatrixMath::MakeTranslateMatrix(obb_.center);

	//アフィン行列の更新
	worldMatrix_ = scaleMat * rotateMatrix_ * translateMat;
}
//=============================================================================
// 衝突判定
//=============================================================================

bool BoxCollider::CheckCollision(Collider* other) {

	if (BoxCollider* box = dynamic_cast<BoxCollider*>(other)) {
		return CheckCollisionOBB(box);
	}
	if (SphereCollider* sphere = dynamic_cast<SphereCollider*>(other)) {
		return sphere->CheckCollisionOBB(this);
	}
	return false;
}

//=============================================================================
// 当たり判定範囲の描画
//=============================================================================

void BoxCollider::DrawCollider() {

	TakeCFrameWork::GetWireFrame()->DrawOBB(obb_, color_);
}

Vector3 BoxCollider::GetWorldPos() {
	
	return obb_.center;
}

void BoxCollider::SetHalfSize(const Vector3& halfSize) {

	halfSize_ = halfSize;
	obb_.halfSize = halfSize_;
}

//=============================================================================
// OBBとの衝突判定
//=============================================================================

bool BoxCollider::CheckCollisionOBB(BoxCollider* otherBox) {
	const Vector3* thisAxis = this->obb_.axis;
	const Vector3* otherAxis = otherBox->obb_.axis;

	//他OBBの中心から自OBBの中心へのベクトル
	Vector3 T = otherBox->obb_.center - this->obb_.center;

	//他OBBの座標を自OBBのローカル座標系へ変換
	float rotation[3][3], absRotation[3][3];

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			rotation[i][j] = thisAxis[i].Dot(otherAxis[j]);
			absRotation[i][j] = std::fabs(rotation[i][j]) + 1e-6f;
		}
	}

	// 中心間距離を自OBBのローカル座標系で表現
	float centerToCenter[3] = { T.Dot(thisAxis[0]), T.Dot(thisAxis[1]), T.Dot(thisAxis[2]) };


	// 1. 自OBBの各軸を分離軸として判定
	for (int i = 0; i < 3; i++) {
		float selfProjection =
			obb_.halfSize.x * std::fabs(thisAxis[i].Dot(thisAxis[0])) +
			obb_.halfSize.y * std::fabs(thisAxis[i].Dot(thisAxis[1])) +
			obb_.halfSize.z * std::fabs(thisAxis[i].Dot(thisAxis[2]));

		float otherProjection =
			otherBox->obb_.halfSize.x * absRotation[i][0] +
			otherBox->obb_.halfSize.y * absRotation[i][1] +
			otherBox->obb_.halfSize.z * absRotation[i][2];

		if (std::fabs(centerToCenter[i]) > selfProjection + otherProjection) return false;
	}

	// 2. 他OBBの各軸を分離軸として判定
	for (int i = 0; i < 3; i++) {
		float selfProjection =
			obb_.halfSize.x * absRotation[0][i] +
			obb_.halfSize.y * absRotation[1][i] +
			obb_.halfSize.z * absRotation[2][i];

		float otherProjection =
			otherBox->obb_.halfSize.x * std::fabs(otherAxis[i].Dot(otherAxis[0])) +
			otherBox->obb_.halfSize.y * std::fabs(otherAxis[i].Dot(otherAxis[1])) +
			otherBox->obb_.halfSize.z * std::fabs(otherAxis[i].Dot(otherAxis[2]));

		if (std::fabs(T.Dot(otherAxis[i])) > selfProjection + otherProjection) return false;
	}

	// 3. OBBの交差軸 (Aの3軸 × Bの3軸) を分離軸として判定
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			Vector3 axis = thisAxis[i].Cross(otherAxis[j]); // 交差軸を求める

			// 交差軸がゼロベクトルならスキップ
			if (axis.Length() < 1e-6f) continue;

			axis = axis.Normalize();

			float selfProjection =
				obb_.halfSize.x * std::fabs(thisAxis[0].Dot(axis)) +
				obb_.halfSize.y * std::fabs(thisAxis[1].Dot(axis)) +
				obb_.halfSize.z * std::fabs(thisAxis[2].Dot(axis));

			float otherProjection =
				otherBox->obb_.halfSize.x * std::fabs(otherAxis[0].Dot(axis)) +
				otherBox->obb_.halfSize.y * std::fabs(otherAxis[1].Dot(axis)) +
				otherBox->obb_.halfSize.z * std::fabs(otherAxis[2].Dot(axis));

			float t = std::fabs(T.Dot(axis));
			if (t > selfProjection + otherProjection) return false;
		}
	}

	// すべての軸で分離ができなかった場合、衝突
	return true;
}