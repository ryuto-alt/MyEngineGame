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
	
	// ディレクショナルライトの初期設定
	directionalLight_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLight_.direction = { 0.0f, -1.0f, 0.5f };
	directionalLight_.intensity = 1.0f;
	
	// スポットライトの初期設定
	spotLight_.color = { 1.0f, 0.9f, 0.8f, 1.0f };
	spotLight_.position = { 0.0f, 5.0f, -2.0f };
	spotLight_.intensity = 2.0f;
	spotLight_.direction = { 0.0f, -1.0f, 0.3f };
	spotLight_.innerCone = cosf(12.0f * 3.14159265f / 180.0f);
	spotLight_.attenuation = { 1.0f, 0.09f, 0.032f };
	spotLight_.outerCone = cosf(20.0f * 3.14159265f / 180.0f);

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
		humanObject3d_->SetDirectionalLight(directionalLight_);
		humanObject3d_->SetSpotLight(spotLight_);
		
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
				groundObject3d_->SetDirectionalLight(directionalLight_);
				groundObject3d_->SetSpotLight(spotLight_);
			}
		} catch (const std::exception&) {
		}
		groundLoaded_ = true;
		return;
	}

	// GLBモデルの読み込み
	if (groundLoaded_ && !cubeGlbLoaded_) {
		try {
			cubeGlbModel_ = std::make_unique<Model>();
			cubeGlbModel_->Initialize(dxCommon_);
			cubeGlbModel_->LoadFromGLB("Resources/Models/cube/cube.glb");
			
			if (cubeGlbModel_) {
				cubeGlbObject3d_ = engine_->CreateObject3D();
				cubeGlbObject3d_->SetModel(cubeGlbModel_.get());
				cubeGlbObject3d_->SetPosition(Vector3{ 2.0f, 0.0f, 0.0f }); // 少し右側に配置
				cubeGlbObject3d_->SetScale(Vector3{ 1.0f, 1.0f, 1.0f });
				cubeGlbObject3d_->SetRotation(Vector3{ 0.0f, 0.0f, 0.0f });
				cubeGlbObject3d_->SetEnableLighting(true);
				cubeGlbObject3d_->SetCamera(camera_);
				cubeGlbObject3d_->SetDirectionalLight(directionalLight_);
				cubeGlbObject3d_->SetSpotLight(spotLight_);
				OutputDebugStringA("GLB Cube model loaded successfully\n");
			}
		} catch (const std::exception& e) {
			OutputDebugStringA(("Failed to load GLB cube model: " + std::string(e.what()) + "\n").c_str());
		}
		cubeGlbLoaded_ = true;
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

	// ライトパラメータの更新
	if (humanObject3d_) {
		humanObject3d_->SetDirectionalLight(directionalLight_);
		humanObject3d_->SetSpotLight(spotLight_);
	}
	if (groundObject3d_) {
		groundObject3d_->SetDirectionalLight(directionalLight_);
		groundObject3d_->SetSpotLight(spotLight_);
		groundObject3d_->Update();
	}

	if (cubeGlbObject3d_) {
		cubeGlbObject3d_->SetDirectionalLight(directionalLight_);
		cubeGlbObject3d_->SetSpotLight(spotLight_);
		cubeGlbObject3d_->Update();
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
		
		// Fキーでライティングデバッグウィンドウの表示切り替え
		if (engine_->IsKeyTriggered(DIK_F)) {
			showLightingDebug_ = !showLightingDebug_;
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
	if (!modelLoaded_ || !groundLoaded_ || !cubeGlbLoaded_) {
		ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f - 100, ImGui::GetIO().DisplaySize.y * 0.5f - 25), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(200, 50), ImGuiCond_Always);
		ImGui::Begin("Loading", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		if (!modelLoaded_) {
			ImGui::Text("Loading Human Model...");
		} else if (!groundLoaded_) {
			ImGui::Text("Loading Ground Model...");
		} else if (!cubeGlbLoaded_) {
			ImGui::Text("Loading GLB Cube Model...");
		}
		ImGui::End();
	} else {
		if (groundObject3d_) {
			groundObject3d_->Draw();
		}
		
		if (cubeGlbObject3d_) {
			cubeGlbObject3d_->Draw();
		}
		
		if (humanObject3d_) {
			humanObject3d_->Draw();
		}
	}

	if (modelLoaded_ && groundLoaded_ && cubeGlbLoaded_) {
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
		ImGui::Text("F - ライティング設定の表示/非表示");
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
		
		// ライティングデバッグウィンドウ
		if (showLightingDebug_) {
			ImGui::Begin("Lighting Settings (F キーで表示切替)");
			
			// ライティング状態の確認
			ImGui::Text("ライティング状態:");
			ImGui::Text("  Human: %s", humanObject3d_ && humanObject3d_->GetEnableLighting() ? "有効" : "無効");
			ImGui::Text("  Ground: %s", groundObject3d_ && groundObject3d_->GetEnableLighting() ? "有効" : "無効");
			ImGui::Text("  Cube: %s", cubeGlbObject3d_ && cubeGlbObject3d_->GetEnableLighting() ? "有効" : "無効");
			
			// ディレクショナルライト設定
			ImGui::Separator();
			ImGui::Text("Directional Light");
			ImGui::Checkbox("Enable Directional Light", &enableDirectionalLight_);
			if (enableDirectionalLight_) {
				ImGui::ColorEdit3("Dir Light Color", &directionalLight_.color.x);
				ImGui::SliderFloat3("Dir Light Direction", &directionalLight_.direction.x, -1.0f, 1.0f);
				ImGui::SliderFloat("Dir Light Intensity", &directionalLight_.intensity, 0.0f, 3.0f);
				
				// 方向ベクトルを正規化
				float dirLength = sqrtf(directionalLight_.direction.x * directionalLight_.direction.x +
									   directionalLight_.direction.y * directionalLight_.direction.y +
									   directionalLight_.direction.z * directionalLight_.direction.z);
				if (dirLength > 0.001f) {
					directionalLight_.direction.x /= dirLength;
					directionalLight_.direction.y /= dirLength;
					directionalLight_.direction.z /= dirLength;
				}
			}
			
			// スポットライト設定
			ImGui::Separator();
			ImGui::Text("Spot Light");
			ImGui::Checkbox("Enable Spot Light", &enableSpotLight_);
			if (enableSpotLight_) {
				ImGui::ColorEdit3("Spot Light Color", &spotLight_.color.x);
				ImGui::DragFloat3("Spot Light Position", &spotLight_.position.x, 0.1f);
				ImGui::SliderFloat3("Spot Light Direction", &spotLight_.direction.x, -1.0f, 1.0f);
				ImGui::SliderFloat("Spot Light Intensity", &spotLight_.intensity, 0.0f, 5.0f);
				
				// コーン角度（度数で表示）
				float innerAngle = acosf(spotLight_.innerCone) * 180.0f / 3.14159265f;
				float outerAngle = acosf(spotLight_.outerCone) * 180.0f / 3.14159265f;
				
				if (ImGui::SliderFloat("Inner Cone Angle", &innerAngle, 0.0f, 90.0f)) {
					spotLight_.innerCone = cosf(innerAngle * 3.14159265f / 180.0f);
				}
				if (ImGui::SliderFloat("Outer Cone Angle", &outerAngle, 0.0f, 90.0f)) {
					spotLight_.outerCone = cosf(outerAngle * 3.14159265f / 180.0f);
				}
				
				// 減衰パラメータ
				ImGui::Text("Attenuation");
				ImGui::SliderFloat("Constant", &spotLight_.attenuation.x, 0.0f, 2.0f);
				ImGui::SliderFloat("Linear", &spotLight_.attenuation.y, 0.0f, 0.5f);
				ImGui::SliderFloat("Quadratic", &spotLight_.attenuation.z, 0.0f, 0.1f);
			}
			
			// ライトを無効化する場合の処理
			static float dirLightIntensityBackup = 1.0f;
			static float spotLightIntensityBackup = 2.0f;
			
			if (!enableDirectionalLight_) {
				if (directionalLight_.intensity > 0.0f) {
					dirLightIntensityBackup = directionalLight_.intensity;
				}
				directionalLight_.intensity = 0.0f;
			} else if (directionalLight_.intensity == 0.0f) {
				directionalLight_.intensity = dirLightIntensityBackup;
			}
			
			if (!enableSpotLight_) {
				if (spotLight_.intensity > 0.0f) {
					spotLightIntensityBackup = spotLight_.intensity;
				}
				spotLight_.intensity = 0.0f;
			} else if (spotLight_.intensity == 0.0f) {
				spotLight_.intensity = spotLightIntensityBackup;
			}
			
			// 現在のライト値の表示
			ImGui::Separator();
			ImGui::Text("現在のライト値:");
			ImGui::Text("Directional Light:");
			ImGui::Text("  強度: %.2f", directionalLight_.intensity);
			ImGui::Text("  方向: (%.2f, %.2f, %.2f)", directionalLight_.direction.x, directionalLight_.direction.y, directionalLight_.direction.z);
			ImGui::Text("Spot Light:");
			ImGui::Text("  強度: %.2f", spotLight_.intensity);
			ImGui::Text("  位置: (%.2f, %.2f, %.2f)", spotLight_.position.x, spotLight_.position.y, spotLight_.position.z);
			
			ImGui::End();
		}
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
