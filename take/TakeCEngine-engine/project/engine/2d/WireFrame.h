#pragma once
#include "Vector3.h"
#include "Matrix4x4.h"
#include "TransformMatrix.h"
#include "Transform.h"
#include "ResourceDataStructure.h"
#include "Mesh/Mesh.h"
#include "PipelineStateObject.h"
#include "DirectXCommon.h"
#include "math/AABB.h"
#include "math/OBB.h"
#include <stdint.h>
#include <string>
#include <d3d12.h>
#include <wrl.h>

struct WireFrameVertexData {
	Vector3 position;
	Vector4 color;
};

struct WireFrameTransFormMatrixData {
	Matrix4x4 WVP;
};

struct LineData {
	ComPtr<ID3D12Resource> vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViews_;
	WireFrameVertexData* vertexData_ = nullptr;
};

class WireFrame {
public:

	void Initialize(DirectXCommon* directXCommon);

	void Update();

	void DrawLine(const Vector3& start, const Vector3& end,const Vector4& color);

	void DrawOBB(const OBB& obb, const Vector4& color);

	void DrawSphere(const Vector3& center, float radius, const Vector4& color);

	void DrawGridGround(const Vector3& center, const Vector3& size,uint32_t division);

	void DrawGridBox(const AABB& aabb, uint32_t division);

	void Draw();

	void Reset();

	void Finalize();

private:

	void CreateVertexData();

	void CalculateSphereVertexData();

	void DrawGridLines(const Vector3& start, const Vector3& end, const Vector3& offset, uint32_t division, const Vector4& color);

private:

	DirectXCommon* dxCommon_ = nullptr;

	std::unique_ptr<PSO> pso_ = nullptr;
	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
	
	LineData* lineData_ = nullptr;

	std::vector<Vector3> spheres_;

	//TransformationMatrix用の頂点リソース
	ComPtr<ID3D12Resource> wvpResource_;
	//TransformationMatrix用の頂点データ
	WireFrameTransFormMatrixData* TransformMatrixData_ = nullptr;

	//instancingResource
	ComPtr<ID3D12Resource> instancingResource_;
	
	//CPU用のTransform
	EulerTransform transform_{};
	//行列
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 worldViewProjectionMatrix_;

	uint32_t lineIndex_ = 0;
	const uint32_t kMaxLineCount_ = 1000000;
	const uint32_t kLineVertexCount_ = 2;

};