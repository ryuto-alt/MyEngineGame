#include "TitleScene.h"
#include "GamePlayScene.h"
#include "SceneManager.h"
#include <algorithm>

void TitleScene::Initialize() {

	//Camera0
	camera0_ = std::make_shared<Camera>();
	camera0_->Initialize(CameraManager::GetInstance()->GetDirectXCommon()->GetDevice());
	camera0_->SetTranslate({ 0.0f,0.0f,-20.0f });
	camera0_->SetRotate({ 0.1f,0.0f,0.0f });
	CameraManager::GetInstance()->AddCamera("Tcamera0", *camera0_);

	//デフォルトカメラの設定
	Object3dCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());
	// Model読み込み
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend","Title.obj");
	ModelManager::GetInstance()->LoadModel("obj_mtl_blend","enterStartText.obj");

	ModelManager::GetInstance()->LoadModel("gltf","TitleText.gltf");
	ModelManager::GetInstance()->LoadModel("gltf","PushEnter.gltf");

	// Sprite
	sprite_ = std::make_shared<Sprite>();
	sprite_->Initialize(SpriteCommon::GetInstance(), "TitleText.png");
	sprite_->AdjustTextureSize();
	sprite_->SetPosition({ 512.0f, 256.0f});

	// object
	startObject = std::make_unique<Object3d>();
	startObject->Initialize(Object3dCommon::GetInstance(), "enterStartText.obj");
	startObject->SetScale({ 0.8f, 0.8f, 0.8f });
	startObject->SetRotation({ 0.0f, 0.0f, 3.14f });
	startObject->SetTranslate({ -1.0f, -2.5f, 0.0f });
}

void TitleScene::Finalize() {
	sprite_.reset();
	titleObject.reset();
	camera0_.reset();
	camera1_.reset();
	CameraManager::GetInstance()->ResetCameras();
}

void TitleScene::Update() {

	//カメラの更新
	CameraManager::GetInstance()->Update();

	sprite_->Update();
	startObject->Update();

	//シーン遷移
	if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
		//シーン切り替え依頼
		SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
	}
}

void TitleScene::UpdateImGui() {

	//ImGuiの更新
	CameraManager::GetInstance()->UpdateImGui();
	sprite_->UpdateImGui("title");
	titleObject->UpdateImGui("title");
}

void TitleScene::Draw() {

	SpriteCommon::GetInstance()->PreDraw();
	sprite_->Draw();
	Object3dCommon::GetInstance()->PreDraw();
	startObject->Draw();
}