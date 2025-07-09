#pragma once
#include <vector>
#include <d3d12.h>
#include <wrl.h>
#include <span>
#include <array>
#include <map>
#include "Mymath.h"
#include "Animation/Skeleton.h"
#include "SRVManager.h"

const uint32_t kNumMaxInfluence = 5;

struct VertexInfluence {
	std::array<float, kNumMaxInfluence> weights;
	std::array<int32_t, kNumMaxInfluence> jointIndices;
};

struct WellForGPU {
	Matrix4x4 skeletonSpaceMatrix;
	Matrix4x4 skeletonSpaceInvTransposeMatrix;
};

struct SkinningInfo {
	uint32_t numVertices;
};

struct VertexWeightData {
	float weight;
	uint32_t vertexIndex;
};

struct JointWeightData {
	Matrix4x4 inverseBindPoseMatrix;
	std::vector<VertexWeightData> vertexWeights;
};

template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

struct SkinCluster {
	std::vector<Matrix4x4> inverseBindPoseMatrices;
	ComPtr<ID3D12Resource> influenceResource;
	std::span<VertexInfluence> mappedInfluences;
	ComPtr<ID3D12Resource> paletteResource;
	std::span<WellForGPU> mappedPalette;
	ComPtr<ID3D12Resource> skinningInfoResource;
	SkinningInfo* skinningInfoData;

	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> paletteSrvHandle;
	uint32_t paletteIndex;
	uint32_t influenceIndex;

	void Create(const ComPtr<ID3D12Device>& device, SrvManager* srvManager,
		Skeleton* skeleton, const ModelData* modelData);

	void Update(Skeleton* skeleton);
};