#include "PipelineStateObject.h"
#include "Utility/Logger.h"
#include "Utility/StringUtility.h"
#include "DirectXCommon.h"
#include "DirectXShaderCompiler.h"
#include "ImGuiManager.h"

#include <cassert>
#include <list>

PSO::~PSO() {

	graphicRootSignature_.Reset();
	if (errorBlob_) {
		errorBlob_.Reset();
	}
	signatureBlob_.Reset();
	graphicPipelineState_.Reset();
}

void PSO::CompileVertexShader(DXC* dxc_, const std::wstring& filePath) {

	//頂点シェーダーのコンパイル
	graphicShaderData_.vertexBlob = dxc_->CompileShader(
		filePath, L"vs_6_6", dxc_->GetDxcUtils().Get(), dxc_->GetDxcCompiler().Get(), dxc_->GetIncludeHandler().Get()
	);
}

void PSO::CompilePixelShader(DXC* dxc_, const std::wstring& filePath) {

	//ピクセルシェーダーのコンパイル
	graphicShaderData_.pixelBlob = dxc_->CompileShader(
		filePath, L"ps_6_6", dxc_->GetDxcUtils().Get(), dxc_->GetDxcCompiler().Get(), dxc_->GetIncludeHandler().Get()
	);

	//
}

void PSO::CompileComputeShader(DXC* dxc_, const std::wstring& filePath) {
	//コンピュートシェーダーのコンパイル
	computeShaderBlob_ = dxc_->CompileShader(
		filePath, L"cs_6_6", dxc_->GetDxcUtils().Get(), dxc_->GetDxcCompiler().Get(), dxc_->GetIncludeHandler().Get()
	);
}

//=============================================================================
// 入力レイアウトの情報を取得する関数
//=============================================================================

void PSO::ExtractInputLayout(ID3D12ShaderReflection* shaderReflection) {
	D3D12_SHADER_DESC shaderDesc;
	shaderReflection->GetDesc(&shaderDesc);

	//InputLayoutが存在しない場合
	if(shaderDesc.InputParameters == 0) {
		inputElementDescs_.clear();
		semanticName_.clear();
		inputLayoutDesc_.pInputElementDescs = nullptr;
		inputLayoutDesc_.NumElements = 0;
		return;
	}

	inputElementDescs_.resize(shaderDesc.InputParameters);
	semanticName_.resize(shaderDesc.InputParameters);
	for (UINT i = 0; i < shaderDesc.InputParameters; ++i) {
		D3D12_SIGNATURE_PARAMETER_DESC inputParamDesc;
		shaderReflection->GetInputParameterDesc(i, &inputParamDesc);

		semanticName_[i] = inputParamDesc.SemanticName;

		inputElementDescs_[i].SemanticName = semanticName_[i].c_str();
		inputElementDescs_[i].SemanticIndex = inputParamDesc.SemanticIndex;
		inputElementDescs_[i].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

		if (inputParamDesc.Mask == 1) {
			inputElementDescs_[i].Format = DXGI_FORMAT_R32_FLOAT;
		} else if (inputParamDesc.Mask <= 3) {
			inputElementDescs_[i].Format = DXGI_FORMAT_R32G32_FLOAT;
		} else if (inputParamDesc.Mask <= 7) {
			inputElementDescs_[i].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		} else {
			inputElementDescs_[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}
	}
	inputLayoutDesc_.pInputElementDescs = inputElementDescs_.data();
	inputLayoutDesc_.NumElements = static_cast<UINT>(inputElementDescs_.size());
}

//=============================================================================
// シェーダーを読み込んでリソース情報を取得する関数
//=============================================================================

ShaderResourceMap PSO::LoadShaderResourceInfo(
	const std::vector<ComPtr<IDxcBlob>>& shaderBlobs) {
	ShaderResourceMap bindResources;
	// シェーダーの可視性を取得する関数
	auto GetShaderVisibility = [&](D3D12_SHADER_DESC shaderDesc) {
		UINT shaderVersion = D3D12_SHVER_GET_TYPE(shaderDesc.Version);
		if (shaderVersion == D3D12_SHVER_VERTEX_SHADER) {
			return D3D12_SHADER_VISIBILITY_VERTEX;
		} else if (shaderVersion == D3D12_SHVER_PIXEL_SHADER) {
			return D3D12_SHADER_VISIBILITY_PIXEL;
		} else if (shaderVersion == D3D12_SHVER_COMPUTE_SHADER) {
			return D3D12_SHADER_VISIBILITY_ALL;
		} else {
			return D3D12_SHADER_VISIBILITY_ALL;
		}
		};
	// 各シェーダーをリフレクションしてリソースを収集
	for (size_t shaderIndex = 0; shaderIndex < shaderBlobs.size(); ++shaderIndex) {
		const auto& shaderBlob = shaderBlobs[shaderIndex];
		ComPtr<IDxcContainerReflection> containerReflection;
		HRESULT hr = DxcCreateInstance(CLSID_DxcContainerReflection, IID_PPV_ARGS(&containerReflection));
		if (FAILED(hr)) {
			//"Failed to create IDxcContainerReflection."
			assert(false);
		}
		hr = containerReflection->Load(shaderBlob.Get());
		if (FAILED(hr)) {
			//"Failed to load shader blob into reflection."
			assert(false);
		}
		UINT32 shaderIdx = 0;
		hr = containerReflection->FindFirstPartKind(DXC_PART_DXIL, &shaderIdx);
		if (FAILED(hr)) {
			//"Failed to find DXIL part in shader blob."
			assert(false);
		}
		ComPtr<ID3D12ShaderReflection> shaderReflection;
		hr = containerReflection->GetPartReflection(shaderIdx, IID_PPV_ARGS(&shaderReflection));
		if (FAILED(hr)) {
			//"Failed to get shader reflection."
			assert(false);
		}
		D3D12_SHADER_DESC shaderDesc;
		shaderReflection->GetDesc(&shaderDesc);
		D3D12_SHADER_VISIBILITY visibility = GetShaderVisibility(shaderDesc);
		// シェーダーのバインドリソースを収集
		for (UINT i = 0; i < shaderDesc.BoundResources; ++i) {
			D3D12_SHADER_INPUT_BIND_DESC bindDesc;
			shaderReflection->GetResourceBindingDesc(i, &bindDesc);
			ShaderResourceKey key = { bindDesc.Type,visibility, bindDesc.BindPoint, bindDesc.Space };
			if (bindResources.find(key) == bindResources.end()) {
				//サンプラーの場合はvisibilityのみ設定
				if (bindDesc.Type == D3D_SIT_SAMPLER) {
					if (visibility == D3D12_SHADER_VISIBILITY_ALL) {
						key.visibility = D3D12_SHADER_VISIBILITY_ALL;

					} else if(visibility == D3D12_SHADER_VISIBILITY_PIXEL){
						key.visibility = D3D12_SHADER_VISIBILITY_PIXEL;
						continue;
					}
					continue;
				}
				bindResources[key] = { key, bindDesc.Name };
			}
		}
		// 入力レイアウト情報を収集（頂点シェーダーのみ
		if (visibility == D3D12_SHADER_VISIBILITY_VERTEX) {
			ExtractInputLayout(shaderReflection.Get());
		}
	}
	return { bindResources };
}

//=============================================================================
// シェーダー情報からRootSignatureの生成
//=============================================================================

ComPtr<ID3D12RootSignature> PSO::CreateRootSignature(ID3D12Device* device, ShaderResourceMap resourceMap) {

	// ルートシグネチャ記述を構築
	D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
	rootSigDesc.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	// ルートシグネチャのパラメータを構築
	std::vector<D3D12_ROOT_PARAMETER> rootParameters;
	std::list<D3D12_DESCRIPTOR_RANGE> descriptorRanges;
	for (const auto& resource : resourceMap) {
		const ShaderResourceKey& key = resource.second.key;
		if (key.type == D3D_SIT_CBUFFER) {
			// 定数バッファ
			D3D12_ROOT_PARAMETER rootParam = {};
			rootParam.ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
			rootParam.ShaderVisibility          = key.visibility;
			rootParam.Descriptor.ShaderRegister = key.bindPoint;
			rootParameters.push_back(rootParam);

		} else if (key.type == D3D_SIT_TEXTURE || key.type == D3D_SIT_STRUCTURED) {
			// テクスチャやストラクチャードバッファ
			D3D12_DESCRIPTOR_RANGE range = {};
			range.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			range.NumDescriptors                    = 1;
			range.BaseShaderRegister                = key.bindPoint;
			range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			descriptorRanges.push_back(range);

			D3D12_ROOT_PARAMETER rootParam = {};
			rootParam.ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParam.ShaderVisibility                    = key.visibility;
			rootParam.DescriptorTable.pDescriptorRanges   = &descriptorRanges.back();
			rootParam.DescriptorTable.NumDescriptorRanges = 1;
			rootParameters.push_back(rootParam);

		} else if (key.type == D3D_SIT_UAV_RWSTRUCTURED || key.type == D3D_SIT_UAV_RWTYPED) {
			// RWStructuredBuffer
			D3D12_DESCRIPTOR_RANGE range = {};
			range.RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			range.NumDescriptors                    = 1;
			range.BaseShaderRegister                = key.bindPoint;
			range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			descriptorRanges.push_back(range);

			D3D12_ROOT_PARAMETER rootParam = {};
			rootParam.ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParam.ShaderVisibility                    = key.visibility;
			rootParam.DescriptorTable.pDescriptorRanges   = &descriptorRanges.back();
			rootParam.DescriptorTable.NumDescriptorRanges = 1;
			rootParameters.push_back(rootParam);
		}
	}
	// ルートパラメータの数を設定
	rootSigDesc.pParameters   = rootParameters.data();
	rootSigDesc.NumParameters = static_cast<UINT>(rootParameters.size());

	//Samplerの設定
	staticSamplers_[0].Filter           = D3D12_FILTER_MIN_MAG_MIP_LINEAR; //バイナリフィルタ
	staticSamplers_[0].AddressU         = D3D12_TEXTURE_ADDRESS_MODE_WRAP; //0~1の範囲外をリピート
	staticSamplers_[0].AddressV         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers_[0].AddressW         = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers_[0].ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER; //比較しない
	staticSamplers_[0].MaxLOD           = D3D12_FLOAT32_MAX; //ありったけのMipmapを使う
	staticSamplers_[0].ShaderRegister   = 0; //レジスタ番号0を使う


	rootSigDesc.pStaticSamplers   = staticSamplers_;
	rootSigDesc.NumStaticSamplers = _countof(staticSamplers_);

	// ルートシグネチャをシリアライズして作成
	signatureBlob_ = nullptr;
	errorBlob_     = nullptr;

	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signatureBlob_, &errorBlob_);
	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
		assert(false);
	}
	ComPtr<ID3D12RootSignature> rootSignature;
	hr = device->CreateRootSignature(0, signatureBlob_->GetBufferPointer(),
		signatureBlob_->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	if (FAILED(hr)) {
		Logger::Log(StringUtility::ConvertString(L"Failed to create root signature."));
		assert(false);
	}
	Logger::Log(StringUtility::ConvertString(L"Root signature successfully created."));
	return rootSignature;
}

//=============================================================================
// BlendStateの生成
//=============================================================================

void PSO::CreateBlendStateForObject3d() {
	blendDesc_.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc_.RenderTarget[0].BlendEnable           = true;
	blendDesc_.RenderTarget[0].SrcBlend              = D3D12_BLEND_ONE;
	blendDesc_.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
	blendDesc_.RenderTarget[0].DestBlend             = D3D12_BLEND_ZERO;
	blendDesc_.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
	blendDesc_.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
	blendDesc_.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ZERO;
}

void PSO::CreateBlendStateForParticle() {
	blendDesc_.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc_.RenderTarget[0].BlendEnable           = true;
	blendDesc_.RenderTarget[0].SrcBlend              = D3D12_BLEND_SRC_ALPHA;
	blendDesc_.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
	blendDesc_.RenderTarget[0].DestBlend             = D3D12_BLEND_ONE;
	blendDesc_.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
	blendDesc_.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
	blendDesc_.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_INV_SRC_ALPHA;
}

void PSO::CreateBlendStateForSprite() {
	blendDesc_.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc_.RenderTarget[0].BlendEnable           = true;
	blendDesc_.RenderTarget[0].SrcBlend              = D3D12_BLEND_SRC_ALPHA;
	blendDesc_.RenderTarget[0].DestBlend             = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc_.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
	blendDesc_.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
	blendDesc_.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_ZERO;
	blendDesc_.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
}

void PSO::InitializeBlendState(BlendState blendState) {
	switch (blendState) {
	case PSO::BlendState::NORMAL:
		CreateBlendStateForObject3d();
		break;
	case PSO::BlendState::ADD:
		CreateBlendStateForParticle();
		break;
	case PSO::BlendState::SUBTRACT:
		CreateBlendStateForParticle();
		break;
	case PSO::BlendState::MULTIPLY:
		CreateBlendStateForParticle();
		break;
	case PSO::BlendState::SCREEN:
		CreateBlendStateForParticle();
		break;
	case PSO::BlendState::SPRITE:
		CreateBlendStateForSprite();
		break;
	default:
		break;
	}
}

//=============================================================================
// RasterizerStateの生成
//=============================================================================

void PSO::CreateRasterizerState(D3D12_FILL_MODE fillMode) {
	rasterizerDesc_.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc_.FillMode = fillMode;
}

//=============================================================================
// graphicPSOの生成
//=============================================================================
void PSO::CreateGraphicPSO(
	ID3D12Device* device,
	D3D12_FILL_MODE fillMode,
	D3D12_DEPTH_WRITE_MASK depthWriteMask,
	BlendState blendState,
	D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType) {

	device_ = device;

	HRESULT result = S_FALSE;
	itemCurrentIdx = 0;
	//シェーダー情報を読み込む
	graphicBindResourceInfo_ = LoadShaderResourceInfo({ graphicShaderData_.vertexBlob,graphicShaderData_.pixelBlob });
	/// ルートシグネチャ初期化
	graphicRootSignature_ = CreateRootSignature(device, graphicBindResourceInfo_);
	/// ブレンドステート初期化
	InitializeBlendState(blendState);
	/// ラスタライザステート初期化
	CreateRasterizerState(fillMode);
#pragma region SetDepthStencilDesc
	depthStencilDesc_.DepthEnable = true;                           //Depthの機能を有効化
	depthStencilDesc_.DepthWriteMask = depthWriteMask;              //Depthの書き込みマスク
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; //比較関数はLessEqual。近ければ描画される
#pragma endregion
	//GraphicPipelineStateDescの設定
	SetGraphicPipelineStateDesc(topologyType);
	//実際に生成
	graphicPipelineState_ = nullptr;
	result = device_->CreateGraphicsPipelineState(&graphicsPipelineStateDesc_,
		IID_PPV_ARGS(&graphicPipelineState_));
	assert(SUCCEEDED(result));
}
//=============================================================================
// ComputePSOの生成
//=============================================================================

void PSO::CreateComputePSO(ID3D12Device* device) {
	HRESULT result = S_FALSE;
	//シェーダー情報を読み込む
	computeBindResourceInfo_ = LoadShaderResourceInfo({ computeShaderBlob_ });
	/// ルートシグネチャ初期化
	computeRootSignature_ = CreateRootSignature(device, computeBindResourceInfo_);
	/// ComputePipelineStateDescの設定
	SetComputePipelineStateDesc();
	//実際に生成
	computePipelineState_ = nullptr;
	result = device->CreateComputePipelineState(&computePipelineStateDesc_,
		IID_PPV_ARGS(&computePipelineState_));
	assert(SUCCEEDED(result));
}
//=============================================================================
// graphicPSOの生成(RenderTexture用)
//=============================================================================
void PSO::CreateRenderTexturePSO(ID3D12Device* device) {

	HRESULT result = S_FALSE;

	device_ = device;

	graphicBindResourceInfo_ = LoadShaderResourceInfo({ graphicShaderData_.vertexBlob,graphicShaderData_.pixelBlob });
	graphicRootSignature_ = CreateRootSignature(device, graphicBindResourceInfo_);

	InitializeBlendState(BlendState::NORMAL);
	CreateRasterizerState(D3D12_FILL_MODE_SOLID);

	//DepthStencilの設定
	//MEMO:全画面に処理をするのでDepthが不要になる
	depthStencilDesc_.DepthEnable = false;

	//GraphicPipelineStateDescの設定
	SetGraphicPipelineStateDesc(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	//実際に生成
	graphicPipelineState_ = nullptr;
	result = device_->CreateGraphicsPipelineState(&graphicsPipelineStateDesc_,
		IID_PPV_ARGS(&graphicPipelineState_));
	assert(SUCCEEDED(result));
}

void PSO::UpdateImGui() {
	ImGui::Text("BlendState");
	UpdateImGuiCombo();
}

bool PSO::UpdateImGuiCombo() {

	//コンボボックスの項目
	std::vector<std::string> items = { "Normal", "Add", "Subtract","Multiply","Screen" };
	//現在の項目
	std::string& currentItem = items[itemCurrentIdx];
	//変更があったかどうか
	bool changed = false;

	//コンボボックスの表示
	if (ImGui::BeginCombo("Lighting", currentItem.c_str())) {
		for (int n = 0; n < items.size(); n++) {
			const bool is_selected = (currentItem == items[n]);
			if (ImGui::Selectable(items[n].c_str(), is_selected)) {
				currentItem = items[itemCurrentIdx];
				itemCurrentIdx = n;
				changed = true;
			}
			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}

		currentItem = items[itemCurrentIdx];
		ImGui::EndCombo();
	}

	//変更があった場合
	if (changed) {

		HRESULT result = S_FALSE;

		switch (itemCurrentIdx) {
		case 0: // Normal
			blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
			blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
			break;
		case 1: // Add
			blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
			break;

		case 2: // Subtract
			blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_SUBTRACT;
			blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
			break;

		case 3: // Multiply
			blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
			blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
			break;

		case 4: // Screen
			blendDesc_.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
			blendDesc_.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc_.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
			break;

		}

		//PSOの再生成をする
		graphicsPipelineStateDesc_.BlendState = blendDesc_; // blendState
		result = device_->CreateGraphicsPipelineState(&graphicsPipelineStateDesc_,
			IID_PPV_ARGS(&graphicPipelineState_));
		assert(SUCCEEDED(result));
	}

	return changed;
}

int32_t PSO::GetGraphicBindResourceIndex(const std::string& name) {

	int index = 0;

	for (const auto& resource : graphicBindResourceInfo_) {
		if (resource.second.name == name) {
			return index;
		}
		index++;
	}

	//見つからなかった場合
	assert(false && L"Failed to find bind resource index for name");
	return -1;
}

int32_t PSO::GetComputeBindResourceIndex(const std::string& name) {

	int index = 0;

	for (const auto& resource : computeBindResourceInfo_) {
		if (resource.second.name == name) {
			return index;
		}
		index++;
	}

	//見つからなかった場合
	assert(false && L"Failed to find bind resource index for name");
	return -1;
}

void PSO::SetGraphicPipelineStateDesc(D3D12_PRIMITIVE_TOPOLOGY_TYPE type) {

	//DepthStencilの設定
	graphicsPipelineStateDesc_.DepthStencilState = depthStencilDesc_;
	graphicsPipelineStateDesc_.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// RootSignature
	graphicsPipelineStateDesc_.pRootSignature = graphicRootSignature_.Get();

	// InputLayout
	graphicsPipelineStateDesc_.InputLayout = inputLayoutDesc_;

	// VertexShader
	graphicsPipelineStateDesc_.VS = { graphicShaderData_.vertexBlob->GetBufferPointer(),
	 graphicShaderData_.vertexBlob->GetBufferSize() };

	// PixelShader
	graphicsPipelineStateDesc_.PS = { graphicShaderData_.pixelBlob->GetBufferPointer(),
	graphicShaderData_.pixelBlob->GetBufferSize() };

	// blendState
	graphicsPipelineStateDesc_.BlendState = blendDesc_;

	// rasterizerState
	graphicsPipelineStateDesc_.RasterizerState = rasterizerDesc_;

	//書き込むRTVの情報
	graphicsPipelineStateDesc_.NumRenderTargets = 1;
	graphicsPipelineStateDesc_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//利用するトロポジ(形状)のタイプ。三角形
	graphicsPipelineStateDesc_.PrimitiveTopologyType = type;

	//どのように画面に色を打ち込むのかの設定
	graphicsPipelineStateDesc_.SampleDesc.Count = 1;
	graphicsPipelineStateDesc_.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
}

void PSO::SetComputePipelineStateDesc() {
	computePipelineStateDesc_.CS = {
		computeShaderBlob_->GetBufferPointer(),
		computeShaderBlob_->GetBufferSize()
	};

	computePipelineStateDesc_.pRootSignature = computeRootSignature_.Get();

}

//パイプラインの名前を設定する
void PSO::SetGraphicPipelineName(const std::string& name) {
	graphicPipelineState_->SetName(StringUtility::ConvertString(name).c_str());
}

void PSO::SetComputePipelineName(const std::string& name) {
	computePipelineState_->SetName(StringUtility::ConvertString(name).c_str());
}
