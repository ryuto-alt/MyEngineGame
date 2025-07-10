#include "ResourcePreloader.h"
#include "DirectXCommon.h"
#include <Windows.h>

ResourcePreloader* ResourcePreloader::GetInstance() {
    static ResourcePreloader instance;
    return &instance;
}

void ResourcePreloader::PreloadAnimatedModel(const std::string& key, const std::string& directoryPath, const std::string& filename, DirectXCommon* dxCommon) {
    OutputDebugStringA(("ResourcePreloader: Preloading model with key: " + key + "\n").c_str());
    
    // 既に存在する場合は何もしない
    if (preloadedModels_.find(key) != preloadedModels_.end()) {
        OutputDebugStringA(("ResourcePreloader: Model already preloaded with key: " + key + "\n").c_str());
        return;
    }
    
    // モデルの作成と読み込み
    auto model = std::make_unique<AnimatedModel>();
    model->Initialize(dxCommon);
    model->LoadFromFile(directoryPath, filename);
    
    // 保存
    preloadedModels_[key] = std::move(model);
    OutputDebugStringA(("ResourcePreloader: Successfully preloaded model with key: " + key + "\n").c_str());
}

std::unique_ptr<AnimatedModel> ResourcePreloader::GetPreloadedModel(const std::string& key) {
    auto it = preloadedModels_.find(key);
    if (it != preloadedModels_.end()) {
        OutputDebugStringA(("ResourcePreloader: Moving preloaded model with key: " + key + "\n").c_str());
        return std::move(it->second);
    }
    
    OutputDebugStringA(("ResourcePreloader: No preloaded model found with key: " + key + "\n").c_str());
    return nullptr;
}

bool ResourcePreloader::HasPreloadedModel(const std::string& key) const {
    return preloadedModels_.find(key) != preloadedModels_.end();
}

void ResourcePreloader::ClearAll() {
    OutputDebugStringA("ResourcePreloader: Clearing all preloaded resources\n");
    preloadedModels_.clear();
}