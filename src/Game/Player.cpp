#include "Player.h"
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
    
    // モデルの読み込み
    UnoEngine* engine = UnoEngine::GetInstance();
    
    animatedModel_ = engine->CreateAnimatedModel();
    animatedModel_->LoadFromFile("Resources/Models/human", "walk.gltf");
    
    // アニメーションの読み込みと登録
    Animation walkAnim = animatedModel_->GetAnimationPlayer().GetAnimation();
    animatedModel_->AddAnimation("walk", walkAnim);
    
    Animation sneakWalkAnim = engine->LoadAnimation("Resources/Models/human", "sneakWalk.gltf");
    animatedModel_->AddAnimation("sneakWalk", sneakWalkAnim);
    
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
}

void Player::Update(UnoEngine* engine) {
    
    const float deltaTime = engine->GetDeltaTime();
    
    HandleMovement(engine, deltaTime);
    UpdateAnimation(deltaTime);
    UpdateRotation(engine, deltaTime);
    
    object3d_->SetPosition(position_);
    object3d_->SetRotation(Vector3{0.0f, currentRotationY_, 0.0f});
    object3d_->Update();
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