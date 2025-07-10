#include "GamePlayScene.h"
#include "Vector3.h"
#include "imgui.h"
#include "SpriteCommon.h"

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

	
	humanAnimatedModel_ = std::make_unique<AnimatedModel>();
	humanAnimatedModel_->Initialize(dxCommon_);
	humanAnimatedModel_->LoadFromFile("Resources/Models/human", "walk.gltf");
	humanAnimatedModel_->PlayAnimation();
	
	// デバッグ：アニメーションが読み込まれたか確認
	const Animation& anim = humanAnimatedModel_->GetAnimationPlayer().GetAnimation();
	OutputDebugStringA(("GamePlayScene: Animation loaded - duration: " + std::to_string(anim.duration) + 
	                    " seconds, nodeAnimations: " + std::to_string(anim.nodeAnimations.size()) + "\n").c_str());
	for (const auto& nodeAnim : anim.nodeAnimations) {
		OutputDebugStringA(("  Node: " + nodeAnim.first + 
		                    " - translate keys: " + std::to_string(nodeAnim.second.translate.size()) +
		                    ", rotate keys: " + std::to_string(nodeAnim.second.rotate.size()) +
		                    ", scale keys: " + std::to_string(nodeAnim.second.scale.size()) + "\n").c_str());
	}

	humanObject3d_ = engine_->CreateObject3D();
	humanObject3d_->SetModel(static_cast<Model*>(humanAnimatedModel_.get()));
	humanObject3d_->SetAnimatedModel(humanAnimatedModel_.get());
	humanObject3d_->SetPosition(Vector3{ 0.0f, 0.0f, 0.0f });
	humanObject3d_->SetScale(Vector3{ 1.0f, 1.0f, 1.0f });
	humanObject3d_->SetColor(Vector4{ 1.0f, 1.0f, 1.0f, 1.0f });
	humanObject3d_->SetEnableLighting(true);
	humanObject3d_->SetEnableAnimation(true);
	humanObject3d_->SetCamera(camera_);

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
	Vector3 currentPos = engine_->GetCameraPosition();
	if (engine_->IsKeyPressed(DIK_W)) currentPos.z += moveSpeed_;
	if (engine_->IsKeyPressed(DIK_S)) currentPos.z -= moveSpeed_;
	if (engine_->IsKeyPressed(DIK_A)) currentPos.x -= moveSpeed_;
	if (engine_->IsKeyPressed(DIK_D)) currentPos.x += moveSpeed_;
	if (engine_->IsKeyPressed(DIK_SPACE)) currentPos.y += moveSpeed_;
	if (engine_->IsKeyPressed(DIK_LSHIFT)) currentPos.y -= moveSpeed_;
	engine_->SetCameraPosition(currentPos);


	if (humanObject3d_ && humanAnimatedModel_) {
		Vector3 humanPos = humanObject3d_->GetPosition();
		if (engine_->IsKeyPressed(DIK_UP)) humanPos.z += humanSpeed_;
		if (engine_->IsKeyPressed(DIK_DOWN)) humanPos.z -= humanSpeed_;
		if (engine_->IsKeyPressed(DIK_LEFT)) humanPos.x -= humanSpeed_;
		if (engine_->IsKeyPressed(DIK_RIGHT)) humanPos.x += humanSpeed_;
		humanObject3d_->SetPosition(humanPos);
		
		// アニメーション制御
		if (engine_->IsKeyTriggered(DIK_P)) {
			animationPaused_ = !animationPaused_;
			// SetEnableAnimationは呼ばない（スケルトン更新を継続するため）
			if (animationPaused_) {
				humanAnimatedModel_->PauseAnimation();
			} else {
				humanAnimatedModel_->PlayAnimation();
			}
		}
		if (engine_->IsKeyTriggered(DIK_R)) {
			humanObject3d_->SetAnimationTime(0.0f);
		}
		
		// アニメーションモデルの更新（一時停止中は0を渡す）
		if (!animationPaused_) {
			humanAnimatedModel_->Update(1.0f / 60.0f);
		} else {
			humanAnimatedModel_->Update(0.0f);  // 時刻を進めない
		}
		
		// Object3dの更新
		humanObject3d_->Update();
	}

	// カメラの更新
	camera_->Update();
}

void GamePlayScene::Draw() {
	if (!initialized_) return;


	spriteCommon_->CommonDraw();

	// アニメーション付きヒューマンモデルの描画
	if (humanObject3d_) {
		humanObject3d_->Draw();
	}

	// シンプルなImGuiウィンドウ
	ImGui::Begin("Human Animation Demo ");

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

	if (humanObject3d_) {
		ImGui::Separator();
		Vector3 humanPos = humanObject3d_->GetPosition();
		ImGui::Text("ヒューマン位置: (%.1f, %.1f, %.1f)", humanPos.x, humanPos.y, humanPos.z);
		ImGui::Text("アニメーション時間: %.2f秒", humanObject3d_->GetAnimationTime());
		ImGui::Text("アニメーション状態: %s", humanObject3d_->GetEnableAnimation() ? "再生中" : "停止");
	}

	if (humanAnimatedModel_) {
		ImGui::Separator();
		ImGui::Text("アニメーション総時間: %.2f秒", humanAnimatedModel_->GetAnimationPlayer().GetDuration());
		ImGui::Text("ノードアニメーション数: %d", (int)humanAnimatedModel_->GetAnimationPlayer().GetAnimation().nodeAnimations.size());
		ImGui::Text("ジョイント数: %d", (int)humanAnimatedModel_->GetSkeleton().joints.size());
	}

	ImGui::End();
}

void GamePlayScene::Finalize() {
	// ヒューマンモデルの終了処理
	if (humanObject3d_) {
		humanObject3d_.reset();
	}
	
	if (humanAnimatedModel_) {
		humanAnimatedModel_.reset();
	}
}
