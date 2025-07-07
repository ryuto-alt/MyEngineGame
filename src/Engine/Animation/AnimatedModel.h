#pragma once
#include "Model.h"
#include "Animation.h"
#include "AnimationPlayer.h"
#include "AnimationUtility.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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
    // assimpを使用したGLTFローダー
    void LoadFromGLTFWithAssimp(const std::string& directoryPath, const std::string& filename);
    
    // assimpシーンからモデルデータを作成
    void ProcessAssimpScene(const aiScene* scene, const std::string& directoryPath);
    
    // assimpメッシュからジオメトリデータを作成
    void ProcessAssimpMesh(const aiMesh* mesh, const aiScene* scene);
    
    // assimpマテリアルからマテリアルデータを作成
    void ProcessAssimpMaterial(const aiMaterial* material, const std::string& directoryPath);
    
    // assimpノードからアニメーションデータを作成
    void ProcessAssimpAnimation(const aiScene* scene);

    AnimationPlayer animationPlayer_;  // アニメーションプレイヤー
    Animation animation_;              // アニメーション格納するでーた　
    std::string rootNodeName_;         // ルートノード名
    
    Assimp::Importer assimpImporter_;  // assimpインポーター
};