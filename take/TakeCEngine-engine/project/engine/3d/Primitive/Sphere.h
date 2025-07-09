#pragma once
#include "Vector3.h"
#include "Matrix4x4.h"
#include "TransformMatrix.h"
#include "Transform.h"
#include "ResourceDataStructure.h"
#include "Mesh/Mesh.h"
#include <stdint.h>
#include <string>
#include <d3d12.h>
#include <wrl.h>

class DirectXCommon;
class Sphere {
public:
	Sphere() = default;
	~Sphere();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, Matrix4x4 cameraView, const std::string& textureFilePath);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(DirectXCommon* dxCommon);

public: //ゲッター

public: //セッター


private:

	//メッシュ
	std::unique_ptr<Mesh> mesh_;

	//filePath
	std::string filePath_;


	//TransformationMatrix用の頂点リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	//TransformationMatrix用の頂点データ
	TransformMatrix* TransformMatrixData_ = nullptr;

	//平行光源用のリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
	DirectionalLightData* directionalLightData_ = nullptr;;

	//CPU用のTransform
	EulerTransform transform_{};
	//行列
	Matrix4x4 worldMatrix_;
	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 worldViewProjectionMatrix_;

	//Texture
	bool useMonsterBall = true;

};

