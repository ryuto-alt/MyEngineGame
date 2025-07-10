#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
class ModelCommon
{
public:

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvMnager);

	//DXCommon
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
	SrvManager* GetSRVManager() { return srvMnager_; }

private:
	DirectXCommon* dxCommon_;
	SrvManager* srvMnager_ = nullptr;


};

