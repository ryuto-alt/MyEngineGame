#pragma once
#include <vector>
#include <memory>
#include <random>
#include "Model.h"
#include "Object3d.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include "Mymath.h"

class Camera;
class DirectXCommon;
class SpriteCommon;

// プロシージャル木生成クラス
class ProceduralTree {
public:
    // 木のパラメータ構造体
    struct TreeParams {
        float trunkHeight = 5.0f;          // 幹の高さ
        float trunkRadius = 0.5f;          // 幹の半径
        int trunkSegments = 6;             // 幹の円周方向の分割数
        int trunkHeightSegments = 3;       // 幹の高さ方向の分割数
        
        float branchLengthRatio = 0.7f;    // 枝の長さの比率（親に対して）
        float branchRadiusRatio = 0.6f;    // 枝の太さの比率（親に対して）
        int branchesPerNode = 3;           // 各ノードの枝の数
        int maxBranchDepth = 3;            // 枝の最大階層深度
        float branchAngleVariance = 0.4f;  // 枝の角度のばらつき
        float branchAngleBase = 0.6f;      // 枝の基本角度（ラジアン）
        
        // 葉のパラメータ
        float leafSize = 0.8f;             // 葉のサイズ
        int leafClustersPerBranch = 3;     // 枝ごとの葉クラスター数
        int leavesPerCluster = 4;          // クラスターごとの葉の数
        
        // 色設定
        Vector4 trunkColor = {0.3f, 0.18f, 0.08f, 1.0f};  // リアルな樹皮色
        Vector4 leafColor = {0.15f, 0.5f, 0.1f, 1.0f};    // リアルな葉の色
        Vector4 leafColorVariation = {0.05f, 0.1f, 0.02f, 0.0f}; // 葉の色のバリエーション
        
        // 木の形状パラメータ
        float trunkCurve = 0.1f;           // 幹の曲がり
        float branchUpwardBias = 0.3f;     // 枝が上向きに生える傾向
    };

private:
    // 枝の情報を保持する構造体
    struct Branch {
        Vector3 startPos;
        Vector3 endPos;
        float radius;
        int depth;
    };

public:
    ProceduralTree();
    ~ProceduralTree();
    
    // 初期化
    void Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon, Camera* camera);
    
    // 木の生成
    void Generate(const TreeParams& params, const Vector3& position);
    
    // 更新
    void Update();
    
    // 描画
    void Draw();
    
    // ライト設定
    void SetDirectionalLight(const DirectionalLight& light);
    void SetSpotLight(const SpotLight& light);
    
    // 木を削除
    void Clear();

private:
    // 幹のメッシュ生成（統合メッシュに追加）
    void AddTrunkToMesh(const TreeParams& params);
    
    // 枝の再帰的生成（統合メッシュに追加）
    void AddBranchesToMesh(const TreeParams& params, const Vector3& startPos, 
                          const Vector3& direction, float length, float radius, int depth, 
                          const Vector3& parentEnd = Vector3{0.0f, 0.0f, 0.0f});
    
    // 葉の生成（統合メッシュに追加）
    void AddLeavesToMesh(const TreeParams& params, const Vector3& position);
    
    // 円柱メッシュの頂点を統合メッシュに追加
    void AddCylinderToMesh(const Vector3& start, const Vector3& end, 
                          float startRadius, float endRadius, int segments, 
                          const Vector4& color);
    
    // 葉用のビルボードメッシュを統合メッシュに追加
    void AddLeafBillboardToMesh(const Vector3& position, float size, const Vector4& color);
    
    // 葉クラスターを生成
    void AddLeafCluster(const TreeParams& params, const Vector3& position, const Vector3& normal);
    
    // ランダムな方向ベクトルを生成
    Vector3 GetRandomDirection(const Vector3& baseDirection, float variance);

private:
    DirectXCommon* dxCommon_;
    SpriteCommon* spriteCommon_;
    Camera* camera_;
    
    // 統合メッシュ用の単一モデルとオブジェクト
    std::unique_ptr<Model> treeModel_;
    std::unique_ptr<Object3d> treeObject_;
    
    // 統合メッシュデータ
    ModelData unifiedMeshData_;
    
    // 木全体の色（頂点カラーとは別）
    Vector4 treeBaseColor_ = {0.4f, 0.25f, 0.15f, 1.0f}; // デフォルトは木の幹の色
    
    // 枝の情報を保持
    std::vector<Branch> branches_;
    
    // 木の基準位置
    Vector3 treePosition_;
    
    // 乱数生成器
    std::mt19937 randomEngine_;
    std::uniform_real_distribution<float> randomDist_;
    
    // ライト情報
    DirectionalLight directionalLight_;
    SpotLight spotLight_;
};