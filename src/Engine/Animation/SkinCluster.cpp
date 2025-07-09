#include "SkinCluster.h"
#include "DirectXCommon.h"
#include "d3dx12.h"
#include <algorithm>
#include <cassert>

void SkinCluster::Create(
	const ComPtr<ID3D12Device>& device, SrvManager* srvManager,
	Skeleton* skeleton, const ModelData* modelData) {

	// DirectXCommonは使用せず、直接デバイスから作成
	D3D12_HEAP_PROPERTIES heapProps{};
	heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
	D3D12_RESOURCE_DESC bufferDesc{};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = sizeof(WellForGPU) * skeleton->GetJoints().size();
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&paletteResource));
	paletteResource->SetName(L"SkinCluster::paletteResource");

	paletteIndex = srvManager->Allocate();
	paletteSrvHandle.first = srvManager->GetCPUDescriptorHandle(paletteIndex);
	paletteSrvHandle.second = srvManager->GetGPUDescriptorHandle(paletteIndex);

	srvManager->CreateSRVForStructuredBuffer(
		paletteIndex, paletteResource, UINT(skeleton->GetJoints().size()), sizeof(WellForGPU));

	WellForGPU* mappedPaletteData = nullptr;
	paletteResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedPaletteData));
	mappedPalette = std::span<WellForGPU>(mappedPaletteData, skeleton->GetJoints().size());

	// influenceResource作成
	D3D12_HEAP_PROPERTIES heapProps2{};
	heapProps2.Type = D3D12_HEAP_TYPE_UPLOAD;
	D3D12_RESOURCE_DESC bufferDesc2{};
	bufferDesc2.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc2.Width = sizeof(VertexInfluence) * modelData->vertices.size();
	bufferDesc2.Height = 1;
	bufferDesc2.DepthOrArraySize = 1;
	bufferDesc2.MipLevels = 1;
	bufferDesc2.SampleDesc.Count = 1;
	bufferDesc2.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	device->CreateCommittedResource(
		&heapProps2,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc2,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&influenceResource));
	influenceResource->SetName(L"SkinCluster::influenceResource");

	influenceIndex = srvManager->Allocate();
	srvManager->CreateSRVForStructuredBuffer(
		influenceIndex, influenceResource, UINT(modelData->vertices.size()), sizeof(VertexInfluence));

	VertexInfluence* mappedInfluenceData = nullptr;
	influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluenceData));
	std::memset(mappedInfluenceData, 0, sizeof(VertexInfluence) * modelData->vertices.size());
	mappedInfluences = std::span<VertexInfluence>(mappedInfluenceData, modelData->vertices.size());

	// skinningInfoResource作成
	D3D12_HEAP_PROPERTIES heapProps3{};
	heapProps3.Type = D3D12_HEAP_TYPE_UPLOAD;
	D3D12_RESOURCE_DESC bufferDesc3{};
	bufferDesc3.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc3.Width = sizeof(SkinningInfo);
	bufferDesc3.Height = 1;
	bufferDesc3.DepthOrArraySize = 1;
	bufferDesc3.MipLevels = 1;
	bufferDesc3.SampleDesc.Count = 1;
	bufferDesc3.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	device->CreateCommittedResource(
		&heapProps3,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc3,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&skinningInfoResource));
	skinningInfoResource->Map(0, nullptr, reinterpret_cast<void**>(&skinningInfoData));
	skinningInfoData->numVertices = uint32_t(modelData->vertices.size());

	inverseBindPoseMatrices.resize(skeleton->GetJoints().size());
	std::generate(inverseBindPoseMatrices.begin(), inverseBindPoseMatrices.end(), []() {
		return MakeIdentity4x4();
	});
}

void SkinCluster::Update(Skeleton* skeleton) {
	for (size_t jointIndex = 0; jointIndex < skeleton->GetJoints().size(); ++jointIndex) {
		assert(jointIndex < inverseBindPoseMatrices.size());

		mappedPalette[jointIndex].skeletonSpaceMatrix =
			Multiply(inverseBindPoseMatrices[jointIndex], skeleton->GetJoints()[jointIndex].skeletonSpaceMatrix);
		mappedPalette[jointIndex].skeletonSpaceInvTransposeMatrix =
			Inverse(mappedPalette[jointIndex].skeletonSpaceMatrix);
	}
}

void SkinCluster::UpdateVertices(Skeleton* skeleton, const ModelData* originalModelData, VertexData* outputVertices) {
	// 簡易実装：ルートボーンの変換を全頂点に適用
	if (skeleton->GetJoints().empty()) {
		// スケルトンがない場合は元データをコピー
		for (size_t i = 0; i < originalModelData->vertices.size(); ++i) {
			outputVertices[i] = originalModelData->vertices[i];
		}
		return;
	}
	
	const Joint& rootJoint = skeleton->GetJoints()[skeleton->GetRoot()];
	Matrix4x4 transform = rootJoint.skeletonSpaceMatrix;
	
	for (size_t i = 0; i < originalModelData->vertices.size(); ++i) {
		const VertexData& original = originalModelData->vertices[i];
		
		// 位置を変換
		Vector4 pos = { original.position.x, original.position.y, original.position.z, 1.0f };
		Vector4 transformedPos = {
			transform.m[0][0] * pos.x + transform.m[0][1] * pos.y + transform.m[0][2] * pos.z + transform.m[0][3] * pos.w,
			transform.m[1][0] * pos.x + transform.m[1][1] * pos.y + transform.m[1][2] * pos.z + transform.m[1][3] * pos.w,
			transform.m[2][0] * pos.x + transform.m[2][1] * pos.y + transform.m[2][2] * pos.z + transform.m[2][3] * pos.w,
			transform.m[3][0] * pos.x + transform.m[3][1] * pos.y + transform.m[3][2] * pos.z + transform.m[3][3] * pos.w
		};
		
		// 法線を変換
		Vector3 normal = { original.normal.x, original.normal.y, original.normal.z };
		Vector3 transformedNormal = {
			transform.m[0][0] * normal.x + transform.m[0][1] * normal.y + transform.m[0][2] * normal.z,
			transform.m[1][0] * normal.x + transform.m[1][1] * normal.y + transform.m[1][2] * normal.z,
			transform.m[2][0] * normal.x + transform.m[2][1] * normal.y + transform.m[2][2] * normal.z
		};
		
		// 出力頂点に設定
		outputVertices[i] = original;
		outputVertices[i].position = { transformedPos.x, transformedPos.y, transformedPos.z, 1.0f };
		outputVertices[i].normal = transformedNormal;
	}
}