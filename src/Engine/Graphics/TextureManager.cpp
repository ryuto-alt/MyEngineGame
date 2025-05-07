#include "TextureManager.h"
#include "StringUtility.h"
#include "SrvManager.h"

using namespace StringUtility;

TextureManager* TextureManager::instance = nullptr;

TextureManager* TextureManager::GetInstance()
{
    if (instance == nullptr) {
        instance = new TextureManager;
    }
    return instance;
}

void TextureManager::Finalize()
{
    delete instance;
    instance = nullptr;
}

void TextureManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager)
{
    assert(dxCommon);
    assert(srvManager);
    dxCommon_ = dxCommon;
    srvManager_ = srvManager;

    // デバッグ出力
    OutputDebugStringA("TextureManager: Initialized successfully\n");
}

const DirectX::TexMetadata& TextureManager::GetMetaData(const std::string& filePath)
{
    // ファイルパスをキーに持つテクスチャデータを取得
    if (textureDatas.count(filePath) <= 0) {
        // テクスチャが存在しない場合はデフォルトテクスチャを返す
        OutputDebugStringA(("TextureManager::GetMetaData - Texture not found: " + filePath + ", using default\n").c_str());
        LoadDefaultTexture();
        return textureDatas[GetDefaultTexturePath()].metadata;
    }
    return textureDatas[filePath].metadata;
}

bool TextureManager::LoadTexture(const std::string& filePath)
{
    // 読み込み済みテクスチャを検索
    if (textureDatas.count(filePath) > 0) {
        OutputDebugStringA(("TextureManager::LoadTexture - Already loaded: " + filePath + "\n").c_str());
        return true; // 読み込み済みなら早期return
    }

    // ファイルが存在するか確認
    DWORD fileAttributes = GetFileAttributesA(filePath.c_str());
    if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
        OutputDebugStringA(("WARNING: TextureManager::LoadTexture - File not found: " + filePath + "\n").c_str());
        // デフォルトテクスチャを読み込む
        LoadDefaultTexture();
        return false;
    }

    try {
        // 最大数チェック
        assert(!srvManager_->IsMaxCount());

        // テクスチャファイルを読んでプログラムで扱えるようにする
        DirectX::ScratchImage image{};
        std::wstring filePathW = ConvertString(filePath);
        HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
        if (FAILED(hr)) {
            OutputDebugStringA(("ERROR: TextureManager::LoadTexture - Failed to load from file: " + filePath + "\n").c_str());
            throw std::runtime_error("Failed to load texture from file");
        }

        // ミニマップの作成
        DirectX::ScratchImage mipImages{};
        hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
        if (FAILED(hr)) {
            OutputDebugStringA("ERROR: TextureManager::LoadTexture - Failed to generate mipmaps\n");
            throw std::runtime_error("Failed to generate mipmaps");
        }

        // テクスチャデータを追加
        TextureData textureData;
        textureData.filePath = filePath;
        textureData.metadata = mipImages.GetMetadata();
        textureData.resource = dxCommon_->CreateTextureResource(textureData.metadata);

        Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = dxCommon_->UploadTextureData(textureData.resource, mipImages);
        dxCommon_->CommandKick();

        // SRVを作成
        textureData.srvIndex = srvManager_->Allocate();
        textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
        textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);

        // SRVの設定
        srvManager_->CreateSRVForTexture2D(
            textureData.srvIndex,
            textureData.resource,
            textureData.metadata.format,
            static_cast<UINT>(textureData.metadata.mipLevels)
        );

        // マップに追加
        textureDatas[filePath] = textureData;

        OutputDebugStringA(("TextureManager::LoadTexture - Successfully loaded: " + filePath + "\n").c_str());
        OutputDebugStringA(("TextureManager::LoadTexture - SRV index: " + std::to_string(textureData.srvIndex) + "\n").c_str());
        return true;
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: TextureManager::LoadTexture - Failed to load texture - " + filePath + " - " + e.what() + "\n").c_str());
        // エラー時もデフォルトテクスチャを読み込む
        LoadDefaultTexture();
        return false;
    }
}

void TextureManager::LoadDefaultTexture()
{
    const std::string& defaultTexturePath = GetDefaultTexturePath();

    // すでに読み込み済みなら何もしない
    if (textureDatas.count(defaultTexturePath) > 0) {
        return;
    }

    // まずデフォルトテクスチャを実際のファイルから読み込もうとする
    DWORD fileAttributes = GetFileAttributesA(defaultTexturePath.c_str());
    if (fileAttributes != INVALID_FILE_ATTRIBUTES) {
        // ファイルが存在する場合は通常通り読み込む
        try {
            DirectX::ScratchImage image{};
            std::wstring filePathW = ConvertString(defaultTexturePath);
            HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
            if (SUCCEEDED(hr)) {
                // ミニマップの作成
                DirectX::ScratchImage mipImages{};
                hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
                if (SUCCEEDED(hr)) {
                    // テクスチャデータを追加
                    TextureData textureData;
                    textureData.filePath = defaultTexturePath;
                    textureData.metadata = mipImages.GetMetadata();
                    textureData.resource = dxCommon_->CreateTextureResource(textureData.metadata);

                    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = dxCommon_->UploadTextureData(textureData.resource, mipImages);
                    dxCommon_->CommandKick();

                    // SRVを作成
                    textureData.srvIndex = srvManager_->Allocate();
                    textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
                    textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);

                    // SRVの設定
                    srvManager_->CreateSRVForTexture2D(
                        textureData.srvIndex,
                        textureData.resource,
                        textureData.metadata.format,
                        static_cast<UINT>(textureData.metadata.mipLevels)
                    );

                    // マップに追加
                    textureDatas[defaultTexturePath] = textureData;

                    OutputDebugStringA("TextureManager::LoadDefaultTexture - Default texture loaded from file successfully\n");
                    return;
                }
            }
        }
        catch (...) {
            // エラーが発生した場合は下のコードでメモリ上に白テクスチャを生成する
            OutputDebugStringA("TextureManager::LoadDefaultTexture - Failed to load default texture from file, creating in memory\n");
        }
    }

    // ファイルが存在しないか読み込みに失敗した場合は1x1の白テクスチャを動的に生成
    try {
        // 1x1の白テクスチャを動的に生成
        const uint32_t textureWidth = 1;
        const uint32_t textureHeight = 1;
        const uint32_t pixelSize = 4; // RGBA

        // 白ピクセルデータ (RGBA: 255, 255, 255, 255)
        std::vector<uint8_t> pixelData(textureWidth * textureHeight * pixelSize, 255);

        // DirectX::ScratchImageを作成
        DirectX::ScratchImage image;
        image.Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM, textureWidth, textureHeight, 1, 1);
        memcpy(image.GetPixels(), pixelData.data(), pixelData.size());

        // テクスチャデータを追加
        TextureData textureData;
        textureData.filePath = defaultTexturePath;
        textureData.metadata = image.GetMetadata();
        textureData.resource = dxCommon_->CreateTextureResource(textureData.metadata);

        // アップロードとSRV作成処理
        Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = dxCommon_->UploadTextureData(textureData.resource, image);
        dxCommon_->CommandKick();

        // SRVを作成
        textureData.srvIndex = srvManager_->Allocate();
        textureData.srvHandleCPU = srvManager_->GetCPUDescriptorHandle(textureData.srvIndex);
        textureData.srvHandleGPU = srvManager_->GetGPUDescriptorHandle(textureData.srvIndex);

        // SRVの設定
        srvManager_->CreateSRVForTexture2D(
            textureData.srvIndex,
            textureData.resource,
            textureData.metadata.format,
            static_cast<UINT>(textureData.metadata.mipLevels)
        );

        // マップに追加
        textureDatas[defaultTexturePath] = textureData;

        OutputDebugStringA("TextureManager::LoadDefaultTexture - Default white texture created in memory successfully\n");
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: TextureManager::LoadDefaultTexture - Failed to create default texture - " + std::string(e.what()) + "\n").c_str());
    }
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath)
{
    // ファイルパスをキーに持つテクスチャデータを取得
    if (textureDatas.count(filePath) <= 0) {
        // テクスチャが存在しない場合はデフォルトテクスチャを返す
        OutputDebugStringA(("TextureManager::GetSrvHandleGPU - Texture not found: " + filePath + ", using default\n").c_str());
        LoadDefaultTexture();
        return textureDatas[GetDefaultTexturePath()].srvHandleGPU;
    }
    return textureDatas[filePath].srvHandleGPU;
}

uint32_t TextureManager::GetSrvIndex(const std::string& filePath)
{
    // ファイルパスをキーに持つテクスチャデータを取得
    if (textureDatas.count(filePath) <= 0) {
        // テクスチャが存在しない場合はデフォルトテクスチャを返す
        OutputDebugStringA(("TextureManager::GetSrvIndex - Texture not found: " + filePath + ", using default\n").c_str());
        LoadDefaultTexture();
        return textureDatas[GetDefaultTexturePath()].srvIndex;
    }
    return textureDatas[filePath].srvIndex;
}