#pragma once
#include "Transform.h"
#include "Matrix4x4.h"
#include "Vector2.h"
#include <string>
#include <d3d12.h>
#include <wrl.h>
#include <optional>

//定数バッファ用の構造体
struct CameraForGPU {
	Vector3 worldPosition;
};

class Camera {
public:
	Camera() = default;
	~Camera();
	void Initialize(ID3D12Device* device);
	void Update();

	void ShakeCamera();

	void UpdateImGui();

private:

	enum class GameCameraState {
		FOLLOW,
		LOOKAT,
	};

	void UpdateDebugCamera();
	void UpdateGameCamera();

	void InitializeCameraFollow();
	void InitializeCameraLookAt();

	void UpdateCameraFollow();
	void UpdateCameraLockOn();

public: //getter
	
	const Matrix4x4& GetWorldMatrix() const { return worldMatrix_; }
	const Matrix4x4& GetViewMatrix() const { return viewMatrix_; }
	const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix_; }
	const Matrix4x4& GetViewProjectionMatrix() const { return viewProjectionMatrix_; }
	const Matrix4x4& GetRotationMatrix() const { return rotationMatrix_; }
	const Vector3& GetTranslate() const { return transform_.translate; }
	const Quaternion& GetRotate()const { return transform_.rotate; }
	const bool& GetIsShaking() const { return isShaking_; }
	const Microsoft::WRL::ComPtr<ID3D12Resource>& GetCameraResource() const { return cameraResource_; }

	const GameCameraState& GetCameraState() const { return cameraState_; }

public: //setter

	void SetTranslate(const Vector3& Translate) { transform_.translate = Translate; }
	void SetRotate(const Vector3& rotate);
	void SetOffset(const Vector3& offset) { offset_ = offset; }
	void SetFovX(const float fovX) { fovX_ = fovX; }
	void SetAspectRatio(const float aspectRatio) { aspectRatio_ = aspectRatio; }
	void SetNearClip(const float nearClip) { nearClip_ = nearClip; }
	void SetFarClip(const float farClip) { farClip_ = farClip; }
	void SetIsShaking(const bool isShaking) { isShaking_ = isShaking; }
	void SetIsDebug(bool isDebug){ isDebug_ = isDebug; }
	void SetShake(float duration, float range);

	void SetStick(const Vector2& stick) { stick_ = stick; }
	void SetFollowTargetPos(const Vector3& target) { *followTargetPosition_ = target; }
	void SetFollowTargetRot(const Vector3& targetRot) { *followTargetRotation_ = targetRot; }
	void SetFocusTargetPos(const Vector3& target) { *focusTargetPosition_ = target; }
private:

	//バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource_;
	CameraForGPU* cameraForGPU_;

	QuaternionTransform transform_;
	Vector3 offset_;
	Vector3 offsetDelta_;
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 viewProjectionMatrix_;
	//回転行列
	Matrix4x4 rotationMatrix_;

	//スティック情報
	Vector2 stick_;

	//カメラの状態リクエスト
	std::optional<GameCameraState> cameraStateRequest_ = std::nullopt;
	//カメラの状態
	GameCameraState cameraState_ = GameCameraState::FOLLOW;

	//追従対象
	Vector3* followTargetPosition_;
	Vector3* followTargetRotation_;
	float followSpeed_ = 0.1f;

	//補足対象
	Vector3* focusTargetPosition_;
	
	//水平方向視野角
	float fovX_;
	//aspect比
	float aspectRatio_;
	//nearクリップ距離
	float nearClip_;
	//farクリップ距離
	float farClip_;
	float yawRot_ = 0.0f;
	float pitchRot_ = 0.0f;

	float pitch_ = 0.0f;
	bool isDebug_ = false;


	bool isShaking_ = false;     // シェイク中かどうか
	float shakeDuration_ = 0.0f; // シェイクの残り時間
	Vector3 originalPosition_;   // シェイク開始前のカメラ位置
	float shakeRange_ = 0.2f;    // シェイクの振幅
};

