#pragma once
#include "base/DirectXCommon.h"
#include "base/PipelineStateObject.h"
#include "base/SrvManager.h"
#include "3d/Material.h"
#include "TransformMatrix.h"
#include "ResourceDataStructure.h"
#include "Primitive/PrimitiveType.h"
#include <memory>
#include <cstdint>
#include <unordered_map>

struct PrimitiveMesh {
	ComPtr<ID3D12Resource> vertexBuffer_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_ = {};
	ComPtr<ID3D12Resource> indexBuffer_ = nullptr;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_ = {};
};

class PrimitiveDrawer {
public:
	PrimitiveDrawer() = default;
	~PrimitiveDrawer() = default;
	
public:

	// Ring全体のデータ
	struct RingData {
		PrimitiveMesh primitiveData_;
		VertexData* vertexData_ = nullptr;
		Material* material_ = nullptr;
		float outerRadius_ = 1.0f; // 外側の半径
		float innerRadius_ = 0.01f; // 内側の半径
	};

	//plane全体のデータ
	struct PlaneData {
		PrimitiveMesh primitiveData_;
		VertexData* vertexData_ = nullptr;
		Material* material_ = nullptr;
		float width_;
		float height_;
	};

	//sphere全体のデータ
	struct SphereData {
		PrimitiveMesh primitiveData_;
		VertexData* vertexData_ = nullptr;
		Material* material_ = nullptr;
		float radius_;
	};

public:
	// 初期化
	void Initialize(DirectXCommon* dxCommon,SrvManager* srvManager);

	/// 終了処理
	void Finalize();

	//更新処理
	void Update();

	void UpdateImGui(uint32_t handle,PrimitiveType type);
	void UpdateImGui(uint32_t handle, PrimitiveType type, const Vector3& param);

	//リングデータの生成
	uint32_t GenerateRing(const float outerRadius, const float innerRadius, const std::string& textureFilePath);
	// 平面データの生成
	uint32_t GeneratePlane(const float width, const float height, const std::string& textureFilePath);
	// 球データの生成
	uint32_t GenerateSphere(const float radius, const std::string& textureFilePath);

	// 描画処理
	void DrawParticle(PSO* pso,UINT instanceCount,PrimitiveType type,uint32_t handle);

	//void CreateVertexData(PrimitiveType type);

	// リングの頂点データの作成関数
	void CreateRingVertexData(RingData* ringData);

	void CreatePlaneVertexData(PlaneData* planeData);

	void CreateSphereVertexData(SphereData* sphereData);

	void CreateRingMaterial(const std::string& textureFilePath,RingData* ringData);

	void CreatePlaneMaterial(const std::string& textureFilePath,PlaneData* planeData);

	void CreateSphereMaterial(const std::string& textureFilePath, SphereData* sphereData);

	void ResetVertexIndex() { ringVertexIndex_ = 0; }

	PlaneData* GetPlaneData(uint32_t handle);
	SphereData* GetSphereData(uint32_t handle);
	RingData* GetRingData(uint32_t handle);

private:

	DirectXCommon* dxCommon_ = nullptr;
	SrvManager* srvManager_ = nullptr;

	TransformMatrix* TransformMatrixData_ = nullptr;
	EulerTransform transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	std::unordered_map<uint32_t,std::unique_ptr<RingData>> ringDatas_;
	uint32_t ringDivide_ = 32;
	uint32_t ringVertexIndex_ = 0;
	uint32_t ringVertexCount_ = 0;
	uint32_t ringHandle_ = 0;

	std::unordered_map<uint32_t,std::unique_ptr<PlaneData>> planeDatas_;
	uint32_t planeVertexIndex_ = 0;
	uint32_t planeVertexCount_ = 0;
	uint32_t planeHandle_ = 0;

	std::unordered_map<uint32_t, std::unique_ptr<SphereData>> sphereDatas_;
	uint32_t sphereDivide_ = 16; // 球の分割数
	uint32_t sphereVertexIndex_ = 0;
	uint32_t sphereVertexCount_ = 0;
	uint32_t sphereIndexCount_ = 0;
	uint32_t sphereHandle_ = 0;

	const uint32_t kMaxVertexCount_ = 32000;
};

