#include "Camera.h"
#include "MatrixMath.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "math/Easing.h"
#include "math/Vector3Math.h"

Camera::~Camera() {
	cameraResource_.Reset();
}

void Camera::Initialize(ID3D12Device* device) {
	
	transform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	offset_ = { 0.0f, 0.0f, -5.0f };
	offsetDelta_ = { 0.0f, 0.0f, -50.0f };
	fovX_ = 0.45f;
	aspectRatio_ = float(WinApp::kScreenWidth / 2) / float(WinApp::kScreenHeight / 2);
	nearClip_ = 0.1f;
	farClip_ = 1000.0f;
	worldMatrix_ = MatrixMath::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	viewMatrix_ = MatrixMath::Inverse(worldMatrix_);
	projectionMatrix_ = MatrixMath::MakePerspectiveFovMatrix(fovX_, aspectRatio_, nearClip_, farClip_);
	viewProjectionMatrix_ = MatrixMath::Multiply(viewMatrix_, projectionMatrix_);
	rotationMatrix_ = MatrixMath::MakeIdentity4x4();

	followTargetPosition_ = new Vector3();
	followTargetRotation_ = new Vector3();
	focusTargetPosition_ = new Vector3();

	cameraResource_ = DirectXCommon::CreateBufferResource(device, sizeof(CameraForGPU));
	cameraResource_->SetName(L"Camera::cameraResource_");
	cameraForGPU_ = nullptr;
	cameraResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPU_));
	cameraForGPU_->worldPosition = transform_.translate;

#ifdef _DEBUG
	isDebug_ = true;
#endif // _DEBUG

}

void Camera::Update() {

	if (Input::GetInstance()->TriggerKey(DIK_F1)) {
		isDebug_ = !isDebug_;
	}

	if (isDebug_) {

		UpdateDebugCamera();
		
	} else {

		UpdateGameCamera();
	}
	
	transform_.rotate = QuaternionMath::Normalize(transform_.rotate); // クォータニオンを正規化して数値誤差を防ぐ

	rotationMatrix_ = MatrixMath::MakeRotateMatrix(transform_.rotate);
	worldMatrix_ = MatrixMath::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	viewMatrix_ = MatrixMath::Inverse(worldMatrix_);

	projectionMatrix_ = MatrixMath::MakePerspectiveFovMatrix(
		fovX_,
		aspectRatio_,
		nearClip_,
		farClip_
	);

	viewProjectionMatrix_ = MatrixMath::Multiply(viewMatrix_, projectionMatrix_);

	cameraForGPU_->worldPosition = transform_.translate;
}


void Camera::SetShake(float duration, float range) {
	if (!isShaking_) {
		isShaking_ = true;
		shakeDuration_ = duration;                // シェイク継続時間を設定
		shakeRange_ = range;                      // シェイクの振幅を設定
		originalPosition_ = offsetDelta_; // 元の位置を保存
	}
}

void Camera::ShakeCamera() {
	if (isShaking_) {
		if (shakeDuration_ > 0.0f) {
			// ランダムな揺れを計算
			float offsetX = (static_cast<float>(rand()) / RAND_MAX) * shakeRange_ * 2.0f - shakeRange_;
			float offsetY = (static_cast<float>(rand()) / RAND_MAX) * shakeRange_ * 2.0f - shakeRange_;
			float offsetZ = (static_cast<float>(rand()) / RAND_MAX) * shakeRange_ * 2.0f - shakeRange_;

			// カメラの位置を揺らす
			offsetDelta_ = originalPosition_ + Vector3{ offsetX, offsetY, offsetZ };

			// 残り時間を減らす
			shakeDuration_ -= 1.0f;
		} else {
			// シェイク終了
			offsetDelta_ = originalPosition_; // 元の位置に戻す
			isShaking_ = false;
		}
	}
}

void Camera::UpdateImGui() {
#ifdef _DEBUG
	ImGui::DragFloat3("Translate", &transform_.translate.x, 0.01f);
	ImGui::DragFloat4("Rotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("offset", &offset_.x, 0.01f);
	ImGui::DragFloat3("offsetDelta", &offsetDelta_.x, 0.01f);
	ImGui::DragFloat("yawRot", &yawRot_, 0.01f);
	ImGui::DragFloat("pitchRot", &pitchRot_, 0.01f);
	ImGui::DragFloat("FovX", &fovX_, 0.01f);
	ImGui::Checkbox("isDebug", &isDebug_);
#endif // DEBUG
}

void Camera::UpdateDebugCamera() {

	// クォータニオンで回転を管理
	Quaternion rotationDelta = QuaternionMath::IdentityQuaternion();

	if (Input::GetInstance()->IsPressMouse(1)) {
		// マウス入力による回転計算
		float deltaPitch = (float)Input::GetInstance()->GetMouseMove().lY * 0.001f; // X軸回転
		float deltaYaw = (float)Input::GetInstance()->GetMouseMove().lX * 0.001f;   // Y軸回転

		// クォータニオンを用いた回転計算
		Quaternion yawRotation = QuaternionMath::MakeRotateAxisAngleQuaternion(
			Vector3(0, 1, 0), deltaYaw);
		Quaternion pitchRotation = QuaternionMath::MakeRotateAxisAngleQuaternion(
			QuaternionMath::RotateVector(Vector3(1, 0, 0), yawRotation * transform_.rotate), deltaPitch);

		//回転の補間
		rotationDelta = pitchRotation * yawRotation;
	}

	// 累積回転を更新
	transform_.rotate = rotationDelta * transform_.rotate;

	if (Input::GetInstance()->IsPressMouse(2)) {
		offsetDelta_.x += (float)Input::GetInstance()->GetMouseMove().lX * 0.01f;
		offsetDelta_.y -= (float)Input::GetInstance()->GetMouseMove().lY * 0.01f;
	}

	// オフセットを考慮したワールド行列の計算
	offsetDelta_.z += (float)Input::GetInstance()->GetWheel() * 0.01f;

	// 回転を適用
	offset_ = offsetDelta_;
	offset_ = QuaternionMath::RotateVector(offset_, transform_.rotate);
	transform_.translate = offset_;

}

void Camera::UpdateGameCamera() {
	
	if (cameraStateRequest_) {
		cameraState_ = cameraStateRequest_.value();

		switch (cameraState_) {
		case Camera::GameCameraState::FOLLOW:
			InitializeCameraFollow();
			break;
		case Camera::GameCameraState::LOOKAT:
			InitializeCameraLookAt();
			break;
		default:
			break;
		}

		cameraStateRequest_ = std::nullopt;
	}

	switch (cameraState_) {
	case Camera::GameCameraState::FOLLOW:
		UpdateCameraFollow();
		break;
	case Camera::GameCameraState::LOOKAT:
		UpdateCameraLockOn();
		break;
	default:
		break;
	}
}

void Camera::InitializeCameraFollow() {

	followSpeed_ = 0.1f;
	offset_ = offsetDelta_;
}

void Camera::InitializeCameraLookAt() {

	followSpeed_ = 0.4f;
	offsetDelta_ = Vector3(0.0f, 5.0f, -50.0f);
}

void Camera::UpdateCameraFollow() {

	// クォータニオンで回転を管理
	Quaternion rotationDelta = QuaternionMath::IdentityQuaternion();

	//スティックによる回転計算
	float deltaPitch = stick_.y * 0.02f; // X軸回転
	float deltaYaw = stick_.x * 0.02f;   // Y軸回転

	yawRot_ += deltaYaw;
	pitchRot_ += deltaPitch;

	// クォータニオンを用いた回転計算
	Quaternion yawRotation = QuaternionMath::MakeRotateAxisAngleQuaternion(
		Vector3(0, 1, 0), yawRot_);
	Quaternion pitchRotation = QuaternionMath::MakeRotateAxisAngleQuaternion(
		QuaternionMath::RotateVector(Vector3(1, 0, 0), yawRotation), pitchRot_);

	//回転の合成
	rotationDelta = pitchRotation * yawRotation;

	// オフセットに回転を適用
	offset_ = offsetDelta_;
	offset_ = QuaternionMath::RotateVector(offset_, rotationDelta);

	//カメラ位置の計算
	Vector3 tagetPosition_ = *followTargetPosition_ + offset_;

	transform_.translate = Easing::Lerp(transform_.translate, tagetPosition_, followSpeed_);
	transform_.rotate = Easing::Slerp(transform_.rotate, rotationDelta, followSpeed_);

	//Rスティック押し込みでカメラの状態変更
	if (Input::GetInstance()->TriggerButton(0,GamepadButtonType::RightStick)) {
		cameraStateRequest_ = GameCameraState::LOOKAT;
	}
}

void Camera::UpdateCameraLockOn() {

	// ターゲット方向を正規化
	Vector3 toTarget = Vector3Math::Normalize(*focusTargetPosition_ - transform_.translate);

	// 方向からクォータニオンを計算（Z+を toTarget に合わせる）
	Quaternion targetRotation = QuaternionMath::LookRotation(toTarget, Vector3(0, 1, 0));

	//Vector3 euler = QuaternionMath::toEuler(transform_.rotate);
	//yawRot_ = euler.y;
	//pitchRot_ = euler.x;
	// 回転補間
	transform_.rotate = Easing::Slerp(transform_.rotate, targetRotation, followSpeed_);

	// 高さを持った固定オフセット
	offset_ = offsetDelta_;

	// ターゲットからの相対位置に補間移動
	Vector3 desiredPosition = *followTargetPosition_ + QuaternionMath::RotateVector(offset_, transform_.rotate);
	transform_.translate = Easing::Lerp(transform_.translate, desiredPosition, followSpeed_);



	// 状態切り替え
	if (Input::GetInstance()->TriggerButton(0, GamepadButtonType::RightStick)) {
		cameraStateRequest_ = GameCameraState::FOLLOW;
	}
}




void Camera::SetRotate(const Vector3& rotate) {
	transform_.rotate = { rotate.x, rotate.y, rotate.z,1.0f };
}