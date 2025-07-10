#pragma once
#include <string>
#include"externals/DirectXTex/DirectXTex.h"
#include"externals/DirectXTex/d3dx12.h"
#include "DirectXCommon.h"
#include "SrvManager.h"
#include <unordered_map>



class TextureManager
{
private:
	static TextureManager* instance;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = default;
	TextureManager& operator=(TextureManager&) = delete;

	//テクスチャ1枚分のデータ
	struct TexturData {

		DirectX::TexMetadata metadata;
		Microsoft::WRL::ComPtr<ID3D12Resource>resource;
		uint32_t srvIndex;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU;
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU;

	};
public:
	//シングルトンインタンス
	static TextureManager* GetInstance();
	//終了
	void Finalize();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvmanager);

	//メタデータを取得
	const DirectX::TexMetadata& GetMetaData(const std::string&filepath);
	
	//テクスチャファイルの読み込み
	void LoadTexture(const std::string& filePath);

	//SRVインデックスの開始番号
	uint32_t GetTextureIndexByFilePath(const std::string& filePath);


	//テクスチャ番号からCPUハンドルを取得
	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(const std::string& filepath);

	//Srvの最初
	static uint32_t kSRVIndexTop;

private:

	//テクスチャデータ
	
	DirectXCommon* dxCommon_=nullptr;
	std::unordered_map<std::string, TexturData> textureDatas;
	SrvManager* srvmanager = nullptr;

};

