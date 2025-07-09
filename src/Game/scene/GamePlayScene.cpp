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

	// アニメーション付きヒューマンモデルの初期化
	humanController_ = std::make_unique<AnimatedHumanController>();
	humanController_->Initialize(dxCommon_);
	humanController_->SetPosition(Vector3{ 0.0f, 0.0f, 0.0f });
	humanController_->SetScale(Vector3{ 1.0f, 1.0f, 1.0f });
	humanController_->SetColor(Vector4{ 1.0f, 1.0f, 1.0f, 1.0f });

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

	// 十字キーでヒューマンモデル移動
	if (humanController_) {
		const float humanSpeed = 0.05f;
		Vector3 humanPos = humanController_->GetPosition();
		
		if (engine_->IsKeyPressed(DIK_UP)) humanPos.z += humanSpeed;
		if (engine_->IsKeyPressed(DIK_DOWN)) humanPos.z -= humanSpeed;
		if (engine_->IsKeyPressed(DIK_LEFT)) humanPos.x -= humanSpeed;
		if (engine_->IsKeyPressed(DIK_RIGHT)) humanPos.x += humanSpeed;
		
		humanController_->SetPosition(humanPos);
		
		// アニメーション制御
		if (engine_->IsKeyTriggered(DIK_P)) {
			humanController_->ToggleAnimation();
		}
		if (engine_->IsKeyTriggered(DIK_R)) {
			humanController_->ResetAnimation();
		}
		
		// ヒューマンモデルの更新
		humanController_->Update();
	}

	// カメラの更新
	camera_->Update();
}

void GamePlayScene::Draw() {
	if (!initialized_) return;

	// アニメーション付きヒューマンモデルの描画
	if (humanController_) {
		humanController_->Draw();
	}

	// シンプルなImGuiウィンドウ
	ImGui::Begin("Human Animation Demo");

	ImGui::Text("操作方法:");
	ImGui::Text("WASD - カメラ移動");
	ImGui::Text("SPACE - 上昇");
	ImGui::Text("SHIFT - 下降");
	ImGui::Text("↑↓←→ - ヒューマンモデル移動");
	ImGui::Text("P - アニメーション一時停止/再開");
	ImGui::Text("R - アニメーションリセット");
	ImGui::Text("ESC - 終了");

	ImGui::Separator();
	Vector3 cameraPos = engine_->GetCameraPosition();
	ImGui::Text("カメラ位置: (%.1f, %.1f, %.1f)", cameraPos.x, cameraPos.y, cameraPos.z);

	if (humanController_) {
		ImGui::Separator();
		Vector3 humanPos = humanController_->GetPosition();
		ImGui::Text("ヒューマン位置: (%.1f, %.1f, %.1f)", humanPos.x, humanPos.y, humanPos.z);
		ImGui::Text("アニメーション時間: %.2f秒", humanController_->GetAnimationTime());
		ImGui::Text("アニメーションスピード: %.2f", humanController_->GetAnimationSpeed());
		ImGui::Text("アニメーション状態: %s", humanController_->IsAnimationPaused() ? "一時停止" : "再生中");
	}

	ImGui::End();
}

void GamePlayScene::Finalize() {
	// ヒューマンモデルの終了処理
	if (humanController_) {
		humanController_->Finalize();
		humanController_.reset();
	}
}
