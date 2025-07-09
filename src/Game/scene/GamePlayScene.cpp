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


	humanObject3d_ = engine_->CreateObject3D();
	humanObject3d_->SetModel(static_cast<Model*>(humanAnimatedModel_.get()));
	humanObject3d_->SetPosition(Vector3{ 0.0f, 0.0f, 0.0f });
	humanObject3d_->SetScale(Vector3{ 1.0f, 1.0f, 1.0f });
	humanObject3d_->SetColor(Vector4{ 1.0f, 1.0f, 1.0f, 1.0f });
	humanObject3d_->SetEnableLighting(true);

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


	if (humanObject3d_) {
		Vector3 humanPos = humanObject3d_->GetPosition();
		if (engine_->IsKeyPressed(DIK_UP)) humanPos.z += humanSpeed_;
		if (engine_->IsKeyPressed(DIK_DOWN)) humanPos.z -= humanSpeed_;
		if (engine_->IsKeyPressed(DIK_LEFT)) humanPos.x -= humanSpeed_;
		if (engine_->IsKeyPressed(DIK_RIGHT)) humanPos.x += humanSpeed_;
		humanObject3d_->SetPosition(humanPos);
		
		// アニメーション制御
		if (engine_->IsKeyTriggered(DIK_P)) {
			animationPaused_ = !animationPaused_;
		}
		if (engine_->IsKeyTriggered(DIK_R)) {
			animationTime_ = 0.0f;
		}
	}


	if (humanAnimatedModel_ && enableAnimation_ && !animationPaused_) {
		// AnimatedModelの更新
		humanAnimatedModel_->Update(1.0f / 60.0f);
		
		// アニメーション時間の更新
		animationTime_ += 1.0f / 60.0f;
		if (humanAnimatedModel_->GetAnimationPlayer().GetDuration() > 0.0f) {
			animationTime_ = std::fmod(animationTime_, humanAnimatedModel_->GetAnimationPlayer().GetDuration());
		}


		if (humanObject3d_) {
			Animation& animation = humanAnimatedModel_->GetAnimationPlayer().GetAnimation();
			Skeleton& skeleton = humanAnimatedModel_->GetSkeleton();
			SkinCluster& skinCluster = humanAnimatedModel_->GetSkinCluster();
			
			if (animation.nodeAnimations.size() > 0) {
				humanObject3d_->ApplyAnimation(skeleton, animation, animationTime_);
				humanObject3d_->SkeletonUpdate(skeleton);
				humanObject3d_->SkinClusterUpdate(skinCluster, skeleton);
			}
			
			// Object3dの更新
			humanObject3d_->Update();
		}
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
		ImGui::Text("アニメーション時間: %.2f秒", animationTime_);
		ImGui::Text("アニメーション状態: %s", animationPaused_ ? "一時停止" : "再生中");
		ImGui::Text("アニメーション有効: %s", enableAnimation_ ? "有効" : "無効");
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
