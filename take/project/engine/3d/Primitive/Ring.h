#pragma once
#include "DirectXCommon.h"
#include "PipelineStateObject.h"
#include "ResourceDataStructure.h"
#include "TransformMatrix.h"
#include "Transform.h"
#include "Mesh/Mesh.h"

#include <memory>

class Ring {
public:
	Ring() = default;
	~Ring() = default;
	void Initialize(DirectXCommon* dxCommon);
	void Update();
	void Draw();
	//void CreateRing(const uint32_t divide, const float outerRadius, const float innerRadius, const Vector3& center, const Vector4& color);

private:

	DirectXCommon* dxCommon_ = nullptr;
	std::unique_ptr<PSO> pso_;
	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
	EulerTransform transform_;

	//頂点リソース
	std::unique_ptr<Mesh> mesh_ = nullptr;
	//wvpResource
	ComPtr<ID3D12Resource> wvpResource_ = nullptr;
	TransformMatrix* TransformMatrixData_ = nullptr;

	//TransformMatrix
	Matrix4x4 worldMatrix_;
	Matrix4x4 WVPMatrix_;
	Matrix4x4 WorldInverseTransposeMatrix_;
};

