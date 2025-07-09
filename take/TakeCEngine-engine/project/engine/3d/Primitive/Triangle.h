#pragma once
#include "DirectXCommon.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include "Material.h"
#include "ResourceDataStructure.h"

#include <string>
#include <vector>
#include <memory>


class DirectXCommon;
class Texture;
class Triangle {
public:

	Triangle() = default;
	~Triangle();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

#ifdef _DEBUG
	void UpdateImGui(int id);
#endif // _DEBUG


	/// <summary>
	/// 頂点バッファビューの取得
	/// </summary>
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() {
		return vertexBufferViews_;
	}

	/// <summary>
	/// MaterialData初期化
	/// </summary>
	void Draw(DirectXCommon* dxCommon);



	Microsoft::WRL::ComPtr<ID3D12Resource> GetVertexResource() {
		return vertexBuffer_;
	}

	/// <summary>
	/// 頂点リソースの取得
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> GetWvpResource() {
		return wvpResource_;
	}

	void SetVertexData(VertexData* vertexData) { vertexData_ = vertexData; }

private:



	//マテリアル
	std::unique_ptr<Material> material_ = nullptr;

	//頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
	//頂点バッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViews_{};
	//TransformationMatrix用の頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;

	//CPU用のTransform
	EulerTransform transform_{};
	//行列
	Matrix4x4 worldMatrix_;
	Matrix4x4 WVPMatrix_;

	//頂点データ
	VertexData* vertexData_ = nullptr;
	//TransformationMatrix用の頂点データ
	Matrix4x4* TransformMatrixData_ = nullptr;

};

