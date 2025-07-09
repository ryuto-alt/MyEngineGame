#pragma once
#include "Model.h"
#include "Animation/Skeleton.h"
#include "Animation/SkinCluster.h"
#include "Animation/Animator.h"

class AnimatedModel : public Model {
public:
    AnimatedModel();
    ~AnimatedModel();
    
    // 初期化（アニメーション用）
    void Initialize(DirectXCommon* dxCommon, const std::string& filePath);
    
    // アニメーションの更新
    void Update(Animation* animation, float animationTime);
    
    // スキニング処理
    void Dispatch() override;
    
    // スケルトンの取得
    Skeleton* GetSkeleton() const { return skeleton_.get(); }
    
    // SkinClusterの取得
    SkinCluster* GetSkinCluster() const { return skinCluster_.get(); }
    
    // アニメーション行列の取得（スキニング処理がない場合に使用）
    Matrix4x4 GetAnimationMatrix() const;
    
private:
    std::unique_ptr<Skeleton> skeleton_;
    std::unique_ptr<SkinCluster> skinCluster_;
    bool haveSkeleton_ = false;
};