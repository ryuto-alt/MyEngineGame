#pragma once
#include "ResourceDataStructure.h"
#include "Transform.h"
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <cstdint>

struct MaterialData {
	Vector4 color; //カラー
	Matrix4x4 uvTransform; //UVトランスフォーム
	uint32_t enableLighting; //ライティングを有効にするフラグ
	float shininess; //鏡面反射の強さ
	float envCoefficient; //環境光の強さ
};

class DirectXCommon;
class Material {
public:

	Material() = default;
	~Material() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, const std::string& filePath, const std::string& envMapfilePath);

	void Update();

	/// <summary>
	/// 更新処理
	/// </summary>
	void UpdateMaterialImGui();

	/// <summary>
	/// マテリアルリソース初期化
	/// </summary>
	void InitializeMaterialResource(Microsoft::WRL::ComPtr<ID3D12Device> device);

public: //ゲッター

	/// <summary>
	/// マテリアルリソースの取得
	/// </summary>
	ID3D12Resource* GetMaterialResource() const { return materialResource_.Get(); }

	MaterialData* GetMaterialData() const { return materialData_; }

	const std::string& GetTextureFilePath() const { return textureFilePath_; }

	const std::string& GetEnvMapFilePath() const { return envMapFilePath_; }

public: //セッター

	/// <summary>
	///uvTransformのセッター
	/// </summary>
	void SetUvTransform(const EulerTransform& uvTransform) { uvTransform_ = uvTransform; }

	void SetUvScale(Vector3 scale) { uvTransform_.scale = scale; }

	void SetUvRotate(Vector3 rotate) { uvTransform_.rotate = rotate; }

	void SetUvTranslate(Vector3 translate) { uvTransform_.translate = translate; }

	/// <summary>
	/// マテリアルデータのセッター
	/// </summary>
	void SetMaterialData(MaterialData* materialData) { materialData_ = materialData; }

	void SetEnableLighting(bool enable) { materialData_->enableLighting = enable; }

	void SetMaterialColor(Vector4 color) { materialData_->color = color; }

	void SetShininess(float shininess) { materialData_->shininess = shininess; }

	void SetEnvCoefficient(float envCoefficient) { materialData_->envCoefficient = envCoefficient; }

	void SetTextureFilePath(const std::string& filePath) { textureFilePath_ = filePath; }

	void SetEnvMapFilePath(const std::string& filePath) { envMapFilePath_ = filePath; }

private:


	//マテリアルリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	MaterialData* materialData_ = nullptr;

	//uvTransform
	EulerTransform uvTransform_;

	std::string textureFilePath_;
	std::string envMapFilePath_;

};