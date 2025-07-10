#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "WinApp.h"
#include <array>
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")
#include"externals/DirectXTex/DirectXTex.h"
#include"externals/DirectXTex/d3dx12.h"
#include<vector>
#include <chrono>
#include <thread>  // std::this_thread
#include <Vector4.h>

class DirectXCommon
{
	void DeviceInitialize();
	void CommandInitialize();
	void SwapChainInitialize();
	void DepthBufferInitialize();
	void DescriptorHeepInitialize();
	void RTVInitialize();
	void DSVInitialize();
	void FenceInitialize();
	void ViewportInitialize();
	void ScissorInitialize();
	void DxcCompilerInitialize();
	void ImguiInitialize();

public:
	//初期化
	void Initialize(WinApp* winApp);
	//描画前処理
	
	void Begin();
	//描画後処理
	
	void End();

	//<summary>
	//SRVの指定番号のCPUデスクリプタハンドルを取得
	//</summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriputorHandole(uint32_t index);
	//<summary>
	//SRVの指定番号のGPUデスクリプタハンドルを取得
	//</summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriputorHandole(uint32_t index);

	//<summary>
	//SRVの指定番号のCPUデスクリプタハンドルを取得
	//</summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVCPUDescriputorHandole(uint32_t index);
	//<summary>
	//SRVの指定番号のGPUデスクリプタハンドルを取得
	//</summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetRTVGPUDescriputorHandole(uint32_t index);

	//<summary>
	//SRVの指定番号のCPUデスクリプタハンドルを取得
	//</summary>
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCPUDescriputorHandole(uint32_t index);
	//<summary>
	//SRVの指定番号のGPUデスクリプタハンドルを取得
	//</summary>
	D3D12_GPU_DESCRIPTOR_HANDLE GetDSVGPUDescriputorHandole(uint32_t index);


	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDesctiptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap,
		uint32_t descriptorSize, uint32_t index);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDesctiptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap,
		uint32_t descriptorSize, uint32_t index);

	ID3D12Device* GetDevice() const { return device.Get(); }//デバイスを取得する
	ID3D12GraphicsCommandList* GetCommandList()const { return commandList.Get(); }//コマンドリストを取得する

	// RTVのビュー記述子を取得する
	const D3D12_RENDER_TARGET_VIEW_DESC& GetRTVDesc() const { return rtvDesc; }
	//getdsvDescriptorHeap
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDSVDescriptorHeap() { return dsvDescriptorHeap; }
	//getviewport
	D3D12_VIEWPORT GetViewport() const { return viewport; }
	//getscissorRect
	D3D12_RECT GetScissorRect() const { return scissorRect; }

	//CompileShader関数の作成
	IDxcBlob* CompileShader(
		//ComilerするSahaderファイルへのパス
		const std::wstring& filePath,
		//compilerに使用するProfile
		const wchar_t* profile);

	//デスクリプタヒープを生成する
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heaptype,
		UINT numDescriptrs, bool shaderVisible);

	//バッファーリソースの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	//テクスチャリソースの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);

	//テクスチャデータの転送
	[[nodiscard]]
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource>texture, const DirectX::ScratchImage& mipImages);

	//バックバッファの数を取得
	size_t GetBackBufferCount()const { return swapChainResources.size(); }

	void CommandKick();

	
	void TransitionResource(ID3D12Resource* resource,D3D12_RESOURCE_STATES before,D3D12_RESOURCE_STATES after);

	
	//最大SRV数(最大テクスチャ枚数)
	static const uint32_t kMaxSRVCount;
private:

	//WindowsAPI
	WinApp* winApp_ = nullptr;
	HRESULT hr;
	//device
	Microsoft::WRL::ComPtr< IDXGIFactory7> dxgiFactory = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;
	//Command
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	//SwapChain
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2>swapChainResources;
	//DepthBuffer
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStenciResource;
	//DescriptorHeep
	uint32_t descriptorSizeSRV;
	uint32_t descriptorSizeRTV;
	uint32_t descriptorSizeDSV;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;//RTV
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;	//SRV
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;//DVS用のヒープでディスクリプタの数は1．//DSVはShader内で触るものではない


	//RTV
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStarHandle;
	//RVTを2つ作るのでディスクリプタを2つ用意
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 2> rtvHandles;
	//fence
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	HANDLE fenceEvent;
	uint64_t fenceValue = 0;
	//ビューポート
	D3D12_VIEWPORT viewport{};
	//シザー矩形
	D3D12_RECT scissorRect{};
	//DXC
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	IDxcIncludeHandler* includeHandler = nullptr;
	//barrier
	D3D12_RESOURCE_BARRIER barrier{};
	//記録時間(FPS固定用)
	std::chrono::steady_clock::time_point reference_;

	

private:


	//FPS固定初期化
	void InitializeFixFPS();
	//FPS固定更新
	void UodateFixFPS();




};

