#pragma once

#include "Model.h"
#include "Sprite.h"
#include "DirectXCommon.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace Graphics {

// グラフィックスリソース管理クラス
class GraphicsManager {
public:
    // シングルトン
    static GraphicsManager* GetInstance();
    
    // 初期化
    void Initialize(DirectXCommon* dxCommon);
    
    // モデル読み込み
    std::shared_ptr<Model> LoadModel(const std::string& directoryPath, const std::string& filename);
    
    // スプライト作成
    std::shared_ptr<Sprite> CreateSprite(const std::string& textureFilePath);
    
private:
    // コンストラクタ
    GraphicsManager() = default;
    // デストラクタ
    ~GraphicsManager() = default;
    
    // DirectXCommon
    DirectXCommon* dxCommon_ = nullptr;
    
    // リソースキャッシュ
    std::unordered_map<std::string, std::shared_ptr<Model>> modelCache_;
    std::unordered_map<std::string, std::shared_ptr<Sprite>> spriteCache_;
};

} // namespace Graphics