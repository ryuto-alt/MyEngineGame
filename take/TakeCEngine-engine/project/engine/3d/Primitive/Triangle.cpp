#include "Triangle.h"

#include "MatrixMath.h"
#include "TextureManager.h"


#include <cassert>
#include <iostream>

#ifdef _DEBUG
#include "ImGuiManager.h"
#endif // DEBUG

Triangle::~Triangle() {

	wvpResource_.Reset();
	vertexBuffer_.Reset();
}

void Triangle::Initialize(DirectXCommon* dxCommon) {


	//======================= transformationMatrix用のVertexResource ===========================//
	
	//スプライト用のTransformationMatrix用のVertexResource生成
	wvpResource_ = DirectXCommon::CreateBufferResource(dxCommon->GetDevice(), sizeof(Matrix4x4));

	//TransformationMatrix用
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&TransformMatrixData_));

	//単位行列を書き込んでおく
	*TransformMatrixData_ = MatrixMath::MakeIdentity4x4();

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

}

void Triangle::Update() {

	//アフィン行列の更新
	worldMatrix_ = MatrixMath::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	//wvpの更新
	
}

#ifdef _DEBUG
void Triangle::UpdateImGui(int id) {
	//ImGuiの更新

	bool botton = false;
	std::string label = "Window::Triangle";
	label += "##" + std::to_string(id);
	ImGui::Begin(label.c_str());

	ImGui::Selectable("TextureTable", botton);
	ImGui::DragFloat3("TriangleScale", &transform_.scale.x, 0.01f);
	ImGui::DragFloat3("TriangleRotate", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("TriangleTranslate", &transform_.translate.x, 0.01f);
	ImGui::End();
}
#endif // DEBUG



void Triangle::Draw(DirectXCommon* dxCommon) {

	dxCommon->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViews_); // VBVを設定
	// 形状を設定。PSOに設定しいるものとはまた別。同じものを設定すると考えておけばいい
	dxCommon->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//materialCBufferの場所を指定
	
	//wvp用のCBufferの場所を指定
	dxCommon->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource_->GetGPUVirtualAddress());
	//SRVのDescriptorTableの先頭を設定。2はrootParameter[2]である。
	//dxCommon->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstance()->GetSrvHandleGPU(textureIndex_));
	// 描画！(DrawCall/ドローコール)。3頂点で1つのインスタンス。
	dxCommon->GetCommandList()->DrawInstanced(3, 1, 0, 0);
}


