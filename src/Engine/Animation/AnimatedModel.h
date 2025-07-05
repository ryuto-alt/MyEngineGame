#pragma once
#include "Model.h"
#include "Animation.h"
#include "AnimationPlayer.h"
#include "AnimationUtility.h"

// アニメーション付きモデルクラス
class AnimatedModel : public Model {
public:
    // コンストラクタ
    AnimatedModel();
    
    // デストラクタ
    ~AnimatedModel();
    
    // 初期化
    void Initialize(DirectXCommon* dxCommon);
    
    // モデルとアニメーションの読み込み
    void LoadFromFile(const std::string& directoryPath, const std::string& filename);
    
    // GLTFファイルからの読み込み
    void LoadFromGLTF(const std::string& directoryPath, const std::string& filename);
    
    // アニメーションの読み込み
    void LoadAnimation(const std::string& directoryPath, const std::string& filename);
    
    // 更新（アニメーション時刻を進める）
    void Update(float deltaTime);
    
    // アニメーションのローカル変換行列を取得
    Matrix4x4 GetAnimationLocalMatrix();
    
    // アニメーションプレイヤーを取得
    AnimationPlayer& GetAnimationPlayer() { return animationPlayer_; }
    const AnimationPlayer& GetAnimationPlayer() const { return animationPlayer_; }
    
    // アニメーション再生制御
    void PlayAnimation();
    void StopAnimation();
    void PauseAnimation();
    void SetAnimationLoop(bool loop);

private:
    AnimationPlayer animationPlayer_;  // アニメーションプレイヤー
    Animation animation_;              // アニメーションデータ
    std::string rootNodeName_;         // ルートノード名
};