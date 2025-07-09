#include "Ring.h"
#include "MatrixMath.h"
#include "CameraManager.h"

void Ring::Initialize(DirectXCommon* dxCommon) {

	dxCommon_ = dxCommon;
	//PSOの生成
	pso_ = std::make_unique<PSO>();
	pso_->CompileVertexShader(dxCommon_->GetDXC(), L"Object3d.VS.hlsl");
	pso_->CompilePixelShader(dxCommon_->GetDXC(), L"Object3d.PS.hlsl");
	//PSOの生成
	pso_->CreateGraphicPSO(
		dxCommon_->GetDevice(),
		D3D12_FILL_MODE_SOLID,
		D3D12_DEPTH_WRITE_MASK_ALL);

	//Meshの生成
	mesh_ = std::make_unique<Mesh>();
	mesh_->InitializeMesh(dxCommon_,"uvChecker.png");


	rootSignature_ = pso_->GetGraphicRootSignature();
	wvpResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformMatrix));
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&TransformMatrixData_));

	//CPUで動かす用のTransform
	transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	worldMatrix_ = MatrixMath::MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);
	WVPMatrix_ = MatrixMath::MakeIdentity4x4();
	WorldInverseTransposeMatrix_ = MatrixMath::MakeIdentity4x4();
}

void Ring::Update() {

	//アフィン行列の更新
	worldMatrix_ = MatrixMath::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	//wvpの更新
	WVPMatrix_ = MatrixMath::Multiply(worldMatrix_, CameraManager::GetInstance()->GetActiveCamera()->GetViewProjectionMatrix());
	WorldInverseTransposeMatrix_ = MatrixMath::InverseTranspose(worldMatrix_);
	TransformMatrixData_->World = worldMatrix_;
	TransformMatrixData_->WVP = WVPMatrix_;
	TransformMatrixData_->WorldInverseTranspose = WorldInverseTransposeMatrix_;
}

void Ring::Draw() {}
