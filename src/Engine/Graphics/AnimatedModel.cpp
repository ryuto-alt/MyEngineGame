#include "AnimatedModel.h"
#include "SrvManager.h"
#include "UnoEngine.h"

AnimatedModel::AnimatedModel() : Model() {
}

AnimatedModel::~AnimatedModel() {
}

void AnimatedModel::Initialize(DirectXCommon* dxCommon, const std::string& filePath) {
    // 基底クラスの初期化
    Model::Initialize(dxCommon);
    
    // UnoEngineの既存のModelLoadを使用して確実に読み込む
    UnoEngine* engine = UnoEngine::GetInstance();
    auto tempModel = engine->LoadModel(filePath);
    
    if (!tempModel) {
        OutputDebugStringA("AnimatedModel: Failed to load model\n");
        return;
    }
    
    // tempModelからデータをコピー
    GetModelDataInternal() = tempModel->GetModelData();
    
    // 頂点・インデックスバッファを作成
    CreateVertexBuffer();
    CreateIndexBuffer();
    
    // デバッグ情報を出力
    char debugMsg[512];
    sprintf_s(debugMsg, "AnimatedModel: Loaded vertices=%d, indices=%d\n", 
        GetVertexCount(), GetIndexCount());
    OutputDebugStringA(debugMsg);
    
    // モデルデータにボーン情報があるかチェック
    const ModelData& modelData = GetModelData();
    
    // デバッグ情報を出力
    sprintf_s(debugMsg, "AnimatedModel: rootNode.children.size() = %d\n", (int)modelData.rootNode.children.size());
    OutputDebugStringA(debugMsg);
    
    sprintf_s(debugMsg, "AnimatedModel: rootNode.name = %s\n", modelData.rootNode.name.c_str());
    OutputDebugStringA(debugMsg);
    
    // rootNodeの子を確認
    for (size_t i = 0; i < modelData.rootNode.children.size(); ++i) {
        sprintf_s(debugMsg, "  Child[%d]: %s\n", (int)i, modelData.rootNode.children[i].name.c_str());
        OutputDebugStringA(debugMsg);
    }
    
    // スケルトンの作成（常に作成するがSkinClusterは条件付き）
    skeleton_ = std::make_unique<Skeleton>();
    skeleton_->Create(modelData.rootNode);
    
    // ジョイント数をチェック
    sprintf_s(debugMsg, "AnimatedModel: Joint count = %d\n", (int)skeleton_->GetJoints().size());
    OutputDebugStringA(debugMsg);
    
    // SkinClusterは条件付きで作成、スケルトンは常に有効
    if (skeleton_->GetJoints().size() > 1) {
        skinCluster_ = std::make_unique<SkinCluster>();
        SrvManager* srvManager = UnoEngine::GetInstance()->GetSrvManager();
        
        // デバイスをComPtrに変換
        Microsoft::WRL::ComPtr<ID3D12Device> deviceComPtr;
        deviceComPtr.Attach(dxCommon->GetDevice());
        deviceComPtr->AddRef(); // 参照カウントを増やす
        
        skinCluster_->Create(deviceComPtr, srvManager, skeleton_.get(), &modelData);
        OutputDebugStringA("AnimatedModel: Skeleton and SkinCluster created\n");
    } else {
        sprintf_s(debugMsg, "AnimatedModel: Not enough joints (%d), SkinCluster disabled but animation enabled\n", 
                  (int)skeleton_->GetJoints().size());
        OutputDebugStringA(debugMsg);
    }
    
    // アニメーション処理は常に有効
    haveSkeleton_ = true;
}

void AnimatedModel::Update(Animation* animation, float animationTime) {
    if (!skeleton_ || !animation || animation->duration == 0.0f) {
        return;
    }
    
    // アニメーションの適用（スケルトンが存在すれば実行）
    skeleton_->ApplyAnimation(animation, animationTime);
    
    // スケルトンの更新
    skeleton_->Update();
    
    // SkinClusterの更新（存在する場合のみ）
    if (skinCluster_) {
        skinCluster_->Update(skeleton_.get());
    }
    
    // デバッグ出力
    char debugMsg[256];
    sprintf_s(debugMsg, "AnimatedModel::Update: animationTime=%.2f, joints=%d, skinCluster=%s\n", 
              animationTime, (int)skeleton_->GetJoints().size(), skinCluster_ ? "yes" : "no");
    OutputDebugStringA(debugMsg);
}

void AnimatedModel::Dispatch() {
    if (!skinCluster_) {
        // SkinClusterがない場合は何もしない（通常の描画を行う）
        return;
    }
    
    // 簡易CPU スキニング処理
    const ModelData& originalData = GetModelData();
    
    // 頂点データを更新（CPU上で変換）
    VertexData* mappedVertices = nullptr;
    if (SUCCEEDED(GetVertexResource()->Map(0, nullptr, reinterpret_cast<void**>(&mappedVertices)))) {
        // SkinClusterで頂点を変換
        skinCluster_->UpdateVertices(skeleton_.get(), &originalData, mappedVertices);
        
        GetVertexResource()->Unmap(0, nullptr);
        OutputDebugStringA("AnimatedModel::Dispatch: Skinning applied\n");
    } else {
        OutputDebugStringA("AnimatedModel::Dispatch: Failed to map vertex buffer\n");
    }
}

Matrix4x4 AnimatedModel::GetAnimationMatrix() const {
    if (!skeleton_ || skeleton_->GetJoints().empty()) {
        return MakeIdentity4x4();
    }
    
    // ルートジョイントの変換行列を返す
    return skeleton_->GetJoints()[skeleton_->GetRoot()].skeletonSpaceMatrix;
}