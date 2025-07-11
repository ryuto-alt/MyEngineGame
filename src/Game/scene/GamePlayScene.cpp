#include "GamePlayScene.h"
#include <stdexcept>
#include <cmath>

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
	engine_->SetCameraPosition(Vector3{ 0.0f, 0.0f, -2.0f });
	engine_->SetCameraFovY(1.37f);



	// 初期化完了（モデルはまだ読み込まない）
	initialized_ = true;
}

void GamePlayScene::Update() {
	if (!initialized_) return;

	// モデルがまだ読み込まれていない場合は読み込む
	if (!modelLoaded_) {
		// 通常の読み込み（高速化済み）
		humanAnimatedModel_ = engine_->CreateAnimatedModel();
		
		// モデルの読み込み（walkで読み込み）
		humanAnimatedModel_->LoadFromFile("Resources/Models/human", "walk.gltf");
		
		// 各アニメーションを読み込んで追加
		Animation walkAnim = humanAnimatedModel_->GetAnimationPlayer().GetAnimation();
		humanAnimatedModel_->AddAnimation("walk", walkAnim);
		
		// sneakWalkアニメーションを読み込み
		Animation sneakWalkAnim = engine_->LoadAnimation("Resources/Models/human", "sneakWalk.gltf");
		humanAnimatedModel_->AddAnimation("sneakWalk", sneakWalkAnim);
		
		// 初期アニメーションを"walk"に設定して再生
		humanAnimatedModel_->ChangeAnimation("walk");
		humanAnimatedModel_->PlayAnimation();

		humanObject3d_ = engine_->CreateObject3D();
		humanObject3d_->SetModel(static_cast<Model*>(humanAnimatedModel_.get()));
		humanObject3d_->SetAnimatedModel(humanAnimatedModel_.get());
		humanObject3d_->SetPosition(Vector3{ 0.0f, 0.0f, 0.0f });
		humanObject3d_->SetScale(Vector3{ 1.0f, 1.0f, 1.0f });
		humanObject3d_->SetColor(Vector4{ 1.0f, 1.0f, 1.0f, 1.0f });
		humanObject3d_->SetRotation(Vector3{ 0.0f,3.14f,0.0f });
		humanObject3d_->SetEnableLighting(true);
		humanObject3d_->SetEnableAnimation(true);
		humanObject3d_->SetCamera(camera_);
		
		modelLoaded_ = true;
		return; // 読み込みフレームでは他の処理をスキップ
	}

	// Humanモデルの読み込み完了後にGroundモデルを読み込み
	if (modelLoaded_ && !groundLoaded_) {
		try {
			groundModel_ = engine_->LoadModel("Resources/Models/ground/ground.obj");
			if (groundModel_) {
				groundObject3d_ = engine_->CreateObject3D();
				groundObject3d_->SetModel(groundModel_.get());
				groundObject3d_->SetPosition(Vector3{ 0.0f, -0.1f, 0.0f });
				groundObject3d_->SetScale(Vector3{ 1.0f, 1.0f, 1.0f });
				groundObject3d_->SetColor(Vector4{ 0.8f, 0.8f, 0.8f, 1.0f });
				groundObject3d_->SetRotation(Vector3{ 0.0f, 0.0f, 0.0f });
				groundObject3d_->SetEnableLighting(true);
				groundObject3d_->SetCamera(camera_);
			}
		} catch (const std::exception&) {
			// Ground モデルの読み込みに失敗した場合はログ出力のみ
			// アプリケーションは継続する
		}
		groundLoaded_ = true;
		return; // 読み込みフレームでは他の処理をスキップ
	}

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

	// Groundオブジェクトの更新
	if (groundObject3d_) {
		groundObject3d_->Update();
	}

	if (humanObject3d_ && humanAnimatedModel_) {
		Vector3 humanPos = humanObject3d_->GetPosition();

		// Xboxコントローラーによる移動処理
		float stickX = engine_->GetXboxLeftStickX();
		float stickY = engine_->GetXboxLeftStickY();
		bool bButtonPressed = engine_->IsXboxButtonPressed(0x2000); // B button
		bool bButtonTriggered = bButtonPressed && !previousBButtonPressed_;
		
		// 移動入力があるかチェック
		float stickMagnitude = std::sqrt(stickX * stickX + stickY * stickY);
		isMoving_ = stickMagnitude > 0.1f;
		
		// シンプルなBボタンのトグル処理
		if (bButtonTriggered && isMoving_ && !isBlending_) {
			// ブレンド中でなく、移動中のみBボタンが有効
			isSneaking_ = !isSneaking_;
			isBlending_ = true;
			blendTimer_ = 0.0f;
			
			if (isSneaking_) {
				OutputDebugStringA("[DEBUG] B押下: スニーク開始\n");
				humanAnimatedModel_->TransitionToAnimation("sneakWalk", 0.3f);
			} else {
				OutputDebugStringA("[DEBUG] B押下: walk開始\n");
				humanAnimatedModel_->TransitionToAnimation("walk", 0.3f);
			}
		}
		
		previousBButtonPressed_ = bButtonPressed;
		
		if (isMoving_) {
			// 移動量計算
			float moveSpeed = isSneaking_ ? humanSpeed_ * 0.5f : humanSpeed_;
			Vector3 movement = Vector3{stickX * moveSpeed, 0.0f, stickY * moveSpeed};
			
			// 移動方向を保存
			moveDirection_ = Vector3{stickX, 0.0f, stickY};
			
			// 移動方向に応じてモデルの向きを設定
			if (stickMagnitude > 0.1f) {
				float targetRotationY = std::atan2(stickX, stickY);
				currentRotationY_ = targetRotationY;
				humanObject3d_->SetRotation(Vector3{0.0f, currentRotationY_, 0.0f});
			}
			
			// 位置を更新
			humanPos = humanPos + movement;
		}

		humanObject3d_->SetPosition(humanPos);

		// アニメーション制御（コントローラー対応）
		if (engine_->IsKeyTriggered(DIK_P)) {
			animationPaused_ = !animationPaused_;
			
			if (animationPaused_) {
				humanAnimatedModel_->PauseAnimation();
			}
			else {
				humanAnimatedModel_->PlayAnimation();
			}
		}
		if (engine_->IsKeyTriggered(DIK_R)) {
			humanObject3d_->SetAnimationTime(0.0f);
		}
		
		// ブレンドタイマーの更新
		if (isBlending_) {
			blendTimer_ += 1.0f / 60.0f; // 60FPSを想定
			
			// ブレンドが完了したかチェック（タイマーベース）
			if (blendTimer_ >= BLEND_DURATION) {
				OutputDebugStringA("[DEBUG] ブレンド完了\n");
				isBlending_ = false;
				blendTimer_ = 0.0f;
			}
		}
		
		// 現在の状態をデバッグ出力（1秒に1回）
		static int frameCount = 0;
		frameCount++;
		if (frameCount % 60 == 0) {
			char debugMsg[256];
			sprintf_s(debugMsg, "[DEBUG] 状態: isMoving_=%d, isSneaking_=%d, isBlending_=%d, blendTimer_=%.2f, currentAnim=%s\n",
				isMoving_, isSneaking_, isBlending_, blendTimer_, 
				humanAnimatedModel_->GetCurrentAnimationName().c_str());
			OutputDebugStringA(debugMsg);
		}
		
		// アニメーション制御（シンプル版）
		if (isMoving_) {
			// 移動中のアニメーション再生
			if (animationPaused_) {
				animationPaused_ = false;
				humanAnimatedModel_->PlayAnimation();
			}
		} else {
			// 停止中はアニメーションを一時停止
			if (!animationPaused_) {
				animationPaused_ = true;
				humanAnimatedModel_->PauseAnimation();
			}
		}
		
		
		// キーボード手動制御（デバッグ用）
		if (engine_->IsKeyTriggered(DIK_1)) {
			if (humanAnimatedModel_->GetCurrentAnimationName() == "walk") {
				humanAnimatedModel_->TransitionToAnimation("sneakWalk", 0.3f);
			} else {
				humanAnimatedModel_->TransitionToAnimation("walk", 0.3f);
			}
		}

		// アニメーション更新
		if (!animationPaused_) {
			humanAnimatedModel_->Update(1.0f / 60.0f);
		}
		else {
			humanAnimatedModel_->Update(0.0f); 
		}

		
		humanObject3d_->Update();
	}

	
	camera_->Update();
}

void GamePlayScene::Draw() {
	if (!initialized_) return;


	spriteCommon_->CommonDraw();

#pragma region imgui
	// モデルが読み込まれていない場合はローディング表示
	if (!modelLoaded_ || !groundLoaded_) {
		ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f - 100, ImGui::GetIO().DisplaySize.y * 0.5f - 25), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(200, 50), ImGuiCond_Always);
		ImGui::Begin("Loading", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		if (!modelLoaded_) {
			ImGui::Text("Loading Human Model...");
		} else if (!groundLoaded_) {
			ImGui::Text("Loading Ground Model...");
		}
		ImGui::End();
	} else {
		// Groundオブジェクトの描画
		if (groundObject3d_) {
			groundObject3d_->Draw();
		}
		
		// アニメーション付きヒューマンモデルの描画
		if (humanObject3d_) {
			humanObject3d_->Draw();
		}
	}

	// モデルが読み込まれている場合のみ操作説明を表示
	if (modelLoaded_ && groundLoaded_) {
		// シンプルなImGuiウィンドウ
		ImGui::Begin("Human Animation Demo ");

		ImGui::Text("操作方法:");
		ImGui::Separator();
		ImGui::Text("【キーボード】");
		ImGui::Text("WASD - カメラ移動");
		ImGui::Text("SPACE - 上昇");
		ImGui::Text("SHIFT - 下降");
		ImGui::Text("P - アニメーション一時停止/再開");
		ImGui::Text("R - アニメーションリセット");
		ImGui::Text("1 - アニメーション切り替え（SneakWalk ⇔ Walk）");
		ImGui::Text("ESC - 終了");
		
		ImGui::Separator();
		ImGui::Text("【Xboxコントローラー】");
		ImGui::Text("左スティック - ヒューマンモデル移動");
		ImGui::Text("移動中にBボタン - スニーク状態");
		ImGui::Text("※停止中はスニーク無効");
		ImGui::Text("※コントローラー接続: %s", engine_->IsXboxControllerConnected() ? "接続済み" : "未接続");

		ImGui::Separator();
		Vector3 cameraPos = engine_->GetCameraPosition();
		ImGui::Text("カメラ位置: (%.1f, %.1f, %.1f)", cameraPos.x, cameraPos.y, cameraPos.z);
		
		// アニメーション情報
		ImGui::Separator();
		ImGui::Text("現在のアニメーション: %s", humanAnimatedModel_->GetCurrentAnimationName().c_str());
		
		std::string stateText;
		if (isBlending_) {
			stateText = "ブレンド中";
		} else if (isSneaking_) {
			stateText = "スニーク中";
		} else if (isMoving_) {
			stateText = "移動中";
		} else {
			stateText = "停止中";
		}
		ImGui::Text("状態: %s", stateText.c_str());
		
		if (isBlending_) {
			ImGui::Text("ブレンド進行: %.1f%%", (blendTimer_ / BLEND_DURATION) * 100.0f);
		}

		ImGui::End();
	}
}
#pragma endregion

void GamePlayScene::Finalize() {
	// ヒューマンモデルの終了処理
	if (humanObject3d_) {
		humanObject3d_.reset();
	}

	if (humanAnimatedModel_) {
		humanAnimatedModel_.reset();
	}

	// Groundモデルの終了処理
	if (groundObject3d_) {
		groundObject3d_.reset();
	}

	if (groundModel_) {
		groundModel_.reset();
	}
}
