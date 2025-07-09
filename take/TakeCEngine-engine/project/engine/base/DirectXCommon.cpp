#include "DirectXCommon.h"

#include "Utility/Logger.h"
#include "Utility/StringUtility.h"
#include "Utility/ResourceBarrier.h"
#include "ImGuiManager.h"
#include <format>
#include <cassert>
#include <thread>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "winmm.lib")

DirectXCommon::~DirectXCommon() {
	
}

//==============================================================================================
//		初期化
//==============================================================================================

void DirectXCommon::Initialize(WinApp* winApp) {

	//FPS固定の初期化
	InitializeFixFPS();
	//システムタイマーの分解能を上げる
	timeBeginPeriod(1);

	//winapp初期化
	winApp_ = winApp;

	//DXGIデバイス初期化
	InitializeDXGIDevice();
	// コマンド関連初期化
	InitializeCommand();
	// スワップチェーンの生成
	CreateSwapChain();
	//深度ステンシルテクスチャの生成
	CreateDepthStencilTextureResource(device_, WinApp::kScreenWidth, WinApp::kScreenHeight);
	//DSV生成
	CreateDSV();

	//rtvManager初期化
	rtvManager_ = std::make_unique<RtvManager>();
	rtvManager_->Initialize(this);

	// RTVの初期化
	InitializeRenderTargetView();
	// DSVの初期化
	InitializeDepthStencilView();
	// フェンス生成
	CreateFence();
	//Viewport初期化
	InitViewport();
	//Scissor矩形初期化
	InitScissorRect();

	dxc_ = std::make_unique<DXC>();
	dxc_->InitializeDxc();
}

//==============================================================================================
//		終了処理
//==============================================================================================

void DirectXCommon::Finalize() {

	CloseHandle(fenceEvent_);
	fenceEvent_ = nullptr;

	fence_.Reset();

	commandList_.Reset();
	commandAllocator_.Reset();
	commandQueue_.Reset();
	dsvHeap_.Reset();
	rtvManager_->Finalize();
	swapChainResources_[0].Reset();
	swapChainResources_[1].Reset();
	swapChain_.Reset();


	device_.Reset();
	useAdapter_.Reset();
	dxgiFactory_.Reset();

	dxc_.reset();
	winApp_ = nullptr;
}

//==============================================================================================
//		描画前処理
//==============================================================================================

void DirectXCommon::ClearRenderTarget() {

	//書き込むバックバッファのインデックスを取得
	UINT bbIndex = swapChain_->GetCurrentBackBufferIndex();

	//バリア設定
	SetBarrier(D3D12_RESOURCE_STATE_PRESENT,D3D12_RESOURCE_STATE_RENDER_TARGET,swapChainResources_[bbIndex].Get());

	//CPUハンドルの取得
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = rtvManager_->GetRtvDescriptorHandleCPU(bbIndex);

	//描画先のRTVを設定する
	commandList_->OMSetRenderTargets(1, &handleCPU, false, nullptr);
	// 全画面クリア
	commandList_->ClearRenderTargetView(handleCPU, clearColor_, 0, nullptr);
	// Viewportを設定
	commandList_->RSSetViewports(1, &viewport_);
	// Scissorの設定
	commandList_->RSSetScissorRects(1, &scissorRect_);
}

void DirectXCommon::PreDraw() {

	//全画面クリア
	ClearRenderTarget();
}

//==============================================================================================
//		描画後処理
//==============================================================================================

void DirectXCommon::PostDraw() {

	HRESULT result = S_FALSE;

	//書き込むバックバッファのインデックスを取得
	UINT bbIndex = swapChain_->GetCurrentBackBufferIndex();

	//画面に描く処理はすべて終わり、画面に移すので、状態を遷移
	//RenderTargetからPresentにする
	SetBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET,D3D12_RESOURCE_STATE_PRESENT,swapChainResources_[bbIndex].Get());

	// コマンドリストの内容を確定
	result = commandList_->Close();
	assert(SUCCEEDED(result));

	// コマンドリストの実行
	ComPtr<ID3D12CommandList> cmdLists[] = { commandList_.Get() }; // コマンドリストの配列
	commandQueue_->ExecuteCommandLists(1, cmdLists->GetAddressOf());

	//GPUとOSに画面の交換を行うよう通知
	swapChain_->Present(1, 0);

	//Fenceの値の更新
	fenceVal_++;
	//GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る
	commandQueue_->Signal(fence_.Get(), fenceVal_);

	//Fenceの値が指定したSignal値にたどり着いているか確認する
	if (fence_->GetCompletedValue() < fenceVal_) {
		//指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
		fence_->SetEventOnCompletion(fenceVal_, fenceEvent_);
		//イベントを待つ
		WaitForSingleObject(fenceEvent_, INFINITE);
	}

	//FPS固定の更新
	UpdateFixFPS();

	//次のフレーム用のコマンドリストを準備
	//コマンドアロケータのリセット
	result = commandAllocator_->Reset();
	assert(SUCCEEDED(result));
	//コマンドリストのリセット
	result = commandList_->Reset(commandAllocator_.Get(), nullptr);
	assert(SUCCEEDED(result));
}



void DirectXCommon::InitializeDXGIDevice() {

	//機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0
	};

	const char* featureLevelStrings[] = {
		"12.2",
		"12.1",
		"12.0"
	};

	//////////////////////////////////////////////////////////
	//DXGIファクトリーの作成
	//////////////////////////////////////////////////////////


	//HRESULTはWindows系のエラーコードであり、
	//関数が成功したかどうかSUCCEEDEDマクロで判定できる
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));

	//初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、
	//どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr));

	//使用するアダプタ(GPU)の決定
	//使用するアダプタ用の変数。最初にnullptrを入れておく
	useAdapter_ = nullptr;

	//良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(
		i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
		IID_PPV_ARGS(&useAdapter_)) != DXGI_ERROR_NOT_FOUND; ++i) {
		//アダプタの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter_->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr)); //取得できないのは一大事



		//ソフトウェアアダプタでなければ採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			//採用したアダプタの情報をログに出力。wstringの方なので注意
			Logger::Log(StringUtility::ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter_ = nullptr; //ソフトウェアアダプタの場合は見なかったことにする
	}

	//適切なアダプタが見つからなかった場合は起動できない
	assert(useAdapter_ != nullptr);

	//D3D12Deviceの生成
	//高い順に生成できるか試していく
	for (size_t i = 0; i < _countof(featureLevels); ++i) {

		//採用したアダプタでデバイスを生成
		hr = D3D12CreateDevice(useAdapter_.Get(), featureLevels[i], IID_PPV_ARGS(&device_));

		//指定した機能レベルでデバイスが生成出来たかを確認
		if (SUCCEEDED(hr)) {
			//生成出来た場合はログ出力を行ってループを抜ける
			Logger::Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}

	//デバイスの生成がうまくいかなかった場合は起動できない
	assert(device_ != nullptr);

	//初期化完了のログを出す
	Logger::Log("Complete create D3D12Device!!!\n");

#ifdef _DEBUG
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		//ヤバいエラーの時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラーの時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//警告時に止まる
		//infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		//抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {
			// Windows11でのDXGIデッバグレイヤーとDX12デッバグレイヤーの相互作用バグによるエラーメッセージ
			// https://stackoverflow.com/questions/69805245/directx-12-application-is-crashing-in-windows-11
		D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		//抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		// 指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);

		//解放
		infoQueue->Release();
	}
#endif // _DEBUG

}

//==============================================================================================
//		コマンド初期化	
//==============================================================================================

void DirectXCommon::InitializeCommand() {
	HRESULT result = S_FALSE;

	// コマンドキューを生成
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc{};
	result = device_->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&commandQueue_));
	// 生成がうまくいかなかった場合は起動できない
	assert(SUCCEEDED(result));

	// コマンドアロケータを生成
	result = device_->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
	// 生成がうまくいかなかった場合は起動できない
	assert(SUCCEEDED(result));

	// コマンドリストを生成
	result = device_->CreateCommandList(
		0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr,
		IID_PPV_ARGS(&commandList_));
	// 生成がうまくいかなかった場合は起動できない
	assert(SUCCEEDED(result));
}

//==============================================================================================
//		SwapChain生成
//==============================================================================================

void DirectXCommon::CreateSwapChain() {

	HRESULT result = S_FALSE;

	// 各種設定をしてスワップチェーンを生成
	swapChainDesc_.Width = winApp_->kWindowWidth;
	swapChainDesc_.Height = winApp_->kWindowHeight;
	swapChainDesc_.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // 色情報の書式
	swapChainDesc_.SampleDesc.Count = 1;                 // マルチサンプルしない
	swapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // バックバッファとして使えるように
	swapChainDesc_.BufferCount = 2;                      // バッファ数を２つに設定(ダブルバッファ)
	swapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // モニタに映したら(フリップ後)は速やかに破棄

	result = dxgiFactory_->CreateSwapChainForHwnd(
		commandQueue_.Get(), winApp_->GetHwnd(), &swapChainDesc_, nullptr, nullptr,
		reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));
	assert(SUCCEEDED(result));
}

//==============================================================================================
//		深度バッファ生成
//==============================================================================================

void DirectXCommon::CreateDepthStencilTextureResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, int32_t width, int32_t height) {

	//生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width; //Texureの幅
	resourceDesc.Height = height; //Textureの高さ
	resourceDesc.MipLevels = 1; //mipmapの数
	resourceDesc.DepthOrArraySize = 1; //奥行き or 配列Textureの配列数
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; //DepthStencilとして利用可能なフォーマット
	resourceDesc.SampleDesc.Count = 1; //サンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D; //2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL; //DepthStencilとして使う通知

	//利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; //VRAM上に作る

	//深度値のクリア設定
	D3D12_CLEAR_VALUE depthCLearValue{};
	depthCLearValue.DepthStencil.Depth = 1.0f; //1.0f(最大値)でクリア
	depthCLearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; //フォーマット。Resoureと合わせる

	//Resoureの生成
	HRESULT hr;
	hr = device->CreateCommittedResource(
		&heapProperties, //Heapの設定
		D3D12_HEAP_FLAG_NONE, //Heapの特殊な設定
		&resourceDesc, //Resourceの設定
		D3D12_RESOURCE_STATE_DEPTH_WRITE, //深度値を書き込む状態にしておく
		&depthCLearValue, //Clear最適値。
		IID_PPV_ARGS(&depthStencilResource_) //作成するResourceポインタへのポインタ
	);
	assert(SUCCEEDED(hr));
}

//==============================================================================================
// 各デスクリプタヒープ生成
//==============================================================================================

void DirectXCommon::CreateDSV() {

	//ディスクリプタヒープのサイズを取得
	descriptorSizeDSV_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	//DSV用のディスクリプタヒープ生成。DSVはShader内で触るものではないので、ShaderVisibleはfalse
	dsvHeap_ = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

}

//==============================================================================================
//		RTVの初期化
//==============================================================================================

void DirectXCommon::InitializeRenderTargetView() {

	HRESULT result = S_FALSE;

	//SwapChainからResourceを引っ張ってくる
	for (int i = 0; i < 2; i++) {
		swapChainResources_[i] = { nullptr };
	}

	result = swapChain_->GetBuffer(0, IID_PPV_ARGS(&swapChainResources_[0]));
	//取得できなければ起動できない
	assert(SUCCEEDED(result));
	result = swapChain_->GetBuffer(1, IID_PPV_ARGS(&swapChainResources_[1]));
	assert(SUCCEEDED(result));


	for (int i = 0; i < 2; ++i) {
		// 作る場所をこちらで指定してあげる必要がある
		swapchainRtvIndex_[i] = rtvManager_->Allocate();
		rtvManager_->CreateRTV(swapChainResources_[i].Get(), swapchainRtvIndex_[i]);
	}
}

//==============================================================================================
//		DSV初期化
//==============================================================================================

void DirectXCommon::InitializeDepthStencilView() {

	//DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; //format.基本的にはResourceに合わせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D; //2dTexture
	//DSVHeapの先頭にDSVを作る
	device_->CreateDepthStencilView(depthStencilResource_.Get(), &dsvDesc, dsvHeap_->GetCPUDescriptorHandleForHeapStart());
}

//==============================================================================================
//		Fence生成
//==============================================================================================

void DirectXCommon::CreateFence() {
	HRESULT result = S_FALSE;
	// フェンスの生成
	result = device_->CreateFence(fenceVal_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(result));

	//FenceのSignalを待つためのイベントを作成する
	fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent_ != nullptr);
}

//==============================================================================================
//		FPS固定初期化
//==============================================================================================

void DirectXCommon::InitializeFixFPS() {

	//現在時間を記録する
	reference_ = std::chrono::steady_clock::now();

}

//==============================================================================================
//		FPS固定更新
//==============================================================================================

void DirectXCommon::UpdateFixFPS() {

	// 1/60秒ぴったりの時間
	const std::chrono::microseconds kMinTime(uint64_t(1000000.0f / 60.0f));

	//1/60秒よりわずかに短い時間
	const std::chrono::microseconds kMinCheckTime(uint64_t(1000000.0f / 65.0f));

	//現在時間を取得
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	//前回記録からの経過時間を取得する
	std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);

	//1/60秒(よりわずかに短い時間) 経っていない場合
	if (elapsed < kMinCheckTime) {

		// 1/60秒経過するまで微小スリープを繰り返す
		while (std::chrono::steady_clock::now() - reference_ < kMinTime) {
			//1マイクロ秒スリープ
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}

	//現在の時間を記録する
	reference_ = std::chrono::steady_clock::now();
}

void DirectXCommon::SetBarrier(D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState, ID3D12Resource* resource) {
	D3D12_RESOURCE_BARRIER barrier{};
	//今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//バリアを張る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = resource;
	//遷移前(現在)のResourceState
	barrier.Transition.StateBefore = beforeState;
	//遷移後のResourceState
	barrier.Transition.StateAfter = afterState;
	//TransitionBarrierを張る
	commandList_->ResourceBarrier(1, &barrier);
}

//==================================================================================
//		デスクリプタヒープ生成
//==================================================================================

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap(
	D3D12_DESCRIPTOR_HEAP_TYPE heapType,
	UINT numDescriptors, bool shaderVisible) {

	ComPtr<ID3D12DescriptorHeap> descriptorHeap_ = nullptr;

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT hr;
	hr = device_->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap_));
	assert(SUCCEEDED(hr));
	return descriptorHeap_;
}

//===================================================================================
// RenderTexture生成
//===================================================================================

ComPtr<ID3D12Resource> DirectXCommon::CreateRenderTextureResource(ComPtr<ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format, const Vector4& clearColor) {
	
	ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT result = S_FALSE;

	//頂点リソースの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width; // リソースのサイズ
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; //renderTargetとして使う

	//ヒーププロパティ
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = format;
	clearValue.Color[0] = clearColor.x;
	clearValue.Color[1] = clearColor.y;
	clearValue.Color[2] = clearColor.z;
	clearValue.Color[3] = clearColor.w;

	//実際に頂点リソースを作る
	result = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue,IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(result));

	return resource;
}

ComPtr<ID3D12Resource> DirectXCommon::CreateTextureResource(ComPtr<ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format) {
	ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT result = S_FALSE;

	//頂点リソースの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width; // リソースのサイズ
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE; //renderTargetとして使わない

	//ヒーププロパティ
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	//実際に頂点リソースを作る
	result = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(result));

	return resource;
}

ComPtr<ID3D12Resource> DirectXCommon::CreateTextureResourceUAV(ComPtr<ID3D12Device> device, uint32_t width, uint32_t height, DXGI_FORMAT format) {
	
	ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT result = S_FALSE;

	//頂点リソースの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width; // リソースのサイズ
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS; //UAVとして使う

	//ヒーププロパティ
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	//実際に頂点リソースを作る
	result = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr,IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(result));

	D3D12_RESOURCE_BARRIER uavBarrier = {};
	
	// TransitionBarrierを張る
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	uavBarrier.Transition.pResource = resource.Get();
	// COMMON >> NON_PIXEL_SHADER_RESOURCE
	uavBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	uavBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	commandList_->ResourceBarrier(1, &uavBarrier);

	return resource;
}

//==============================================================================================
///		CPUディスクリプタハンドルの取得
//==============================================================================================

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {

	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

//==============================================================================================
//		GPUディスクリプタハンドルの取得
//==============================================================================================

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index) {

	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

//==============================================================================================
//		BufferResource生成
//==============================================================================================

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {

	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT result = S_FALSE;

	//ヒーププロパティ
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // UploadHeapを使う

	//頂点リソースの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes; // リソースのサイズ
	//バッファの場合はこれらは1にする決まり
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれにする決まり
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//実際に頂点リソースを作る
	result = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(result));

	return resource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateBufferResourceUAV(ID3D12Device* device, size_t sizeInBytes, ID3D12GraphicsCommandList* commandList) {

	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT result = S_FALSE;

	//ヒーププロパティ
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // defaultを使う

	//頂点リソースの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes; // リソースのサイズ
	//バッファの場合はこれらは1にする決まり
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれにする決まり
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	//UAVを利用するかどうか
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	//実際に頂点リソースを作る
	result = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(result));

	D3D12_RESOURCE_BARRIER uavBarrier = {};

	//今回のバリアはTransition
	uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//Noneにしておく
	uavBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//バリアを張る対象のリソース。現在のバックバッファに対して行う
	uavBarrier.Transition.pResource = resource.Get();
	//遷移前(現在)のResourceState
	uavBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
	//遷移後のResourceState
	uavBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	//TransitionBarrierを張る
	commandList->ResourceBarrier(1, &uavBarrier);

	return resource;
}

void DirectXCommon::InitViewport() {
	viewport_ = winApp_->GetViewport();
}

void DirectXCommon::InitScissorRect() {
	scissorRect_ = winApp_->GetScissorRect();
}