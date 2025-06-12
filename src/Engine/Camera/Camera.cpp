#include "Camera.h"
#include "RenderingPipeline.h"

// 静的メンバ変数の実体化
Camera* Object3dCommon::defaultCamera_ = nullptr;

Camera::Camera() :
    fovY_(1.57f), // 90度（π/2ラジアン）
    aspectRatio_(16.0f / 9.0f),
    nearClip_(0.1f),
    farClip_(100.0f),
    mouseSensitivity_(0.003f),
    cameraMode_(0)
{
    // トランスフォームの初期設定
    transform_.scale = { 1.0f, 1.0f, 1.0f };
    transform_.rotate = { 0.0f, 0.0f, 0.0f };
    transform_.translate = { 0.0f, 0.0f, -5.0f };

    // 初期更新
    Update();
}

Camera::~Camera() {
    // デフォルトカメラが自分自身だった場合はnullptrにする
    if (Object3dCommon::GetDefaultCamera() == this) {
        Object3dCommon::SetDefaultCamera(nullptr);
    }
}

void Camera::Update() {
    // ワールド行列の計算
    worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

    // ビュー行列の計算（ワールド行列の逆行列）
    viewMatrix_ = Inverse(worldMatrix_);

    // プロジェクション行列の計算
    projectionMatrix_ = MakePerspectiveFovMatrix(fovY_, aspectRatio_, nearClip_, farClip_);

    // ビュープロジェクション行列の計算
    viewProjectionMatrix_ = Multiply(viewMatrix_, projectionMatrix_);
}

// セッター
void Camera::SetRotate(const Vector3& rotate) {
    transform_.rotate = rotate;
}

void Camera::SetTranslate(const Vector3& translate) {
    transform_.translate = translate;
}

void Camera::SetFovY(float fovY) {
    fovY_ = fovY;
}

void Camera::SetAspectRatio(float aspectRatio) {
    aspectRatio_ = aspectRatio;
}

void Camera::SetNearClip(float nearClip) {
    nearClip_ = nearClip;
}

void Camera::SetFarClip(float farClip) {
    farClip_ = farClip;
}

// ゲッター
const Matrix4x4& Camera::GetWorldMatrix() const {
    return worldMatrix_;
}

const Matrix4x4& Camera::GetViewMatrix() const {
    return viewMatrix_;
}

const Matrix4x4& Camera::GetProjectionMatrix() const {
    return projectionMatrix_;
}

const Matrix4x4& Camera::GetViewProjectionMatrix() const {
    return viewProjectionMatrix_;
}

const Vector3& Camera::GetRotate() const {
    return transform_.rotate;
}

const Vector3& Camera::GetTranslate() const {
    return transform_.translate;
}

float Camera::GetFovY() const {
    return fovY_;
}

float Camera::GetAspectRatio() const {
    return aspectRatio_;
}

float Camera::GetNearClip() const {
    return nearClip_;
}

float Camera::GetFarClip() const {
    return farClip_;
}

// マウス視点移動関連
void Camera::ProcessMouseInput(float deltaX, float deltaY) {
    if (cameraMode_ == 0) { // フリーカメラモード
        // Y軸回転 (水平回転)
        transform_.rotate.y += deltaX * mouseSensitivity_;
        
        // X軸回転 (垂直回転) - 制限あり
        transform_.rotate.x += deltaY * mouseSensitivity_;
        
        // X軸回転を-90度から50度に制限
        const float maxPitch = 1.57f;  // 90度
        if (transform_.rotate.x > maxPitch) {
            transform_.rotate.x = maxPitch;
        }
        if (transform_.rotate.x < -maxPitch) {
            transform_.rotate.x = -maxPitch;
        }
    }
}

void Camera::SetMouseSensitivity(float sensitivity) {
    mouseSensitivity_ = sensitivity;
}

void Camera::SetCameraMode(int mode) {
    cameraMode_ = mode;
}

int Camera::GetCameraMode() const {
    return cameraMode_;
}

void Camera::ToggleCameraMode() {
    cameraMode_ = (cameraMode_ + 1) % 2;  // 0と 1を切り替え
}

// カメラ移動関連（フリーカメラモード用）
void Camera::MoveForward(float distance) {
    Vector3 forward = GetForwardVector();
    transform_.translate.x += forward.x * distance;
    transform_.translate.y += forward.y * distance;
    transform_.translate.z += forward.z * distance;
}

void Camera::MoveRight(float distance) {
    Vector3 right = GetRightVector();
    transform_.translate.x += right.x * distance;
    transform_.translate.y += right.y * distance;
    transform_.translate.z += right.z * distance;
}

void Camera::MoveUp(float distance) {
    Vector3 up = GetUpVector();
    transform_.translate.x += up.x * distance;
    transform_.translate.y += up.y * distance;
    transform_.translate.z += up.z * distance;
}

Vector3 Camera::GetForwardVector() const {
    // カメラのワールド行列からZ軸方向（前方向）を取得
    // DirectXは左手座標系でZ+が前方向
    return { worldMatrix_.m[2][0], worldMatrix_.m[2][1], worldMatrix_.m[2][2] };
}

Vector3 Camera::GetRightVector() const {
    // カメラのワールド行列からX軸方向（右方向）を取得
    return { worldMatrix_.m[0][0], worldMatrix_.m[0][1], worldMatrix_.m[0][2] };
}

Vector3 Camera::GetUpVector() const {
    // カメラのワールド行列からY軸方向（上方向）を取得
    return { worldMatrix_.m[1][0], worldMatrix_.m[1][1], worldMatrix_.m[1][2] };
}

// デフォルトカメラの設定・取得
void Object3dCommon::SetDefaultCamera(Camera* camera) {
    defaultCamera_ = camera;
}

Camera* Object3dCommon::GetDefaultCamera() {
    return defaultCamera_;
}