#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include "AnimatedModel.h"

class DirectXCommon;

// リソースのプリロード管理クラス
class ResourcePreloader {
public:
    // シングルトンインスタンスの取得
    static ResourcePreloader* GetInstance();

    // アニメーションモデルのプリロード
    void PreloadAnimatedModel(const std::string& key, const std::string& directoryPath, const std::string& filename, DirectXCommon* dxCommon);

    // プリロードされたモデルの取得（所有権を移動）
    std::unique_ptr<AnimatedModel> GetPreloadedModel(const std::string& key);

    // プリロードされたモデルが存在するか確認
    bool HasPreloadedModel(const std::string& key) const;

    // すべてのプリロードされたリソースをクリア
    void ClearAll();

private:
    ResourcePreloader() = default;
    ~ResourcePreloader() = default;
    ResourcePreloader(const ResourcePreloader&) = delete;
    ResourcePreloader& operator=(const ResourcePreloader&) = delete;

    // プリロードされたモデルの保存
    std::unordered_map<std::string, std::unique_ptr<AnimatedModel>> preloadedModels_;
};