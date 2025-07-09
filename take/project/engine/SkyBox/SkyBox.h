#pragma once
#include "DirectXCommon.h"
#include "ResourceDataStructure.h"
#include "TransformMatrix.h"
#include "Model.h"

class Camera;
class PSO;
class SkyBox {
public:

	//エイリアステンプレート
	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	SkyBox() = default;
	~SkyBox();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* directXCommon,const std::string& filename);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	void SetMaterialColor(const Vector4& color) { model_->GetMesh()->GetMaterial()->SetMaterialColor(color); }

private: // privateメンバ変数

	//DirectXCommon
	DirectXCommon* dxCommon_ = nullptr;
	//PSO
	std::unique_ptr<PSO> pso_ = nullptr;
	//RootSignature
	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	//モデル
	Model* model_ = nullptr;

	//TransformationMatrix用の頂点リソース
	ComPtr<ID3D12Resource> wvpResource_;
	//TransformationMatrix用の頂点データ
	TransformMatrix* TransformMatrixData_ = nullptr;

	//Transform
	EulerTransform transform_{};
	//TransformMatrix
	Matrix4x4 worldMatrix_;
	Matrix4x4 WVPMatrix_;
	Matrix4x4 WorldInverseTransposeMatrix_;
	//Camera
	Camera* camera_ = nullptr;
};

