#include "Sphere.h"
#include "MatrixMath.h"
#include "Vector3Math.h"
#include "DirectXCommon.h"
#include"TextureManager.h"
#include <numbers>

#pragma region imgui
#ifdef _DEBUG
#include "../externals/imgui/imgui.h"
#include "../externals/imgui/imgui_impl_dx12.h"
#include "../externals/imgui/imgui_impl_win32.h"
#endif 
#pragma endregion

Sphere::~Sphere() {
	mesh_.reset();
	directionalLightResource_.Reset();
	wvpResource_.Reset();
}

void Sphere::Initialize(DirectXCommon* dxCommon, Matrix4x4 cameraView, const std::string& textureFilePath) {

	//メッシュ初期化
	mesh_ = std::make_unique<Mesh>();
	//mesh_->InitializeMesh(dxCommon,textureFilePath);

	//======================= VertexResource ===========================//

	mesh_->InitializeVertexResourceSphere(dxCommon->GetDevice());

	//======================= MaterialResource ===========================//

	mesh_->GetMaterial()->GetMaterialResource();

	//======================= IndexResource ===========================//

	mesh_->InitializeIndexResourceSphere(dxCommon->GetDevice());

	//テクスチャ番号の検索と記録
	filePath_ = textureFilePath;



	//======================= transformationMatrix用のVertexResource ===========================//

	//TransformationMatrix用のVertexResource生成
	wvpResource_ = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(TransformMatrix));

	//TransformationMatrix用
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&TransformMatrixData_));

	//単位行列を書き込んでおく
	TransformMatrixData_->WVP = MatrixMath::MakeIdentity4x4();

	//======================= Transform・各行列の初期化 ===========================//

	//CPUで動かす用のTransform
	transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	//アフィン行列
	worldMatrix_ = MatrixMath::MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	//ViewProjectionの初期化
	viewMatrix_ = MatrixMath::Inverse(cameraView);
	projectionMatrix_ = MatrixMath::MakePerspectiveFovMatrix(
		0.45f, float(WinApp::kScreenWidth) / float(WinApp::kScreenHeight), 0.1f, 100.0f
	);
}

void Sphere::Update() {

	//アフィン行列の更新
	worldMatrix_ = MatrixMath::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	//wvpの更新
	worldViewProjectionMatrix_ = MatrixMath::Multiply(
		worldMatrix_, MatrixMath::Multiply(viewMatrix_, projectionMatrix_));
	TransformMatrixData_->WVP = worldViewProjectionMatrix_;
	TransformMatrixData_->World = worldMatrix_;


#ifdef _DEBUG
	ImGui::Begin("Window::Sphere");
	ImGui::DragFloat3("SphereScale", &transform_.scale.x, 0.01f);
	ImGui::DragFloat3("SphereRotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("SphereTranslate", &transform_.translate.x, 0.01f);
	ImGui::Checkbox("useMonsterBall", &useMonsterBall);
	ImGui::ColorEdit4("LightColor", &directionalLightData_->color_.x);
	ImGui::SliderFloat3("LightDirection", &directionalLightData_->direction_.x, -1.0f, 1.0f);
	ImGui::SliderFloat("LightIntensity", &directionalLightData_->intensity_, 0.0f, 1.0f);
	Vector3Math::Normalize(directionalLightData_->direction_);
	ImGui::End();


	
#endif // _DEBUG
}


void Sphere::Draw(DirectXCommon* dxCommon) {

	mesh_->SetVertexBuffers(dxCommon->GetCommandList(),0);

	// 形状を設定。PSOに設定しいるものとはまた別。同じものを設定すると考えておけばいい
	//dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//materialCBufferの場所を指定
	dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(
		0, mesh_->GetMaterial()->GetMaterialResource()->GetGPUVirtualAddress());
	//wvp用のCBufferの場所を指定
	dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	//dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU());
	//Lighting用のCBufferの場所を指定
	dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());
	//IBVの設定
	dxCommon->GetCommandList()->IASetIndexBuffer(&mesh_->GetIndexBufferView());
	// 描画！(DrawCall/ドローコール)
	dxCommon->GetCommandList()->DrawIndexedInstanced(mesh_->kSubdivision * mesh_->kSubdivision * 6, 1, 0, 0, 0);
}

