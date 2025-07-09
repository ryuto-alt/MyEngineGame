#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <memory>
#include "PipelineStateObject.h"

class Camera;
class DirectXCommon;
class SrvManager;
class ParticleCommon {
public:

	//エイリアステンプレート
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	static ParticleCommon* GetInstance();

	void Initialize(DirectXCommon* dxCommon,SrvManager* srvManager);

	void UpdateImGui();

	void Finalize();

	void PreDraw();

	void PreDrawForGPUParticle();

	void DispatchForGPUParticle();

	void DispatchUpdateParticle();

	//================================================================================================
	// 	   getter
	//================================================================================================

	DirectXCommon* GetDirectXCommon() const { return dxCommon_; }

	SrvManager* GetSrvManager() const { return srvManager_; }

	PSO* GetGraphicPSO() const { return graphicPso_.get(); }

	PSO* GetGraphicPSOForGPUParticle() const { return graphicPsoForGPUParticle_.get(); }

	PSO* GetComputePSOForGPUParticle() const { return computePsoForGPUParticle_.get(); }

	Camera* GetDefaultCamera() const { return defaultCamera_; }


	//================================================================================================
	// 	   setter
	//================================================================================================

	void SetDefaultCamera(Camera* camera) { defaultCamera_ = camera; }


private:

	ParticleCommon() = default;
	~ParticleCommon() = default;
	ParticleCommon(const ParticleCommon&) = delete;
	ParticleCommon& operator=(const ParticleCommon&) = delete;

private:

	static ParticleCommon* instance_;

	DirectXCommon* dxCommon_ = nullptr;

	std::unique_ptr<PSO> graphicPso_ = nullptr;
	std::unique_ptr<PSO> graphicPsoForGPUParticle_ = nullptr;
	std::unique_ptr<PSO> computePsoForGPUParticle_ = nullptr;
	std::unique_ptr<PSO> psoUpdateParticle_ = nullptr;

	SrvManager* srvManager_ = nullptr;

	//RootSignature
	ComPtr<ID3D12RootSignature> graphicRootSignature_ = nullptr;
	ComPtr<ID3D12RootSignature> graphicRootSignatureForGPUParticle_ = nullptr;
	ComPtr<ID3D12RootSignature> computeRootSignatureForGPUParticle_ = nullptr;
	ComPtr<ID3D12RootSignature> rootSignatureUpdateParticle_ = nullptr;

	//defaultCamera
	Camera* defaultCamera_ = nullptr;
};

