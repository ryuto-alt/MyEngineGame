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

	// シンプルなキューブオブジェクトの作成
	cubeObject_ = engine_->CreateObject3D();
	cubeModel_ = engine_->LoadModel("Resources/Models/cube/cube.obj");
	cubeObject_->SetModel(cubeModel_.get());
	
	// キューブの初期位置を設定
	cubePosition_ = Vector3{ 0.0f, 0.0f, 0.0f };
	cubeObject_->SetPosition(cubePosition_);

	// 初期化完了
	initialized_ = true;
}

void GamePlayScene::Update() {
	if (!initialized_) return;

	// ESCキーでアプリケーション終了
	if (engine_->IsKeyTriggered(DIK_ESCAPE)) {
		exit(0);
	}

	// WASD でカメラ移動
	const float moveSpeed = 0.1f;
	Vector3 currentPos = engine_->GetCameraPosition();

	if (engine_->IsKeyPressed(DIK_W)) currentPos.z += moveSpeed;
	if (engine_->IsKeyPressed(DIK_S)) currentPos.z -= moveSpeed;
	if (engine_->IsKeyPressed(DIK_A)) currentPos.x -= moveSpeed;
	if (engine_->IsKeyPressed(DIK_D)) currentPos.x += moveSpeed;

	engine_->SetCameraPosition(currentPos);

	// 十字キーでキューブ移動
	const float cubeSpeed = 0.05f;

	if (engine_->IsKeyPressed(DIK_UP)) cubePosition_.z += cubeSpeed;
	if (engine_->IsKeyPressed(DIK_DOWN)) cubePosition_.z -= cubeSpeed;
	if (engine_->IsKeyPressed(DIK_LEFT)) cubePosition_.x -= cubeSpeed;
	if (engine_->IsKeyPressed(DIK_RIGHT)) cubePosition_.x += cubeSpeed;

	// キューブの位置を更新
	cubeObject_->SetPosition(cubePosition_);

	// オブジェクトの更新
	cubeObject_->Update();

	// カメラの更新
	camera_->Update();
}

void GamePlayScene::Draw() {
	if (!initialized_) return;

	// 3Dオブジェクトの描画
	cubeObject_->Draw();

	// シンプルなImGuiウィンドウ
	ImGui::Begin("Simple GamePlay Scene");

	ImGui::Text("操作方法:");
	ImGui::Text("WASD - カメラ移動");
	ImGui::Text("↑↓←→ - キューブ移動");
	ImGui::Text("ESC - 終了");

	ImGui::Separator();
	ImGui::Text("キューブ位置: (%.1f, %.1f, %.1f)", cubePosition_.x, cubePosition_.y, cubePosition_.z);

	ImGui::End();
}

void GamePlayScene::Finalize() {
	cubeObject_.reset();
	cubeModel_.reset();
}
