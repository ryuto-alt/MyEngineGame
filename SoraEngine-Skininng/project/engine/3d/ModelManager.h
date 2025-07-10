#pragma once
#include <map>
#include <string>
#include "Model.h"
class ModelManager
{
	static ModelManager* instance;

	ModelManager() = default;
	~ModelManager() = default;
	ModelManager(ModelManager&) = default;
	ModelManager& operator=(ModelManager&) = delete;

public:
	//シングルトンインスタンスの取得
	static ModelManager* GetInstans();
	//終了
	void Finalize();

/// <summary>
/// 初期化
/// </summary>
	void Initialize(DirectXCommon* dxcommon, SrvManager* srvmnager);
/// <summary>
/// モデルの読み込み
/// </summary>
	void LoadModel(const std::string& filePath);
/// <summary>
///	モデル検索
/// </summary>
	Model* FindModel(const std::string& filePath);

private:
	//モデルデータ
	std::map<std::string, std::unique_ptr < Model>> models;

	std::unique_ptr< ModelCommon> modelCommon = nullptr;
	SrvManager* srvmnager_ = nullptr;
};

