#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "WinApp.h"
#include <array>
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")
#include "imgui.h"
#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"
#include "DirectXTex.h"
#include "d3dx12.h"
#include <vector>
#include <chrono>
#include <thread>
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


class DirectXCommon
{
	void DeviceInitialize();
	void CommandInitialize();
	void SwapChainInitialize();
	void DepthBufferInitialize();
	void DescriptorHeapInitialize();
	void RTVInitialize();
	void DSVInitialize();
	void FenceInitialize();
	void ViewportInitialize();
	void ScissorInitialize();
	void DxcCompilerInitialize();
	void ImguiInitialize();

public:
	// デフォルトコンストラクタ
	DirectXCommon() = default;
	
	//デストラクタ
	~DirectXCommon();
	
	// コピー/ムーブを禁止
	DirectXCommon(const DirectXCommon&) = delete;
	DirectXCommon& operator=(const DirectXCommon&) = delete;
	DirectXCommon(DirectXCommon&&) = delete;
	DirectXCommon& operator=(DirectXCommon&&) = delete;
	
	//初期化
	void Initialize(WinApp* winApp);
	//描画前処理
	void Begin();
	//描画後処理
	void End();

	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetRTVGPUDescriptorHandle(uint32_t index);
	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVCPUDescriptorHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetDSVGPUDescriptorHandle(uint32_t index);

	// RTV管理用機能（RenderTexture対応）
	uint32_t AllocateRTVIndex();
	void CreateRenderTargetViewAt(uint32_t index, Microsoft::WRL::ComPtr<ID3D12Resource> resource, DXGI_FORMAT format);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap,
		uint32_t descriptorSize, uint32_t index);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap,
		uint32_t descriptorSize, uint32_t index);

	ID3D12Device* GetDevice() const { return device.Get(); }
	ID3D12GraphicsCommandList* GetCommandList()const { return commandList.Get(); }
	IDXGISwapChain4* GetSwapChain() const { return swapChain.Get(); }
	Microsoft::WRL::ComPtr<ID3D12Resource> GetSwapChainResource(uint32_t index) const { return swapChainResources[index]; }

	IDxcBlob* CompileShader(
		//ComilerするSahaderファイルへのパス
		const std::wstring& filePath,
		//compilerに使用するProfile
		const wchar_t* profile);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);

	[[nodiscard]]
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource>texture, const DirectX::ScratchImage& mipImages);

	DirectX::ScratchImage LoadTexture(const std::string& filePath);

	void CommandKick();
	const D3D12_DEPTH_STENCIL_DESC& GetDepthStencilDesc() const {
		return depthStencilDesc;
	}

private:
	//WindowsAPI
	WinApp* winApp_ = nullptr;
	HRESULT hr = S_OK;

	Microsoft::WRL::ComPtr< IDXGIFactory7> dxgiFactory = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2>swapChainResources;

	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource;

	uint32_t descriptorSizeRTV = 0;
	uint32_t descriptorSizeDSV = 0;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStarHandle{};
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 2> rtvHandles{};

	// RTV管理用（RenderTexture対応）
	static const uint32_t kMaxRTVCount = 10; // SwapChain用2つ + RenderTexture用8つ
	uint32_t nextRTVIndex = 2; // SwapChain用のインデックス0,1の後から開始

	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	HANDLE fenceEvent = nullptr;
	uint64_t fenceValue = 0;

	D3D12_VIEWPORT viewport{};

	D3D12_RECT scissorRect{};

	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	IDxcIncludeHandler* includeHandler = nullptr;

	D3D12_RESOURCE_BARRIER barrier{};

	std::chrono::steady_clock::time_point reference_;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};

#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
#endif

private:

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>
		CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heaptype,
			UINT numDescriptrs, bool shaderVisible);

	//FPS固定初期化
	void InitializeFixFPS();
	//FPS固定更新
	void UpdateFixFPS();
};