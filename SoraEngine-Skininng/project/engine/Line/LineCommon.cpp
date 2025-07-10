#include "LineCommon.h"
#include "MyMath.h"
#include <CameraManager.h>

LineCommon* LineCommon::instance_ = nullptr;
LineCommon* LineCommon::GetInstance()
{
	if (instance_ == nullptr) {
		instance_ = new LineCommon();
	}
	return instance_;
}

void LineCommon::Initialize(DirectXCommon* dxCommon, SrvManager* srvManager)
{
	dxCommon_ = dxCommon;
	srvManager_ = srvManager;
	//パイプラインの生成
	graphicsPipeline_ = std::make_unique<GraphicsPipeline>();
	graphicsPipeline_->Initialize(dxCommon_);
	graphicsPipeline_->CreateLine();





	vertexResource_ = dxCommon_->CreateBufferResource(sizeof(VertexDataLine) * linevertices.size());
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexDataLine) * linevertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexDataLine);
	void* mapped = nullptr;
	vertexResource_->Map(0, nullptr, &mapped);
	memcpy(mapped, linevertices.data(), sizeof(VertexDataLine) * linevertices.size());

	//カメラ
	cameraResource = dxCommon_->CreateBufferResource(sizeof(CameraBufferforGpu));
	cameraResource->Map(0, nullptr, reinterpret_cast<void**>(&camerabuffer));

	instanceSrvIndex_ = UINT32_MAX;
}
void LineCommon::Finalize()
{
	delete instance_;
	instance_ = nullptr;
}
void LineCommon::CommonDraw()
{
	dxCommon_->GetCommandList()->SetGraphicsRootSignature(graphicsPipeline_->GetRootSignatureLine());
	dxCommon_->GetCommandList()->SetPipelineState(graphicsPipeline_->GetGraphicsPipelineStateLine());
	// 1本ずつ独立した線なので LINESTRIP ではなく LINELIST
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

}

void LineCommon::Update()
{

	 
	camerabuffer->projection = CameraManager::GetInstance()->GetActiveCamera()->GetProjextionMatrix();
	camerabuffer->view = CameraManager::GetInstance()->GetActiveCamera()->GetViewMatrix();



	if (instances_.empty()) return;
	size_t instanceSize = sizeof(LineInstanceData) * instances_.size();
	if (!instanceResource_ || instanceResource_->GetDesc().Width < instanceSize) {
		// リソース作り直し（大きさ足りない場合）
		D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(instanceSize);
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		dxCommon_->GetDevice()->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&instanceResource_));
	}


	// マップしてコピー
	LineInstanceData* mapped = nullptr;
	instanceResource_->Map(0, nullptr, reinterpret_cast<void**>(&mapped));
	memcpy(mapped, instances_.data(), instanceSize);
	instanceResource_->Unmap(0, nullptr);

	// SRVインデックスは初回だけ確保（使い回し）
	if (instanceSrvIndex_ == UINT32_MAX) {
		instanceSrvIndex_ = srvManager_->Allocate();
	}

	// StructuredBuffer 用の SRV を SrvManager 経由で作成
	srvManager_->CreateSRVforStructuredBuffer(
		instanceSrvIndex_,
		instanceResource_.Get(),
		static_cast<UINT>(instances_.size()),
		sizeof(LineInstanceData));

}

void LineCommon::Draw()
{
	if (instances_.empty()) return;

	CommonDraw();
	dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);
	// RootParameter[0] → b0：カメラ（CBV）
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, cameraResource->GetGPUVirtualAddress());
	srvManager_->SetGraficsRootDescriptorTable(1, instanceSrvIndex_);
	dxCommon_->GetCommandList()->DrawInstanced(2, static_cast<UINT>(instances_.size()), 0, 0);

	instances_.clear(); // ← 正しい変数名




}

void LineCommon::DrawLine(const Vector3& start, const Vector3& end, const Vector4& color)
{
	instances_.push_back({ start, end, color });


}
