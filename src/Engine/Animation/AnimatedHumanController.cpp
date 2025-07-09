#include "AnimatedHumanController.h"
#include "Logger.h"
#include <string>

AnimatedHumanController::AnimatedHumanController()
    : animatedModel_(nullptr)
    , object3d_(nullptr)
    , position_(Vector3{0.0f, 0.0f, 0.0f})
    , rotation_(Vector3{0.0f, 0.0f, 0.0f})
    , scale_(Vector3{1.0f, 1.0f, 1.0f})
    , color_(Vector4{1.0f, 1.0f, 1.0f, 1.0f})
    , animationTime_(0.0f)
    , animationSpeed_(1.0f)
    , animationPaused_(false)
    , animationDuration_(0.0f)
    , initialized_(false)
    , dxCommon_(nullptr)
    , engine_(nullptr)
{
}

AnimatedHumanController::~AnimatedHumanController() {
    Finalize();
}

void AnimatedHumanController::Initialize(DirectXCommon* dxCommon) {
    dxCommon_ = dxCommon;
    engine_ = UnoEngine::GetInstance();
    
    // アニメーション付きモデルの作成
    animatedModel_ = std::make_unique<AnimatedModel>();
    animatedModel_->Initialize(dxCommon_);
    
    // walk.gltfファイルの読み込み
    const std::string modelPath = "Resources/Models/human";
    const std::string fileName = "walk.gltf";
    
    try {
        animatedModel_->LoadFromFile(modelPath, fileName);
        animatedModel_->PlayAnimation();
        
        // アニメーションの継続時間を取得
        animationDuration_ = animatedModel_->GetAnimationPlayer().GetDuration();
        
        Logger::Log("AnimatedHumanController: walk.gltf loaded successfully");
        Logger::Log("Animation duration: " + std::to_string(animationDuration_) + " seconds");
        
    } catch (const std::exception& e) {
        Logger::Log("AnimatedHumanController: Failed to load walk.gltf - " + std::string(e.what()));
        return;
    }
    
    // 3Dオブジェクトの作成（AnimatedModelを通常のModelとして使用）
    object3d_ = engine_->CreateObject3D();
    if (object3d_) {
        // AnimatedModelをModelとして設定する（キャストして使用）
        object3d_->SetModel(static_cast<Model*>(animatedModel_.get()));
        object3d_->SetPosition(position_);
        object3d_->SetColor(color_);
        Logger::Log("AnimatedHumanController: Object3D created and model set");
    }
    
    initialized_ = true;
    Logger::Log("AnimatedHumanController: Initialization completed");
}

void AnimatedHumanController::Update() {
    if (!initialized_ || !animatedModel_) {
        return;
    }
    
    // アニメーションが一時停止されていない場合、時間を更新
    if (!animationPaused_) {
        animationTime_ += (1.0f / 60.0f) * animationSpeed_;
        
        // アニメーションをループさせる
        if (animationDuration_ > 0.0f) {
            animationTime_ = std::fmod(animationTime_, animationDuration_);
        }
    }
    
    // アニメーションモデルの更新
    animatedModel_->Update(1.0f / 60.0f);
    

    if (object3d_ && animatedModel_) {
        Animation& animation = animatedModel_->GetAnimationPlayer().GetAnimation();
        Skeleton& skeleton = animatedModel_->GetSkeleton();
        SkinCluster& skinCluster = animatedModel_->GetSkinCluster();
        
        if (animation.nodeAnimations.size() > 0) {
            object3d_->ApplyAnimation(skeleton, animation, animationTime_);
            object3d_->SkeletonUpdate(skeleton);
            object3d_->SkinClusterUpdate(skinCluster, skeleton);
        }
    }
    
    // Object3Dの更新
    if (object3d_) {
        object3d_->SetPosition(position_);
        object3d_->SetRotation(rotation_);
        object3d_->SetScale(scale_);
        object3d_->SetColor(color_);
        object3d_->Update();
    }
}

void AnimatedHumanController::Draw() {
    if (!initialized_ || !object3d_) {
        return;
    }
    
    // Object3Dを通じて描画
    object3d_->Draw();
}

void AnimatedHumanController::Finalize() {
    if (object3d_) {
        object3d_.reset();
    }
    
    if (animatedModel_) {
        animatedModel_.reset();
    }
    
    initialized_ = false;
}

void AnimatedHumanController::ResetAnimation() {
    animationTime_ = 0.0f;
    animationPaused_ = false;
    
    if (animatedModel_) {
        animatedModel_->PlayAnimation();
    }
    
    Logger::Log("AnimatedHumanController: Animation reset");
}

void AnimatedHumanController::ToggleAnimation() {
    animationPaused_ = !animationPaused_;
    
    std::string state = animationPaused_ ? "paused" : "playing";
    Logger::Log("AnimatedHumanController: Animation " + state);
}