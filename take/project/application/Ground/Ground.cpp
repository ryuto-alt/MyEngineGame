#define NOMINMAX
#include "Ground.h"
#include "Input.h"
#include "MatrixMath.h"
#include "Object3dCommon.h"
#include "Vector3Math.h"
#include "Model.h"
#include "Sprite.h"
#include "TextureManager.h"
#include "WinApp.h"
#include "TakeCFrameWork.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <numbers>
#ifdef _DEBUG
#include "ImGuiManager.h"
#endif // DEBUG

Ground::~Ground() {
	object3dCommon_ = nullptr;
	model_ = nullptr;
}

void Ground::Initialize(Object3dCommon* object3dCommon, const std::string& filePath) {


	Object3d::Initialize(object3dCommon, filePath);

	transform_.translate = { 0.0f, -1.0f, 0.0f };
	//transform_.scale = { 0.1f, .0f, 100.0f };
	model_->GetMesh()->GetMaterial()->SetEnableLighting(false);
	model_->GetMesh()->GetMaterial()->SetEnvCoefficient(0.0f);
}

void Ground::Update() {

	//
	Object3d::Update();

	ImGuiDebug();
}

void Ground::Draw() {

	// モデル描画
	Object3d::Draw();
}

void Ground::DrawGridGround() {
	TakeCFrameWork::GetWireFrame()->DrawGridGround(transform_.translate, { gridGroundSize_, 0.0f, gridGroundSize_ }, 100);
}

void Ground::ImGuiDebug() {
#ifdef _DEBUG

	ImGui::Begin("Ground");

	ImGui::DragFloat3("translate", &transform_.translate.x, 0.01f);

	ImGui::ColorEdit4("color", &model_->GetMesh()->GetMaterial()->GetMaterialData()->color.x);

	ImGui::End();

#endif // DEBUG
}