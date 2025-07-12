#include "GamePlayScene.h"
#include <stdexcept>
#include <cmath>

GamePlayScene::GamePlayScene() {
}

GamePlayScene::~GamePlayScene() {
}

void GamePlayScene::Initialize() {
	assert(dxCommon_);
	assert(input_);
	assert(spriteCommon_);
	assert(camera_);

	engine_ = UnoEngine::GetInstance();

	engine_->SetCameraPosition(Vector3{ 0.0f, 0.0f, -2.0f });
	engine_->SetCameraFovY(1.37f);

	// 回転の初期化
	currentRotationY_ = 0.0f;
	targetRotationY_ = 0.0f;

	initialized_ = true;
}

void GamePlayScene::Update() {
	if (!initialized_) return;

	if (!modelLoaded_) {
		humanAnimatedModel_ = engine_->CreateAnimatedModel();
		humanAnimatedModel_->LoadFromFile("Resources/Models/human", "walk.gltf");
		
		Animation walkAnim = humanAnimatedModel_->GetAnimationPlayer().GetAnimation();
		humanAnimatedModel_->AddAnimation("walk", walkAnim);
		
		Animation sneakWalkAnim = engine_->LoadAnimation("Resources/Models/human", "sneakWalk.gltf");
		humanAnimatedModel_->AddAnimation("sneakWalk", sneakWalkAnim);
		
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
		return;
	}

	if (modelLoaded_ && !groundLoaded_) {
		try {
			groundModel_ = engine_->LoadModel("Resources/Models/ground/ground.obj");
			if (groundModel_) {
				groundObject3d_ = engine_->CreateObject3D();
				groundObject3d_->SetModel(groundModel_.get());
				groundObject3d_->SetPosition(Vector3{ 0.0f, -0.1f, 0.0f });
				groundObject3d_->SetScale(Vector3{ 1.0f, 1.0f, 1.0f });
				//groundObject3d_->SetColor(Vector4{ 0.8f, 0.8f, 0.8f, 1.0f });
				groundObject3d_->SetRotation(Vector3{ 0.0f, 0.0f, 0.0f });
				groundObject3d_->SetEnableLighting(true);
				groundObject3d_->SetCamera(camera_);
			}
		} catch (const std::exception&) {
		}
		groundLoaded_ = true;
		return;
	}

	if (engine_->IsKeyTriggered(DIK_ESCAPE)) {
		exit(0);
	}

	Vector3 currentPos = engine_->GetCameraPosition();
	if (engine_->IsKeyPressed(DIK_W)) currentPos.z += moveSpeed_;
	if (engine_->IsKeyPressed(DIK_S)) currentPos.z -= moveSpeed_;
	if (engine_->IsKeyPressed(DIK_A)) currentPos.x -= moveSpeed_;
	if (engine_->IsKeyPressed(DIK_D)) currentPos.x += moveSpeed_;
	if (engine_->IsKeyPressed(DIK_SPACE)) currentPos.y += moveSpeed_;
	if (engine_->IsKeyPressed(DIK_LSHIFT)) currentPos.y -= moveSpeed_;
	engine_->SetCameraPosition(currentPos);

	if (groundObject3d_) {
		groundObject3d_->Update();
	}

	if (humanObject3d_ && humanAnimatedModel_) {
		Vector3 humanPos = humanObject3d_->GetPosition();

		float stickX = engine_->GetXboxLeftStickX();
		float stickY = engine_->GetXboxLeftStickY();
		bool bButtonPressed = engine_->IsXboxButtonPressed(0x2000);
		bool bButtonTriggered = bButtonPressed && !previousBButtonPressed_;
		
		float stickMagnitude = std::sqrt(stickX * stickX + stickY * stickY);
		isMoving_ = stickMagnitude > 0.1f;
		
		if (bButtonTriggered && isMoving_ && !isBlending_) {
			isSneaking_ = !isSneaking_;
			isBlending_ = true;
			blendTimer_ = 0.0f;
			
			if (isSneaking_) {
				humanAnimatedModel_->TransitionToAnimation("sneakWalk", 0.3f);
			} else {
				humanAnimatedModel_->TransitionToAnimation("walk", 0.3f);
			}
		}
		
		previousBButtonPressed_ = bButtonPressed;
		
		if (isMoving_) {
			float moveSpeed = isSneaking_ ? humanSpeed_ * 0.5f : humanSpeed_;
			Vector3 movement = Vector3{stickX * moveSpeed, 0.0f, stickY * moveSpeed};
			
			moveDirection_ = Vector3{stickX, 0.0f, stickY};
			
			if (stickMagnitude > 0.1f) {
				targetRotationY_ = std::atan2(stickX, stickY);
			}
			
			humanPos = humanPos + movement;
		}

		humanObject3d_->SetPosition(humanPos);

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
		
		if (isBlending_) {
			blendTimer_ += 1.0f / 60.0f;
			
			if (blendTimer_ >= BLEND_DURATION) {
				isBlending_ = false;
				blendTimer_ = 0.0f;
			}
		}
		
		if (isMoving_) {
			if (animationPaused_) {
				animationPaused_ = false;
				humanAnimatedModel_->PlayAnimation();
			}
		} else {
			if (!animationPaused_) {
				animationPaused_ = true;
				humanAnimatedModel_->PauseAnimation();
			}
		}
		
		
		if (engine_->IsKeyTriggered(DIK_1)) {
			if (humanAnimatedModel_->GetCurrentAnimationName() == "walk") {
				humanAnimatedModel_->TransitionToAnimation("sneakWalk", 0.3f);
			} else {
				humanAnimatedModel_->TransitionToAnimation("walk", 0.3f);
			}
		}

		if (!animationPaused_) {
			humanAnimatedModel_->Update(1.0f / 60.0f);
		}
		else {
			humanAnimatedModel_->Update(0.0f); 
		}

		// 回転のスムーシング処理（UnoEngineの機能を使用）
		float deltaTime = 1.0f / 60.0f;  // フレームレート60FPS想定
		currentRotationY_ = engine_->SmoothRotation(currentRotationY_, targetRotationY_, rotationSmoothingSpeed_, deltaTime);
		humanObject3d_->SetRotation(Vector3{0.0f, currentRotationY_, 0.0f});

		humanObject3d_->Update();
	}

	camera_->Update();
}

void GamePlayScene::Draw() {
	if (!initialized_) return;


	spriteCommon_->CommonDraw();

#pragma region imgui
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
		if (groundObject3d_) {
			groundObject3d_->Draw();
		}
		
		if (humanObject3d_) {
			humanObject3d_->Draw();
		}
	}

	if (modelLoaded_ && groundLoaded_) {
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

		// 回転スムーシング設定
		ImGui::Separator();
		ImGui::Text("回転スムーシング設定:");
		ImGui::SliderFloat("スムーシング速度", &rotationSmoothingSpeed_, 0.1f, 20.0f, "%.1f");
		ImGui::Text("現在の回転: %.2f度", currentRotationY_ * 180.0f / 3.14159f);
		ImGui::Text("目標回転: %.2f度", targetRotationY_ * 180.0f / 3.14159f);

		ImGui::End();
	}
}
#pragma endregion

void GamePlayScene::Finalize() {
	if (humanObject3d_) {
		humanObject3d_.reset();
	}

	if (humanAnimatedModel_) {
		humanAnimatedModel_.reset();
	}

	if (groundObject3d_) {
		groundObject3d_.reset();
	}

	if (groundModel_) {
		groundModel_.reset();
	}
}
