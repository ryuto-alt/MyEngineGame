#pragma once
#include "base/DirectXCommon.h"
#include "base/ResourceDataStructure.h"
#include <unordered_map>

class ResourceBarrier {
public:
	
	static ResourceBarrier* GetInstance();

	/// <summary>
	/// リソースバリアの初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon);

	void Finalize();

	void Transition(D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState,ID3D12Resource* resource);

private:

	ResourceBarrier() = default;
	~ResourceBarrier() = default;
	ResourceBarrier(const ResourceBarrier&) = delete;
	ResourceBarrier& operator=(const ResourceBarrier&) = delete;

private:

	static ResourceBarrier* instance_;

	DirectXCommon* dxCommon_ = nullptr;
	
	//リソースごとの状態を管理するためのマップ
	std::unordered_map<ID3D12Resource*, D3D12_RESOURCE_STATES> resourceStates_;
};