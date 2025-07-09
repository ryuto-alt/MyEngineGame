#include "TextureManager.h"
#include "base/SrvManager.h"
#include "Utility/StringUtility.h"
#include "TransformMatrix.h"
#include <cassert>
#include <filesystem>
#include <chrono>


TextureManager* TextureManager::instance_ = nullptr;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///			シングルトンインスタンスの取得
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TextureManager* TextureManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = new TextureManager();
	}
	return instance_;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///			初期化
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TextureManager::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager) {


	//dxCommon_の取得
	dxCommon_ = dxCommon;
	//SRVManagerの取得
	srvManager_ = srvManager;
	//SRVの数と同数
	textureDatas_.reserve(srvManager_->kMaxSRVCount_);
	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///			終了処理
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TextureManager::Finalize() {
	srvManager_ = nullptr;
	dxCommon_ = nullptr;
	textureDatas_.clear();
	delete instance_;
	instance_ = nullptr;
	
}

//=============================================================================================
///			テクスチャの更新チェック
//=============================================================================================

void TextureManager::CheckAndReloadTextures() {

	//テクスチャの更新チェック
	for(auto& [filePath, textureData] : textureDatas_) {
		std::string fullPath = "Resources/images/" + filePath;
		//ファイルの最終更新日時を取得
		time_t newTime = GetFileLastWriteTime(fullPath);

		if (fileUpdateTimes_[filePath] != newTime) {
			//更新されていたら再読み込み
			LoadTexture(filePath, true);
			//更新日時を更新
			fileUpdateTimes_[filePath] = newTime;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///			テクスチャファイルの読み込み
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TextureManager::LoadTexture(const std::string& filePath,bool forceReload) {

	//読み込み済みテクスチャを検索
	if (!forceReload && textureDatas_.contains(filePath)) {
		return;
	}

	//テクスチャ枚数上限チェック
	assert(srvManager_->CheckTextureAllocate());

	//テクスチャファイルを読み込んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertString(filePath);
	std::wstring fullPathW = L"Resources/images/" + filePathW;
	HRESULT hr;
	if (filePathW.empty()) {
		hr = DirectX::LoadFromWICFile(L"Resources/images/white1x1.png", DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
		assert(SUCCEEDED(hr));

	} else if(filePathW.ends_with(L".dds")){
		hr = DirectX::LoadFromDDSFile(fullPathW.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
		assert(SUCCEEDED(hr));

	} else {
		hr = DirectX::LoadFromWICFile(fullPathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
		assert(SUCCEEDED(hr));
	}

	//ミップマップの作成
	DirectX::ScratchImage mipImages{};

	//圧縮フォーマットかどうかの判定
	if (DirectX::IsCompressed(image.GetMetadata().format)) {
		mipImages = std::move(image); //圧縮フォーマットの場合はそのまま使う

	} else if (image.GetMetadata().width == 1 && image.GetMetadata().height == 1) {
		mipImages = std::move(image); // 1x1はミップマップ不要

	} else { //非圧縮フォーマットの場合はミップマップを作成

		hr = DirectX::GenerateMipMaps(
			image.GetImages(),
			image.GetImageCount(),
			image.GetMetadata(),
			DirectX::TEX_FILTER_SRGB,
			0, mipImages);

		assert(SUCCEEDED(hr));
	}
	//テクスチャデータを追加して書き込む
	TextureData& textureData = textureDatas_[filePath];

	//テクスチャデータの設定
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource = CreateTextureResource(textureData.metadata);
	textureData.intermediateResource = UploadTextureData(textureData.resource, mipImages);

	//テクスチャデータの要素数番号をSRVのインデックスとして設定
	textureData.srvIndex = srvManager_->Allocate();
	textureData.srvHandleCPU = srvManager_->GetSrvDescriptorHandleCPU(textureData.srvIndex);
	textureData.srvHandleGPU = srvManager_->GetSrvDescriptorHandleGPU(textureData.srvIndex);
	
	//metadataを基にSRVの設定
	srvManager_->CreateSRVforTexture2D(
		textureData.metadata.IsCubemap(),
		textureData.metadata.format,
		UINT(textureData.metadata.mipLevels),
		textureData.resource.Get(),
		textureData.srvIndex
	);
}

///=================================================================================================
///			テクスチャリソースの作成
//==================================================================================================

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::CreateTextureResource(const DirectX::TexMetadata& metadata) {

	//metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width);
	resourceDesc.Height = UINT(metadata.height);
	resourceDesc.MipLevels = UINT16(metadata.mipLevels);
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);
	resourceDesc.Format = metadata.format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);

	//利用するHeapの設定。VRAMを使う設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource;
	HRESULT hr;
	hr = dxCommon_->GetDevice()->CreateCommittedResource(
		&heapProperties, //Heapの設定
		D3D12_HEAP_FLAG_NONE, //Heapの特殊な設定
		&resourceDesc, //Resourceの設定
		D3D12_RESOURCE_STATE_COPY_DEST, //データ転送される設定
		nullptr, //Clear最適値。
		IID_PPV_ARGS(&textureResource));

	assert(SUCCEEDED(hr));
	return textureResource;
}

//============================================================================================
//			テクスチャデータのアップロード
//============================================================================================

ComPtr<ID3D12Resource> TextureManager::UploadTextureData(
	ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages) {

	//サブリソースの作成
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(
		dxCommon_->GetDevice(),
		mipImages.GetImages(),
		mipImages.GetImageCount(),
		mipImages.GetMetadata(),
		subresources);

	//中間リソースのサイズの取得
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(),0,UINT(subresources.size()));

	//中間リソースの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = nullptr;
	intermediateResource = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), intermediateSize);

	//中間リソースにデータをコピー
	UpdateSubresources(dxCommon_->GetCommandList(),texture.Get(),
	intermediateResource.Get(),0,0,UINT(subresources.size()),subresources.data());

	//Textureへ転送後に利用できるようにResourseStateを変更
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	dxCommon_->GetCommandList()->ResourceBarrier(1, &barrier);

	return intermediateResource;
}

//============================================================================================
///			テクスチャのSRVハンドルを取得
//============================================================================================

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(const std::string& filePath) {
	assert(textureDatas_.contains(filePath));
	return textureDatas_.at(filePath).srvHandleGPU;
}

//============================================================================================
// テクスチャのSRVハンドルを取得
//============================================================================================

uint32_t TextureManager::GetSrvIndex(const std::string& filePath) {

	//読み込み済みテクスチャを検索
	if (textureDatas_.contains(filePath)) {
		return textureDatas_.at(filePath).srvIndex;
	}
	assert(false);
	return 0;
}

//============================================================================================
// テクスチャのメタデータを取得
//============================================================================================

const DirectX::TexMetadata& TextureManager::GetMetadata(const std::string& filePath) {
	assert(textureDatas_.contains(filePath));
	return textureDatas_.at(filePath).metadata;
}

//=====================================================================
// 読み込んだテクスチャのファイル名を取得
//=====================================================================

std::vector<std::string> TextureManager::GetLoadedTextureFileNames() const {
	std::vector<std::string> fileNames;
	for (const auto& pair : textureDatas_) {
		fileNames.push_back(pair.first);
	}
	return fileNames;
}

//============================================================================================
//			ファイルの最終更新日時を取得
//============================================================================================

time_t TextureManager::GetFileLastWriteTime(const std::string& filePath) {
	namespace fs = std::filesystem;
	namespace chrono = std::chrono;
	try {
		if(!fs::exists(filePath)) {
			return 0;
		}

		//ファイルの最終更新日時を取得
		auto ftime = fs::last_write_time(filePath);
		// std::filesystem::file_time_typeはクロックの型なので、system_clockに変換
		auto sctp = chrono::clock_cast<chrono::system_clock>(ftime);
		return chrono::system_clock::to_time_t(sctp);
		

	}catch (...) {
		// エラー処理
		assert(false && "ファイルの最終更新日時の取得に失敗しました。");
		return 0;
	}
}