#include "Player.h"
#include "../Engine/Resource/ResourcePreloader.h"
#include <cmath>

Player::Player() {
}

Player::~Player() {
}

void Player::Initialize(Camera* camera) {
    camera_ = camera;
    
    // 初期位置と回転の設定
    position_ = Vector3{0.0f, 0.0f, 0.0f};
    currentRotationY_ = 0.0f;
    targetRotationY_ = 0.0f;
    
    // プリロードされたモデルの取得を試行
    UnoEngine* engine = UnoEngine::GetInstance();
    
    auto preloadedWalk = ResourcePreloader::GetInstance()->GetPreloadedModel("human_walk");
    if (preloadedWalk) {
        OutputDebugStringA("Player: Using preloaded walk model\n");
        animatedModel_ = std::move(preloadedWalk);
        
        // アニメーション登録
        Animation walkAnim = animatedModel_->GetAnimationPlayer().GetAnimation();
        animatedModel_->AddAnimation("walk", walkAnim);
        
        // スニークモデルもプリロード済みを使用
        auto preloadedSneak = ResourcePreloader::GetInstance()->GetPreloadedModel("human_sneak");
        if (preloadedSneak) {
            Animation sneakWalkAnim = preloadedSneak->GetAnimationPlayer().GetAnimation();
            animatedModel_->AddAnimation("sneakWalk", sneakWalkAnim);
            OutputDebugStringA("Player: Using preloaded sneak animation\n");
        } else {
            // フォールバック: 通常読み込み
            Animation sneakWalkAnim = engine->LoadAnimation("Resources/Models/human", "sneakWalk.gltf");
            animatedModel_->AddAnimation("sneakWalk", sneakWalkAnim);
        }
    } else {
        OutputDebugStringA("Player: Preloaded model not found, loading normally\n");
        // フォールバック: 通常読み込み
        animatedModel_ = engine->CreateAnimatedModel();
        animatedModel_->LoadFromFile("Resources/Models/human", "walk.gltf");
        
        Animation walkAnim = animatedModel_->GetAnimationPlayer().GetAnimation();
        animatedModel_->AddAnimation("walk", walkAnim);
        
        Animation sneakWalkAnim = engine->LoadAnimation("Resources/Models/human", "sneakWalk.gltf");
        animatedModel_->AddAnimation("sneakWalk", sneakWalkAnim);
    }
    
    animatedModel_->ChangeAnimation("walk");
    animatedModel_->PlayAnimation();
    
    // Object3Dの作成と設定
    object3d_ = engine->CreateObject3D();
    object3d_->SetModel(static_cast<Model*>(animatedModel_.get()));
    object3d_->SetAnimatedModel(animatedModel_.get());
    object3d_->SetPosition(position_);
    object3d_->SetScale(Vector3{1.0f, 1.0f, 1.0f});
    object3d_->SetRotation(Vector3{0.0f, 3.14f, 0.0f});
    object3d_->SetEnableLighting(true);
    object3d_->SetEnableAnimation(true);
    object3d_->SetCamera(camera_);
    
    // humanモデルのマテリアル情報を確認
    if (animatedModel_) {
        const MaterialData& material = animatedModel_->GetMaterial();
        char debugMsg[512];
        sprintf_s(debugMsg, "Player Human Model Material:\n"
            "  isPBR: %s\n"
            "  BaseColor: R=%.3f, G=%.3f, B=%.3f, A=%.3f\n"
            "  Metallic=%.3f, Roughness=%.3f\n"
            "  Texture: %s\n",
            material.isPBR ? "true" : "false",
            material.baseColorFactor.x, material.baseColorFactor.y,
            material.baseColorFactor.z, material.baseColorFactor.w,
            material.metallicFactor, material.roughnessFactor,
            material.textureFilePath.empty() ? "None" : material.textureFilePath.c_str());
        OutputDebugStringA(debugMsg);
        
        // PBRマテリアルでない場合、強制的にPBRを有効化
        if (!material.isPBR) {
            OutputDebugStringA("Player: Human model is not PBR, forcing PBR settings...\n");
            // モデルのマテリアルをPBRモードに強制変更
            MaterialData& mutableMaterial = const_cast<MaterialData&>(animatedModel_->GetMaterial());
            mutableMaterial.isPBR = true;
            mutableMaterial.baseColorFactor = { 0.8f, 0.7f, 0.6f, 1.0f };  // 人間っぽい肌色
            mutableMaterial.metallicFactor = 0.0f;   // 非金属
            mutableMaterial.roughnessFactor = 0.8f;  // 少し粗い表面
            mutableMaterial.emissiveFactor = { 0.0f, 0.0f, 0.0f };
            mutableMaterial.alphaMode = "OPAQUE";
            mutableMaterial.doubleSided = false;
            
            OutputDebugStringA("Player: Applied PBR material settings to human model\n");
            // SetModelを再呼び出しして更新されたマテリアルを適用
            object3d_->SetModel(static_cast<Model*>(animatedModel_.get()));
        }
        
        // humanモデルに強制的に環境マップを適用
        OutputDebugStringA("Player: Enabling environment map for human model\n");
        object3d_->SetEnableEnvironmentMap(true);
        
        // デフォルトの環境マップテクスチャを設定
        object3d_->SetEnvironmentTexture("Resources/Models/skybox/rostock_laage_airport_4k.dds");
    }
}

void Player::Update(UnoEngine* engine) {
    
    const float deltaTime = engine->GetDeltaTime();
    
    // HandleMovement(engine, deltaTime);  // 新しい統合移動システムを使用するため一時的に無効化
    HandleGamepadFeatures(engine, deltaTime);  // ゲームパッド固有機能（スニーク切り替えなど）
    UpdateAnimation(deltaTime);
    UpdateRotation(engine, deltaTime);
    
    object3d_->SetPosition(position_);
    object3d_->SetRotation(Vector3{0.0f, currentRotationY_, 0.0f});
    object3d_->Update();
}

// ゲームパッド固有機能処理（スニーク切り替えなど）
void Player::HandleGamepadFeatures(UnoEngine* engine, float deltaTime) {
    bool bButtonPressed = engine->IsXboxButtonPressed(0x2000);
    bool bButtonTriggered = bButtonPressed && !previousBButtonPressed_;
    
    // スニーク状態の切り替え（移動中のみ）
    if (bButtonTriggered && isMoving_ && !isBlending_) {
        isSneaking_ = !isSneaking_;
        isBlending_ = true;
        blendTimer_ = 0.0f;
        
        if (isSneaking_) {
            animatedModel_->TransitionToAnimation("sneakWalk", 0.3f);
        } else {
            animatedModel_->TransitionToAnimation("walk", 0.3f);
        }
    }
    
    previousBButtonPressed_ = bButtonPressed;
}

void Player::Draw() {
    if (!object3d_) return;
    object3d_->Draw();
}

void Player::Finalize() {
    if (object3d_) {
        object3d_.reset();
    }
    if (animatedModel_) {
        animatedModel_.reset();
    }
}

void Player::SetDirectionalLight(const DirectionalLight& light) {
    if (object3d_) {
        object3d_->SetDirectionalLight(light);
    }
}

void Player::SetSpotLight(const SpotLight& light) {
    if (object3d_) {
        object3d_->SetSpotLight(light);
    }
}

void Player::SetEnvironmentTexture(const std::string& texturePath) {
    if (object3d_) {
        object3d_->SetEnvironmentTexture(texturePath);
    }
}

void Player::SetEnableEnvironmentMap(bool enable) {
    if (object3d_) {
        object3d_->SetEnableEnvironmentMap(enable);
    }
}

bool Player::GetEnableEnvironmentMap() const {
    if (object3d_) {
        return object3d_->GetEnableEnvironmentMap();
    }
    return false;
}

void Player::SetPosition(const Vector3& position) {
    position_ = position;
    if (object3d_) {
        object3d_->SetPosition(position_);
    }
}

void Player::PauseAnimation() {
    if (animatedModel_) {
        animationPaused_ = true;
        animatedModel_->PauseAnimation();
    }
}

void Player::PlayAnimation() {
    if (animatedModel_) {
        animationPaused_ = false;
        animatedModel_->PlayAnimation();
    }
}

void Player::ResetAnimation() {
    if (object3d_) {
        object3d_->SetAnimationTime(0.0f);
    }
}

void Player::ToggleSneakWalk() {
    if (!animatedModel_ || isBlending_) return;
    
    if (animatedModel_->GetCurrentAnimationName() == "walk") {
        animatedModel_->TransitionToAnimation("sneakWalk", 0.3f);
    } else {
        animatedModel_->TransitionToAnimation("walk", 0.3f);
    }
    
    isBlending_ = true;
    blendTimer_ = 0.0f;
}

std::string Player::GetCurrentAnimationName() const {
    if (!animatedModel_) return "";
    return animatedModel_->GetCurrentAnimationName();
}

float Player::GetBlendProgress() const {
    if (!isBlending_) return 1.0f;
    return blendTimer_ / BLEND_DURATION;
}

void Player::HandleMovement(UnoEngine* engine, float deltaTime) {
    float stickX = engine->GetXboxLeftStickX();
    float stickY = engine->GetXboxLeftStickY();
    bool bButtonPressed = engine->IsXboxButtonPressed(0x2000);
    bool bButtonTriggered = bButtonPressed && !previousBButtonPressed_;
    
    // 矢印キー入力の処理
    float keyboardInputX = 0.0f;
    float keyboardInputY = 0.0f;
    
    if (engine->IsKeyPressed(DIK_LEFTARROW)) {
        keyboardInputX = -1.0f;
    }
    if (engine->IsKeyPressed(DIK_RIGHTARROW)) {
        keyboardInputX = 1.0f;
    }
    if (engine->IsKeyPressed(DIK_UPARROW)) {
        keyboardInputY = 1.0f;
    }
    if (engine->IsKeyPressed(DIK_DOWNARROW)) {
        keyboardInputY = -1.0f;
    }
    
    // スティック入力と矢印キー入力を合成
    float totalX = stickX + keyboardInputX;
    float totalY = stickY + keyboardInputY;
    
    // 入力値を正規化（最大値1.0にクランプ）
    float totalMagnitude = std::sqrt(totalX * totalX + totalY * totalY);
    if (totalMagnitude > 1.0f) {
        totalX /= totalMagnitude;
        totalY /= totalMagnitude;
        totalMagnitude = 1.0f;
    }
    
    float stickMagnitude = std::sqrt(stickX * stickX + stickY * stickY);
    bool previouslyMoving = isMoving_;
    isMoving_ = totalMagnitude > 0.1f;
    
    // 移動開始時のアニメーション設定
    if (isMoving_ && !previouslyMoving && !isBlending_) {
        if (isSneaking_) {
            animatedModel_->TransitionToAnimation("sneakWalk", 0.3f);
        } else {
            animatedModel_->TransitionToAnimation("walk", 0.3f);
        }
    }
    
    // スニーク状態の切り替え
    if (bButtonTriggered && isMoving_ && !isBlending_) {
        isSneaking_ = !isSneaking_;
        isBlending_ = true;
        blendTimer_ = 0.0f;
        
        if (isSneaking_) {
            animatedModel_->TransitionToAnimation("sneakWalk", 0.3f);
        } else {
            animatedModel_->TransitionToAnimation("walk", 0.3f);
        }
    }
    
    previousBButtonPressed_ = bButtonPressed;
    
    // 移動処理（デルタタイム考慮）
    if (isMoving_) {
        float currentSpeed = isSneaking_ ? moveSpeed_ * sneakSpeedMultiplier_ : moveSpeed_;
        Vector3 movement = Vector3{totalX * currentSpeed * deltaTime, 0.0f, totalY * currentSpeed * deltaTime};
        
        moveDirection_ = Vector3{totalX, 0.0f, totalY};
        
        if (totalMagnitude > 0.1f) {
            targetRotationY_ = std::atan2(totalX, totalY);
        }
        
        position_ = position_ + movement;
    }
}

void Player::UpdateAnimation(float deltaTime) {
    // ブレンドタイマーの更新
    if (isBlending_) {
        blendTimer_ += deltaTime;
        
        if (blendTimer_ >= BLEND_DURATION) {
            isBlending_ = false;
            blendTimer_ = 0.0f;
        }
    }
    
    // 移動状態に応じたアニメーション制御
    if (isMoving_) {
        if (animationPaused_) {
            animationPaused_ = false;
            animatedModel_->PlayAnimation();
        }
    } else {
        if (!animationPaused_) {
            animationPaused_ = true;
            animatedModel_->PauseAnimation();
        }
    }
    
    // アニメーションの更新
    if (!animationPaused_) {
        animatedModel_->Update(deltaTime);
    } else {
        animatedModel_->Update(0.0f);
    }
}

void Player::UpdateRotation(UnoEngine* engine, float deltaTime) {
    currentRotationY_ = engine->SmoothRotation(currentRotationY_, targetRotationY_, rotationSmoothingSpeed_, deltaTime);
}

// カメラ方向ベースの移動機能
void Player::MoveForward(float distance) {
    if (!camera_) return;
    Vector3 forward = camera_->GetForwardVector();
    forward.y = 0.0f; // Y軸移動を無効化（水平移動のみ）
    float length = std::sqrt(forward.x * forward.x + forward.z * forward.z);
    if (length > 0.0f) {
        forward.x /= length;
        forward.z /= length;
    }
    position_.x += forward.x * distance;
    position_.z += forward.z * distance;
    
    // 移動方向に応じて回転とアニメーション
    if (distance != 0.0f) {
        targetRotationY_ = std::atan2(forward.x, forward.z);
        
        // 移動状態とアニメーションの管理
        if (!isMoving_ && !isBlending_) {
            isMoving_ = true;
            if (isSneaking_) {
                animatedModel_->TransitionToAnimation("sneakWalk", 0.3f);
            } else {
                animatedModel_->TransitionToAnimation("walk", 0.3f);
            }
        }
    }
}

void Player::MoveBackward(float distance) {
    MoveForward(-distance);
}

void Player::MoveRight(float distance) {
    if (!camera_) return;
    Vector3 right = camera_->GetRightVector();
    right.y = 0.0f; // Y軸移動を無効化（水平移動のみ）
    float length = std::sqrt(right.x * right.x + right.z * right.z);
    if (length > 0.0f) {
        right.x /= length;
        right.z /= length;
    }
    position_.x += right.x * distance;
    position_.z += right.z * distance;
    
    // 移動方向に応じて回転とアニメーション
    if (distance != 0.0f) {
        targetRotationY_ = std::atan2(right.x, right.z);
        
        // 移動状態とアニメーションの管理
        if (!isMoving_ && !isBlending_) {
            isMoving_ = true;
            if (isSneaking_) {
                animatedModel_->TransitionToAnimation("sneakWalk", 0.3f);
            } else {
                animatedModel_->TransitionToAnimation("walk", 0.3f);
            }
        }
    }
}

void Player::MoveLeft(float distance) {
    MoveRight(-distance);
}

// 統合移動関数（複数キー同時押し対応）
void Player::MoveWithCameraDirection(float forward, float right, float deltaTime) {
    if (!camera_) return;
    
    // 入力がない場合は処理しない
    if (forward == 0.0f && right == 0.0f) {
        return;
    }
    
    // カメラの方向ベクトルを取得（水平方向のみ）
    Vector3 cameraForward = camera_->GetForwardVector();
    Vector3 cameraRight = camera_->GetRightVector();
    
    // Y成分を0にして水平移動のみに制限
    cameraForward.y = 0.0f;
    cameraRight.y = 0.0f;
    
    // 正規化
    float forwardLength = std::sqrt(cameraForward.x * cameraForward.x + cameraForward.z * cameraForward.z);
    float rightLength = std::sqrt(cameraRight.x * cameraRight.x + cameraRight.z * cameraRight.z);
    
    if (forwardLength > 0.0f) {
        cameraForward.x /= forwardLength;
        cameraForward.z /= forwardLength;
    }
    if (rightLength > 0.0f) {
        cameraRight.x /= rightLength;
        cameraRight.z /= rightLength;
    }
    
    // 最終的な移動方向を計算
    Vector3 moveDirection;
    moveDirection.x = cameraForward.x * forward + cameraRight.x * right;
    moveDirection.y = 0.0f;
    moveDirection.z = cameraForward.z * forward + cameraRight.z * right;
    
    // 移動方向を正規化
    float moveLength = std::sqrt(moveDirection.x * moveDirection.x + moveDirection.z * moveDirection.z);
    if (moveLength > 0.0f) {
        moveDirection.x /= moveLength;
        moveDirection.z /= moveLength;
        
        // 移動速度を適用
        float currentSpeed = isSneaking_ ? moveSpeed_ * sneakSpeedMultiplier_ : moveSpeed_;
        float distance = currentSpeed * deltaTime;
        
        // プレイヤーの位置を更新
        position_.x += moveDirection.x * distance;
        position_.z += moveDirection.z * distance;
        
        // プレイヤーの向きを移動方向に設定
        targetRotationY_ = std::atan2(moveDirection.x, moveDirection.z);
        
        // 移動状態とアニメーションの管理
        if (!isMoving_ && !isBlending_) {
            isMoving_ = true;
            if (isSneaking_) {
                animatedModel_->TransitionToAnimation("sneakWalk", 0.3f);
                isBlending_ = true;
                blendTimer_ = 0.0f;
            } else {
                animatedModel_->TransitionToAnimation("walk", 0.3f);
                isBlending_ = true;
                blendTimer_ = 0.0f;
            }
        }
    }
}

// カメラフォロー機能（オービットカメラ）
void Player::UpdateCameraFollow() {
    if (!camera_) return;
    
    // プレイヤーの位置をオービットカメラのターゲットに設定
    camera_->SetOrbitTarget(position_);
    
    // オービットカメラの距離と高さを設定
    camera_->SetOrbitDistance(3.0f);  // プレイヤーから3ユニットの距離
    camera_->SetOrbitHeight(2.5f);    // プレイヤーから2.5ユニット上の高さ
    
    // オービットカメラの位置を更新
    camera_->UpdateOrbitCamera();
}

// 移動停止の管理
void Player::StopMoving() {
    if (isMoving_) {
        isMoving_ = false;
        // アニメーションを明示的に停止
        if (!animationPaused_) {
            animationPaused_ = true;
            if (animatedModel_) {
                animatedModel_->PauseAnimation();
            }
        }
    }
}