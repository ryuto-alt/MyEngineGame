#include "DirectXCommon.h"
#include <cassert>
#include <format>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#include "Logger.h"
#include "StringUtility.h"



using namespace Microsoft::WRL;
using namespace Logger;
using namespace StringUtility;


void DirectXCommon::DeviceInitialize()
{


#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		//デバッグレイヤーを有効にする
		debugController->EnableDebugLayer();
		//さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif

#pragma region DxgiFactory
	//DXGIファクターの生成

	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region アダプタ－列挙
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
		DXGI_ERROR_NOT_FOUND; i++) {

		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));

		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {

			Log(ConvertString(std::format(L"Use Adapater:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;
	}
	assert(useAdapter != nullptr);
#pragma endregion

#pragma region D3D12Device
	//D3D12Deviceの生成

	D3D_FEATURE_LEVEL featureLevels[] = {

	 D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0

	};
	const char* fetureLevelstrings[] = { "12.2","12.1","12.0" };
	for (size_t i = 0; i < _countof(featureLevels); ++i) {

		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
		if (SUCCEEDED(hr)) {

			Log(std::format("FeatureLevel:{}\n", fetureLevelstrings[i]));
			break;
		}
	}
	assert(device != nullptr);
	Log("Complete create D3D12Device!!!\n");
#pragma endregion

#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;

	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		//やばいエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラーに止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//警告の時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);


		//抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {
			//windows11でのDXGIでバックレイヤーとDX12デバックレイヤーの相互性によるエラーメッセージ
			//https:stackoverflow.com/quessions/69805245/directx-12application-is-crashing-in-windows-11
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		//抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		//指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);

	}

#endif


}

void DirectXCommon::CommandInitialize()
{
#pragma region CommandQueue,CommandAllocator,CommandList
	//コマンドキューを生成する
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	//生成がうまくできなかった
	assert(SUCCEEDED(hr));

	//コマンドアロケーターを生成する
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	//コマンドアロケーターの生成がうまく行かなった
	assert(SUCCEEDED(hr));

	//コマンドリストを生成する
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr,
		IID_PPV_ARGS(&commandList));
	//コマンドリストの生成がうまく行かなかったので起動できない
	assert(SUCCEEDED(hr));
#pragma endregion

}

void DirectXCommon::SwapChainInitialize()
{
#pragma region SwapChain
	//スワップチェーンを生成する

	swapChainDesc.Width = WinApp::kClientWindth;		//画面の幅。ウィンドウのクライアント領域を同じ物にしておく
	swapChainDesc.Height = WinApp::kClientHeight;		//画面の高さ。ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//色の形式	
	swapChainDesc.SampleDesc.Count = 1;//マルチサンプルなし
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//描画のターゲットとして利用する
	swapChainDesc.BufferCount = 2;//ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;//モニタに写したら、中身を破壊
	//コマンドキュー、ウィンドウハンドル、設定渡して生成する
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), winApp_->GetHwnd(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
	assert(SUCCEEDED(hr));

#pragma endregion 


}

void DirectXCommon::DepthBufferInitialize()
{
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = WinApp::kClientWindth;//Textureの幅
	resourceDesc.Height = WinApp::kClientHeight;//Textureの高さ
	resourceDesc.MipLevels = 1;//mipmapの数
	resourceDesc.DepthOrArraySize = 1;//奥行きor配列Texturの配列数
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//DetpthStencilとして利用可能なフォーマット
	resourceDesc.SampleDesc.Count = 1;//サンプリング。１固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//DepthSrencilとして使う通知

	//利用するhepの設定
	D3D12_HEAP_PROPERTIES heapProperties{  };
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//VRAM上に作る
	//深度値のクリア設定
	D3D12_CLEAR_VALUE depthClerValue{};
	depthClerValue.DepthStencil.Depth = 1.0f;//最大値
	depthClerValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//フォーアット。Resource合わせる
	//Resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	hr = device->CreateCommittedResource(
		&heapProperties,//Heapの設定
		D3D12_HEAP_FLAG_NONE,//Heapの特殊設定。特になし
		&resourceDesc,//REesourceの設定
		D3D12_RESOURCE_STATE_DEPTH_WRITE,//深度値を書き込む状態のしておく
		&depthClerValue,//Clear最適値
		IID_PPV_ARGS(&resource));//作成するResourceポインタへのポインタ
	assert(SUCCEEDED(hr));
	//DepthStencilTextureをウィンドウサイズで作成
	depthStenciResource = resource;

}

const uint32_t DirectXCommon::kMaxSRVCount = 512;
void DirectXCommon::DescriptorHeepInitialize()
{
	//サイズを取得
	descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	//ディスクリプタヒープの生成
	rtvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 3, false);//RTV
	srvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kMaxSRVCount, true);//SRV
	dsvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);//DVS用のヒープでディスクリプタの数は1．DSVはShader内で触るものではない

}

void DirectXCommon::RTVInitialize()
{

	//SwapChainからResourceをひっぱってくる

	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	//うまく取得できなければ起動できない
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));

#pragma region RTV
	//RTVの設定
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;//2dテクスチャとして書き込む
	//ディスクリプトの先頭を取得する
	rtvStarHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//まず1つ目を作る。1つ目は最初のところに作る。作る場所をこちらで指定してあげる必要がある
	rtvHandles[0] = rtvStarHandle;

	device->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);
	//２つ目のディスクリプタハンドルを得る（自力で）
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//２つ目を作る
	device->CreateRenderTargetView(swapChainResources[1].Get(), &rtvDesc, rtvHandles[1]);
#pragma endregion
}



void DirectXCommon::DSVInitialize()
{
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//Format
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2Dtexture
	//DSHeapの先頭にDSVを作る
	device->CreateDepthStencilView(depthStenciResource.Get(),
		&dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

}

void DirectXCommon::FenceInitialize()
{

#pragma region Fence
	//初期値0でFenceを作る

	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	//fenceのSignalを待つためのイベントを作成する
	fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);

#pragma endregion

}

void DirectXCommon::ViewportInitialize()
{

	//クライアント領域のサイズと一緒にして画面全体に表示
	viewport.Width = WinApp::kClientWindth;
	viewport.Height = WinApp::kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

}

void DirectXCommon::ScissorInitialize()
{

	//基本的にビューポートと同じ矩形が構成さるようにする
	scissorRect.left = 0;
	scissorRect.right = WinApp::kClientWindth;
	scissorRect.top = 0;
	scissorRect.bottom = WinApp::kClientHeight;

}

void DirectXCommon::DxcCompilerInitialize()
{
#pragma region DxcCompiler
	//dicCompilerを初期化
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));
	//includeに対する設定
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));
#pragma endregion
}

void DirectXCommon::ImguiInitialize()
{
	////ImGui初期化
	//IMGUI_CHECKVERSION();
	//ImGui::CreateContext();
	//ImGui::StyleColorsDark();
	//ImGui_ImplWin32_Init(winApp_->GetHwnd());
	//ImGui_ImplDX12_Init(device.Get(),
	//	swapChainDesc.BufferCount,
	//	rtvDesc.Format,
	//	srvDescriptorHeap.Get(),
	//	srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
	//	srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

}
//初期化
void DirectXCommon::Initialize(WinApp* winApp)
{
	assert(winApp);//NULL検出
	winApp_ = winApp;

	InitializeFixFPS();

	DeviceInitialize();
	CommandInitialize();
	SwapChainInitialize();
	DepthBufferInitialize();
	DescriptorHeepInitialize();
	RTVInitialize();
	
	DSVInitialize();
	FenceInitialize();
	ViewportInitialize();
	ScissorInitialize();
	DxcCompilerInitialize();
	ImguiInitialize();

}



void DirectXCommon::Begin()
{

	//これから書き込むバックバッファのインデックスを取得する
	UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();
	//今回のバリアはTransition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//Noneにしておく
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//バリアを張る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();
	//遷移前（現在）のResourceState
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	//遷移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//Transition Barrierを張る
	commandList->ResourceBarrier(1, &barrier);
	
	//描画先のRTVとDSVを設定する
	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
	
	//指定した色で画面全体をクリアする
	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };//青っぽい色。RGBAの順
	commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);



	//コマンドリストの内容を確定させる。すべてのコマンドを積んでからCliseすること
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);




}



void DirectXCommon::End()
{
	//バックバッファのインデックスを取得する
	UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();
	//画面に描く処理はすべて終わり、画面に映すので、状態遷移
		//今回はRenderTragetからPresentにする
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	//ToransitionBarrierを張る
	commandList->ResourceBarrier(1, &barrier);
	//コマンドリストの内容を確定させる。すべてのコマンドを積んでからCliseすること
	hr = commandList->Close();
	assert(SUCCEEDED(hr));
	//GPUにコマンドリストの実行を行わせる
	ID3D12CommandList* commandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(1, commandLists);
	//GPUとOSに画面の交換を行う通知する
	swapChain->Present(1, 0);
	//Fenceの値の更新
	fenceValue++;
	//GPUがここまでたどりついた時に、Fenceの値を指定したあたいに代入するようにsignalを送る
	commandQueue->Signal(fence.Get(), fenceValue);
	//Femceの値が指定したSignal値にたどり着いているか確認する
		//GetCompletebValuの初期値はFence作成時に渡した初期値
	if (fence->GetCompletedValue() < fenceValue) {

		//指定したSignalにたどりついていないので、たどり着くまで待つようにイベントを設定する
		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		//イベントを待つ
		WaitForSingleObject(fenceEvent, INFINITE);
	}
	UodateFixFPS();
	//次のフレーム用のコマンドリストを準備
	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));

}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVCPUDescriputorHandole(uint32_t index)
{
	return GetCPUDesctiptorHandle(srvDescriptorHeap, descriptorSizeSRV, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVGPUDescriputorHandole(uint32_t index)
{
	return GetGPUDesctiptorHandle(srvDescriptorHeap, descriptorSizeSRV, index);
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetRTVCPUDescriputorHandole(uint32_t index)
{
	return GetCPUDesctiptorHandle(rtvDescriptorHeap, descriptorSizeRTV, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetRTVGPUDescriputorHandole(uint32_t index)
{
	return GetGPUDesctiptorHandle(rtvDescriptorHeap, descriptorSizeRTV, index);
}



Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heaptype, UINT numDescriptrs, bool shaderVisible)
{
	//ディスクリプタヒープの生成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heaptype;//レンダーターゲットビュー用
	descriptorHeapDesc.NumDescriptors = numDescriptrs;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//ダブルバッファ用に2つ。多くても別に構わない
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	//ディスクリプトひーぷが作れなかったので起動できない
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}

void DirectXCommon::InitializeFixFPS()
{
	//現在時間を記録する
	reference_ = std::chrono::steady_clock::now();
}

void DirectXCommon::UodateFixFPS()
{
	//1/60秒ピッタリの時間
	const std::chrono::microseconds kMinTime(uint64_t(1000000.0f / 60.0f));
	//1/60秒よりわずかに短い時間
	const std::chrono::microseconds kMInCheckTime(uint64_t(1000000.0f / 65.0f));

	//現在時間を取得する
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	//前回記録からの経過時間を取得する
	std::chrono::microseconds elapsed =
		std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);

	//1/60秒(よりわずかに短い時間)経っていない場合
	if (elapsed < kMInCheckTime) {
		//1/60秒経過するまで微小なスリープを繰り返す
		while (std::chrono::steady_clock::now() - reference_ < kMinTime) {

			//1マイクロ秒スリープ
			std::this_thread::sleep_for(std::chrono::microseconds(1));

		}
	}
	//現在の時間を記録をする
	reference_ = std::chrono::steady_clock::now();
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCPUDesctiptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index)
{

	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;

}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDesctiptorHandle(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap, uint32_t descriptorSize, uint32_t index)
{

	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;

}

IDxcBlob* DirectXCommon::CompileShader(const std::wstring& filePath, const wchar_t* profile)
{


	//シェーダーをコンパイルする旨をログに出す
	Log(ConvertString(std::format(L"Begin CompileShader,path:{},profile:{}\n", filePath, profile)));
	//hlslファイルを読む
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	//読めなかったら止める
	assert(SUCCEEDED(hr));
	//読み込んだファイルの内容を設定する
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;//UTF8の文字コードであることを通知

	LPCWSTR arguments[] = {

		filePath.c_str(),//コンパイル対象のhlslファイル名
		L"-E",L"main",//エントリーpointの指定。基本的にmain以外にはしない
		L"-T",profile,//shaerProfileの設定
		L"-Zi",L"-Qembed_debug",//デバック用の情報を埋め込む
		L"-Od",//最適化を外しておく
		L"-Zpr",//メモリレイアウトは行優先
	};

	//実際にshaerをコンパイルする
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer,//読み込んだファイル
		arguments,			//コンパイルオプション
		_countof(arguments),//コンパイルオプションの数
		includeHandler,		//includeが含まれた
		IID_PPV_ARGS(&shaderResult)//コンパイル結果

	);
	//コンパイルエラーではなくDXCが起動できない致命的な状況
	assert(SUCCEEDED(hr));

	//警告・エラーが出たらログに出して止める
	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		assert(false);
	}

	//コンパイル結果から実行用のバイナリ部分を取得
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	//成功したログを出す
	Log(ConvertString(std::format(L"Complite Succeded,path:{},profile:{}\n", filePath, profile)));

	return shaderBlob;
}




Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateBufferResource(size_t sizeInBytes)
{

	//VertexResourceを作成
	//頂点リソース用ヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	//頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	//バッファーリソース。テクスチャの場合はまた別の設定をする
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeInBytes;
	//バッファの場合はこれらには1する決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	//バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//実際に頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));

	return vertexResource;

}

Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateTextureResource(const DirectX::TexMetadata& metadata)
{
	//metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resouceDesc{ };
	resouceDesc.Width = UINT(metadata.width);//Textureの幅
	resouceDesc.Height = UINT(metadata.height);//Textureの高さ
	resouceDesc.MipLevels = UINT16(metadata.mipLevels);//mipmapの数
	resouceDesc.DepthOrArraySize = UINT16(metadata.arraySize);//奥行きor配Texturaの配列数
	resouceDesc.Format = metadata.format;//Textureのフォーマット
	resouceDesc.SampleDesc.Count = 1;//サンプリクト。１固定。
	resouceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);//Textureの次元数。普段使っているのは２次元
	//利用するHeapの設定。非常に特殊な運用。
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//細かい設定を行う
	//heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//writeBackポリシーでCPUアクセス可能
	//heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//プロセッサの近くに配置
	//Resouceの作成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,//Heapの設定
		D3D12_HEAP_FLAG_NONE,//Heapの特殊設定。特になし
		&resouceDesc,//Resourceの設定
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,//Clear最適値。使わないのでnullptr
		IID_PPV_ARGS(&resource));

	assert(SUCCEEDED(hr));
	return resource;

}




Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::UploadTextureData
(Microsoft::WRL::ComPtr<ID3D12Resource> texture, const DirectX::ScratchImage& mipImages)
{
	std::vector<D3D12_SUBRESOURCE_DATA> subresouces;
	DirectX::PrepareUpload(device.Get(), mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresouces);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, UINT(subresouces.size()));

	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = CreateBufferResource(intermediateSize);
	UpdateSubresources(commandList.Get(), texture.Get(), intermediateResource.Get(), 0, 0, UINT(subresouces.size()), subresouces.data());

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);


	return intermediateResource;
}

void DirectXCommon::CommandKick()
{

	hr = commandList->Close();
	assert(SUCCEEDED(hr));
	//GPUにコマンドリストの実行を行わせる
	ID3D12CommandList* commandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(1, commandLists);
	//Fenceの値の更新
	fenceValue++;
	//GPUがここまでたどりついた時に、Fenceの値を指定したあたいに代入するようにsignalを送る
	commandQueue->Signal(fence.Get(), fenceValue);
	//Femceの値が指定したSignal値にたどり着いているか確認する
		//GetCompletebValuの初期値はFence作成時に渡した初期値
	if (fence->GetCompletedValue() < fenceValue) {

		//指定したSignalにたどりついていないので、たどり着くまで待つようにイベントを設定する
		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		//イベントを待つ
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	//次のフレーム用のコマンドリストを準備
	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));


}

void DirectXCommon::TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{




}








