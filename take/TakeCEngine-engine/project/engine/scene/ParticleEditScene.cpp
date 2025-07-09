#include "ParticleEditScene.h"
#include "camera/CameraManager.h"
#include "base/TakeCFrameWork.h"

void ParticleEditScene::Initialize() {

	// カメラの初期化
	previewCamera_ = std::make_shared<Camera>();
	previewCamera_->Initialize(CameraManager::GetInstance()->GetDirectXCommon()->GetDevice());
	previewCamera_->SetIsDebug(true);
	previewCamera_->SetTranslate({ 5.0f,0.0f,-10.0f });
	previewCamera_->SetRotate({ 0.0f,-1.4f,0.0f });
	CameraManager::GetInstance()->AddCamera("previewCamera", *previewCamera_);

	//デフォルトカメラの設定
	Object3dCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());
	ParticleCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());

	particleEditor_ = std::make_unique<ParticleEditor>();
	particleEditor_->Initialize(TakeCFrameWork::GetParticleManager(), ParticleCommon::GetInstance());

	//SkyBox
	skyBox_ = std::make_unique<SkyBox>();
	skyBox_->Initialize(Object3dCommon::GetInstance()->GetDirectXCommon(), "skyBox_pool.obj");
	skyBox_->SetMaterialColor({ 0.2f,0.2f,0.2f,1.0f });

}

void ParticleEditScene::Finalize() {

	// カメラのリセット
	CameraManager::GetInstance()->ResetCameras();

	// ParticleManagerのリセット
	//TakeCFrameWork::GetParticleManager()

	// ParticleEditorの解放
	particleEditor_->Finalize();
}

void ParticleEditScene::Update() {

	
	//カメラの更新
	CameraManager::GetInstance()->Update();
	//SkyBoxの更新
	skyBox_->Update();

	// ParticleEditorの更新
	particleEditor_->Update();
}

void ParticleEditScene::UpdateImGui() {

	CameraManager::GetInstance()->UpdateImGui();
	ParticleCommon::GetInstance()->UpdateImGui();
	TakeCFrameWork::GetParticleManager()->UpdateImGui();

	// ImGuiの更新
	particleEditor_->UpdateImGui();
}

void ParticleEditScene::Draw() {

	skyBox_->Draw();

	//グリッド地面の描画
	TakeCFrameWork::GetWireFrame()->DrawGridGround({ 0.0f,0.0f,0.0f }, { 1000.0f, 1000.0f, 1000.0f }, 50);
	TakeCFrameWork::GetWireFrame()->DrawGridBox({
		{-500.0f,-500.0f,-500.0f},{500.0f,500.0f,500.0f } }, 2);
	TakeCFrameWork::GetWireFrame()->Draw();

	// ParticleEditorの描画
	particleEditor_->Draw();
}
