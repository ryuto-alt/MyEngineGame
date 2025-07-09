#include "Material.h"
#include "DirectXCommon.h"
#include "MatrixMath.h"
#include "TextureManager.h"
#include "ImGuiManager.h"

#include <algorithm>

void Material::Initialize(DirectXCommon* dxCommon, const std::string& filePath, const std::string& envMapfilePath) {

	//マテリアルリソース初期化
	InitializeMaterialResource(dxCommon->GetDevice());

	textureFilePath_ = filePath;
	envMapFilePath_ = envMapfilePath;
	//テクスチャ初期化
	TextureManager::GetInstance()->LoadTexture(filePath,false);
	if(envMapfilePath != ""){
		TextureManager::GetInstance()->LoadTexture(envMapfilePath,false);
	}


	//uvTransform
	uvTransform_ = {
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};
}

void Material::Update() {

	Matrix4x4 scaleMatrix = MatrixMath::MakeScaleMatrix(uvTransform_.scale);
	Matrix4x4 rotateMatrix = MatrixMath::MakeRotateZMatrix(uvTransform_.rotate.z);
	Matrix4x4 translateMatrix = MatrixMath::MakeTranslateMatrix(uvTransform_.translate);
	materialData_->uvTransform = MatrixMath::Multiply(MatrixMath::Multiply(scaleMatrix, rotateMatrix), translateMatrix);
}

void Material::UpdateMaterialImGui() {
#ifdef _DEBUG
	//ImGuiの更新
	if (ImGui::TreeNode("Material")) {
		ImGui::ColorEdit4("Color", &materialData_->color.x);
		ImGui::Text("uvTransform");
		ImGui::DragFloat2("UVTranslate", &uvTransform_.translate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat2("UVScale", &uvTransform_.scale.x, 0.01f, -10.0f, 10.0f);
		ImGui::SliderAngle("UVRotate", &uvTransform_.rotate.z);
		ImGui::DragFloat("Shininess", &materialData_->shininess, 0.1f, 0.1f, 100.0f);
		ImGui::DragFloat("EnvCoefficient", &materialData_->envCoefficient, 0.001f, 0.0f, 1.0f);

		bool enableLighting = materialData_->enableLighting;
		ImGui::Checkbox("EnableLighting", &enableLighting);
		materialData_->enableLighting = enableLighting;

		Matrix4x4 uvTransformMatrix = MatrixMath::MakeScaleMatrix(uvTransform_.scale);
		uvTransformMatrix = MatrixMath::Multiply(uvTransformMatrix, MatrixMath::MakeRotateZMatrix(uvTransform_.rotate.z));
		uvTransformMatrix = MatrixMath::Multiply(uvTransformMatrix, MatrixMath::MakeTranslateMatrix(uvTransform_.translate));
		materialData_->uvTransform = uvTransformMatrix;
		ImGui::TreePop();
	}
#endif // DEBUG
}

void Material::InitializeMaterialResource(Microsoft::WRL::ComPtr<ID3D12Device> device) {

	//マテリアル用リソース作成
	materialResource_ = DirectXCommon::CreateBufferResource(device.Get(), sizeof(MaterialData));
	materialResource_->SetName(L"Material::materialResource_");
	//materialにデータを書き込む
	materialData_ = nullptr;
	//書き込むためのアドレスを取得
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));

	//色を書き込む
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData_->uvTransform = MatrixMath::MakeIdentity4x4();
	materialData_->enableLighting = false;
	materialData_->shininess = 60.0f;
	materialData_->envCoefficient = 0.5f;
}