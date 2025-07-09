#pragma once

class DirectXCommon;
class SrvManager;
class ModelCommon {
public:

	static ModelCommon* GetInstance();

	void Initialize(DirectXCommon* dxCommon,SrvManager* srvManager);

	void Finalize();

	DirectXCommon* GetDirectXCommon() const { return dxCommon_; }

	SrvManager* GetSrvManager() const { return srvManager_; }

private:

	ModelCommon() = default;
	~ModelCommon() = default;
	ModelCommon(const ModelCommon&) = delete;
	ModelCommon& operator=(const ModelCommon&) = delete;

private:

	static ModelCommon* instance_;

	DirectXCommon* dxCommon_ = nullptr;

	SrvManager* srvManager_ = nullptr;
};

