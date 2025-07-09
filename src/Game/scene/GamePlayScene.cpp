#include "GamePlayScene.h"
#include "Vector3.h"
#include "imgui.h"

GamePlayScene::GamePlayScene() {
}

GamePlayScene::~GamePlayScene() {
}

void GamePlayScene::Initialize() {
	// 必要なリソースの取得確認
	assert(dxCommon_);
	assert(input_);
	assert(spriteCommon_);
	assert(camera_);

	// UnoEngineインスタンスを取得
	engine_ = UnoEngine::GetInstance();

	// カメラの初期設定
	engine_->SetCameraPosition(Vector3{ 0.0f, 0.0f, -10.0f });
	engine_->SetCameraFovY(1.37f);


	// 初期化完了
	initialized_ = true;
}

void GamePlayScene::Update() {
	if (!initialized_) return;

	// ESCキーでアプリケーション終了
	if (engine_->IsKeyTriggered(DIK_ESCAPE)) {
		exit(0);
	}

	// WASD + SPACE/SHIFT でカメラ移動
	const float moveSpeed = 0.1f;
	Vector3 currentPos = engine_->GetCameraPosition();

	if (engine_->IsKeyPressed(DIK_W)) currentPos.z += moveSpeed;
	if (engine_->IsKeyPressed(DIK_S)) currentPos.z -= moveSpeed;
	if (engine_->IsKeyPressed(DIK_A)) currentPos.x -= moveSpeed;
	if (engine_->IsKeyPressed(DIK_D)) currentPos.x += moveSpeed;
	if (engine_->IsKeyPressed(DIK_SPACE)) currentPos.y += moveSpeed;
	if (engine_->IsKeyPressed(DIK_LSHIFT)) currentPos.y -= moveSpeed;

	engine_->SetCameraPosition(currentPos);

	// カメラの更新
	camera_->Update();
}

void GamePlayScene::Draw() {
	if (!initialized_) return;

	// シンプルなImGuiウィンドウ
	ImGui::Begin("Simple GamePlay Scene");

	ImGui::Text("操作方法:");
	ImGui::Text("WASD - カメラ移動");
	ImGui::Text("SPACE - 上昇");
	ImGui::Text("SHIFT - 下降");
	ImGui::Text("ESC - 終了");

	ImGui::Separator();
	Vector3 cameraPos = engine_->GetCameraPosition();
	ImGui::Text("カメラ位置: (%.1f, %.1f, %.1f)", cameraPos.x, cameraPos.y, cameraPos.z);

	ImGui::End();
}

void GamePlayScene::Finalize() {
}
