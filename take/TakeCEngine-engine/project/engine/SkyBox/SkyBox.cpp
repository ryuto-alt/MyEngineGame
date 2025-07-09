#include "SkyBox.h"
#include "Camera.h"
#include "CameraManager.h"
#include "PipelineStateObject.h"
#include "Model.h"
#include "ModelManager.h"
#include "math/MatrixMath.h"
#include "TakeCFrameWork.h"

SkyBox::~SkyBox() {
	pso_.reset();
	rootSignature_.Reset();
	wvpResource_.Reset();
	model_ = nullptr;
	dxCommon_ = nullptr;
}

void SkyBox::Initialize(DirectXCommon* directXCommon,const std::string& filename) {

	dxCommon_ = directXCommon;

	//PSOの生成
	pso_ = std::make_unique<PSO>();
	pso_->CompileVertexShader(dxCommon_->GetDXC(), L"SkyBox.VS.hlsl");
	pso_->CompilePixelShader(dxCommon_->GetDXC(), L"SkyBox.PS.hlsl");
	pso_->CreateGraphicPSO(dxCommon_->GetDevice(), D3D12_FILL_MODE_SOLID, D3D12_DEPTH_WRITE_MASK_ALL);
	pso_->SetGraphicPipelineName("SkyBoxPSO");
	//RootSignatureの生成
	rootSignature_ = pso_->GetGraphicRootSignature();

	//モデルの生成
	model_ = ModelManager::GetInstance()->FindModel(filename);

	//TransformationMatrix用のResource生成
	wvpResource_ = DirectXCommon::CreateBufferResource(dxCommon_->GetDevice(), sizeof(TransformMatrix));
	wvpResource_->SetName(L"SkyBox::wvpResource_");
	//TransformationMatrix用
	wvpResource_->Map(0, nullptr, reinterpret_cast<void**>(&TransformMatrixData_));
	//単位行列を書き込んでおく
	TransformMatrixData_->WVP = MatrixMath::MakeIdentity4x4();

	//CPUで動かす用のTransform
	transform_ = { {50.0f,50.0f,50.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	//アフィン行列
	worldMatrix_ = MatrixMath::MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	//カメラのセット
	camera_ = CameraManager::GetInstance()->GetActiveCamera();
}

void SkyBox::Update() {

	//アフィン行列の更新
	worldMatrix_ = MatrixMath::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	//wvpの更新
	if (camera_) {
		const Matrix4x4& viewProjectionMatrix =
			CameraManager::GetInstance()->GetActiveCamera()->GetViewProjectionMatrix();
		WVPMatrix_ = MatrixMath::Multiply(worldMatrix_, viewProjectionMatrix);
	} else {
		WVPMatrix_ = worldMatrix_;
	}

	WorldInverseTransposeMatrix_ = MatrixMath::InverseTranspose(worldMatrix_);
	TransformMatrixData_->World = worldMatrix_;
	TransformMatrixData_->WVP = model_->GetModelData()->rootNode.localMatrix * WVPMatrix_;
	TransformMatrixData_->WorldInverseTranspose = WorldInverseTransposeMatrix_;
}

void SkyBox::Draw() {

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	//プリミティブトポロジー設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//ルートシグネチャ設定
	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	//PSO設定
	commandList->SetPipelineState(pso_->GetGraphicPipelineState());


	//TransformationMatrix
	commandList->SetGraphicsRootConstantBufferView(0, wvpResource_->GetGPUVirtualAddress());

	if (model_ != nullptr) {
		model_->DrawSkyBox();
	}
}
