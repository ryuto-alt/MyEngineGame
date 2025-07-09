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
    char debugMsg[256];
    sprintf_s(debugMsg, "AnimatedModel: Loaded vertices=%d, indices=%d\n", 
        GetVertexCount(), GetIndexCount());
    OutputDebugStringA(debugMsg);
    
    // モデルデータにボーン情報があるかチェック
    const ModelData& modelData = GetModelData();
    if (!modelData.rootNode.children.empty()) {
        // スケルトンの作成
        skeleton_ = std::make_unique<Skeleton>();
        skeleton_->Create(modelData.rootNode);
        
        // SkinClusterの作成
        skinCluster_ = std::make_unique<SkinCluster>();
        SrvManager* srvManager = UnoEngine::GetInstance()->GetSrvManager();
        
        // デバイスをComPtrに変換
        Microsoft::WRL::ComPtr<ID3D12Device> deviceComPtr;
        deviceComPtr.Attach(dxCommon->GetDevice());
        deviceComPtr->AddRef(); // 参照カウントを増やす
        
        skinCluster_->Create(deviceComPtr, srvManager, skeleton_.get(), &modelData);
        
        haveSkeleton_ = true;
        OutputDebugStringA("AnimatedModel: Skeleton created\n");
    } else {
        OutputDebugStringA("AnimatedModel: No skeleton found\n");
    }
}

void AnimatedModel::Update(Animation* animation, float animationTime) {
    if (!haveSkeleton_ || !animation || animation->duration == 0.0f) {
        return;
    }
    
    // アニメーションの適用
    skeleton_->ApplyAnimation(animation, animationTime);
    
    // スケルトンの更新
    skeleton_->Update();
    
    // SkinClusterの更新
    if (skinCluster_) {
        skinCluster_->Update(skeleton_.get());
    }
}

void AnimatedModel::Dispatch() {
    if (!haveSkeleton_ || !skinCluster_) {
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
    }
}