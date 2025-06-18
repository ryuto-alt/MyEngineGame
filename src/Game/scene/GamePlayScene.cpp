#include "GamePlayScene.h"
#include "Vector3.h"

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

	// カメラの初期設定（統合APIを使用）
	engine_->SetCameraPosition(Vector3{ 0.0f, 2.0f, -8.0f });
	engine_->SetCameraFovY(1.37f); // 90度の視野角を設定

	// ゲームリソースの読み込み（統合APIを使用）
	// BGMの読み込み（失敗してもエラーにしない）
	if (!engine_->LoadAudio("bgm", "Resources/Audio/bgm.mp3")) {
		OutputDebugStringA("警告: BGMファイルの読み込みに失敗しました。BGM機能は無効になります。\n");
	}
	engine_->CreateParticleEffect("explosion", "Resources/particle/explosion.png");

	// 3Dオブジェクトの作成
	cubeObject_ = engine_->CreateObject3D();
	
	// 複数のモデルを読み込む（domeは別途読み込むので除外）
	modelNames_ = {"cube.glb", "delta.glb", "jett.glb"};
	
	for (const auto& modelName : modelNames_) {
		std::string fullPath = "Resources/Models/cube/" + modelName;
		auto model = engine_->LoadModel(fullPath);
		
		if (model) {
			OutputDebugStringA(("GamePlayScene: " + modelName + " loaded successfully\n").c_str());
			OutputDebugStringA(("  Vertex count: " + std::to_string(model->GetVertexCount()) + "\n").c_str());
			models_.push_back(std::move(model));
		} else {
			OutputDebugStringA(("ERROR: Failed to load " + modelName + "\n").c_str());
			// 空のモデルを追加してインデックスを保持
			models_.push_back(nullptr);
		}
	}
	
	// 最初のモデルを設定
	if (!models_.empty() && models_[0]) {
		cubeObject_->SetModel(models_[0].get());
	}

	// キューブの初期位置を設定
	cubePosition_ = Vector3{ 0.0f, 0.0f, 0.0f };
	cubeObject_->SetPosition(cubePosition_);
	
	// キューブのスケールを大きくして見やすくする
	cubeObject_->SetScale(Vector3{ 2.0f, 2.0f, 2.0f });
	
	// ディレクショナルライトの設定（デフォルト）
	DirectionalLight defaultLight;
	defaultLight.color = { 1.0f, 1.0f, 1.0f, 1.0f };  // 白色光
	defaultLight.direction = { 0.0f, -1.0f, 0.0f };   // 上から下へ
	defaultLight.intensity = 1.0f;  // 標準の強さ
	cubeObject_->SetDirectionalLight(defaultLight);
	
	// domeモデルの設定
	domeObject_ = engine_->CreateObject3D();
	domeModel_ = engine_->LoadModel("Resources/Models/cube/dome.glb");
	if (domeModel_) {
		OutputDebugStringA("GamePlayScene: dome.glb loaded successfully\n");
		OutputDebugStringA(("  Dome vertex count: " + std::to_string(domeModel_->GetVertexCount()) + "\n").c_str());
		domeObject_->SetModel(domeModel_.get());
		// domeの位置とスケールを設定
		domeObject_->SetPosition(Vector3{ 0.0f, 0.0f, 0.0f });
		domeObject_->SetScale(Vector3{ 10.0f, 10.0f, 10.0f });  // 大きくして空のように
		domeObject_->SetDirectionalLight(defaultLight);
	} else {
		OutputDebugStringA("ERROR: Failed to load dome.glb\n");
	}

	// 2Dスプライトの作成
	titleSprite_ = engine_->CreateSprite("Resources/textures/title_logo.png");
	titleSprite_->SetPosition({ 100.0f, 50.0f });
	titleSprite_->SetSize({ 200.0f, 100.0f });

	// 3D空間オーディオの作成（キューブの位置に音源を配置）
	if (engine_->LoadAudio("cube_bgm", "Resources/Audio/bgm.mp3")) {
		cubeSpatialAudio_ = engine_->CreateSpatialAudioSource("cube_bgm", cubePosition_);
		if (cubeSpatialAudio_) {
			cubeSpatialAudio_->SetMaxDistance(20.0f);  // 20ユニット以内で聞こえる
			cubeSpatialAudio_->SetMinDistance(2.0f);   // 2ユニット以内では減衰なし
			cubeSpatialAudio_->SetVolume(0.7f);        // 音量70%
		}
	}

	// 初期化完了
	initialized_ = true;
}

void GamePlayScene::Update() {
	if (!initialized_) return;

	// ESCキーでタイトルシーンに戻る（統合APIを使用）
	if (engine_->IsKeyTriggered(DIK_ESCAPE)) {
		engine_->ChangeScene("Title");
	}

	// モデルの切り替え（数字キー1、4）
	if (engine_->IsKeyTriggered(DIK_1) && models_.size() > 0 && models_[0]) {
		currentModelIndex_ = 0;
		cubeObject_->SetModel(models_[0].get());
		OutputDebugStringA(("Switched to model: " + modelNames_[0] + "\n").c_str());
	}
	if (engine_->IsKeyTriggered(DIK_2) && models_.size() > 1 && models_[1]) {
		currentModelIndex_ = 1;
		cubeObject_->SetModel(models_[1].get());
		OutputDebugStringA(("Switched to model: " + modelNames_[1] + "\n").c_str());
	}
	if (engine_->IsKeyTriggered(DIK_3) && models_.size() > 2 && models_[2]) {
		currentModelIndex_ = 2;
		cubeObject_->SetModel(models_[2].get());
		OutputDebugStringA(("Switched to model: " + modelNames_[2] + "\n").c_str());
	}

	// パーティクル発生（統合APIを使用）
	if (engine_->IsKeyTriggered(DIK_F)) {
		engine_->PlayParticle("explosion", Vector3{ 0.0f, 2.0f, 0.0f }, 50);
	}

	// BGM制御（統合APIを使用）
	if (engine_->IsKeyTriggered(DIK_B)) {
		try {
			if (engine_->IsAudioPlaying("bgm")) {
				engine_->StopAudio("bgm");
			}
			else {
				engine_->PlayAudio("bgm", true, 0.3f);
			}
		}
		catch (...) {
			// BGMの操作でエラーが発生しても続行
		}
	}

	// TABキーでカメラモード切り替え
	if (engine_->IsKeyTriggered(DIK_TAB)) {
		engine_->ToggleCameraMode();

		// カメラモードに応じてマウスカーソルを制御
		if (engine_->GetCameraMode() == 0) {
			// フリーカメラモード: マウスカーソルを非表示で中央固定
			engine_->SetMouseCursor(false);
			engine_->ResetMouseCenter();
		}
		else {
			// 固定カメラモード: マウスカーソルを表示
			engine_->SetMouseCursor(true);
		}
	}

	// マウス視点移動（フリーカメラモードのみ）
	if (engine_->GetCameraMode() == 0) {
		float mouseX, mouseY;
		engine_->GetMouseMovement(mouseX, mouseY);
		engine_->ProcessCameraMouseInput(mouseX, mouseY);

		// マウスを中央にリセット（連続的な視点移動のため）
		engine_->ResetMouseCenter();
	}

	// カメラのWASD移動（カメラモードに応じて切り替え）
	const float moveSpeed = 0.1f;

	if (engine_->GetCameraMode() == 0) {
		// フリーカメラモード: 水平移動のみ、上下はSHIFT/SPACE
		Vector3 currentPos = engine_->GetCameraPosition();
		Vector3 forward = engine_->GetCameraForwardVector();
		Vector3 right = engine_->GetCameraRightVector();
		
		// 水平移動のみ（Y軸を固定）
		if (engine_->IsKeyPressed(DIK_W)) {
			currentPos.x += forward.x * moveSpeed;
			currentPos.z += forward.z * moveSpeed;
		}
		if (engine_->IsKeyPressed(DIK_S)) {
			currentPos.x -= forward.x * moveSpeed;
			currentPos.z -= forward.z * moveSpeed;
		}
		if (engine_->IsKeyPressed(DIK_A)) {
			currentPos.x -= right.x * moveSpeed;
			currentPos.z -= right.z * moveSpeed;
		}
		if (engine_->IsKeyPressed(DIK_D)) {
			currentPos.x += right.x * moveSpeed;
			currentPos.z += right.z * moveSpeed;
		}
		
		// 上下移動はSHIFTとSPACEのみ
		if (engine_->IsKeyPressed(DIK_SPACE)) currentPos.y += moveSpeed;  // 上昇
		if (engine_->IsKeyPressed(DIK_LSHIFT) || engine_->IsKeyPressed(DIK_RSHIFT)) currentPos.y -= moveSpeed; // 下降
		
		engine_->SetCameraPosition(currentPos);
	}
	else {
		// 固定カメラモード: ワールド座標軸で移動（従来通り）
		Vector3 currentPos = engine_->GetCameraPosition();

		if (engine_->IsKeyPressed(DIK_W)) currentPos.z += moveSpeed;
		if (engine_->IsKeyPressed(DIK_S)) currentPos.z -= moveSpeed;
		if (engine_->IsKeyPressed(DIK_A)) currentPos.x -= moveSpeed;
		if (engine_->IsKeyPressed(DIK_D)) currentPos.x += moveSpeed;

		engine_->SetCameraPosition(currentPos);
	}

	// 十字キーでキューブ移動
	const float cubeSpeed = 0.05f;

	if (engine_->IsKeyPressed(DIK_UP)) cubePosition_.z += cubeSpeed;
	if (engine_->IsKeyPressed(DIK_DOWN)) cubePosition_.z -= cubeSpeed;
	if (engine_->IsKeyPressed(DIK_LEFT)) cubePosition_.x -= cubeSpeed;
	if (engine_->IsKeyPressed(DIK_RIGHT)) cubePosition_.x += cubeSpeed;

	// キューブの位置を更新
	cubeObject_->SetPosition(cubePosition_);

	// 3D空間オーディオリスナーの位置と向きをカメラと同期
	Vector3 cameraPos = engine_->GetCameraPosition();
	Vector3 cameraForward = engine_->GetCameraForwardVector();
	Vector3 cameraUp = engine_->GetCameraUpVector();

	// デバッグ情報表示
	static int debugCounter = 0;
	if (debugCounter++ % 60 == 0) {  // 1秒に1回表示
		char debugMsg[256];
		sprintf_s(debugMsg, "Camera Forward: (%.2f, %.2f, %.2f)",
			cameraForward.x, cameraForward.y, cameraForward.z);
		OutputDebugStringA(debugMsg);
	}

	engine_->SetAudioListenerPosition(cameraPos);
	engine_->SetAudioListenerOrientation(cameraForward, cameraUp);

	// 3D空間オーディオソースの操作
	if (cubeSpatialAudio_) {
		// Cキーで3D音源再生/停止
		if (engine_->IsKeyTriggered(DIK_C)) {
			if (cubeSpatialAudio_->IsPlaying()) {
				cubeSpatialAudio_->Stop();
			}
			else {
				cubeSpatialAudio_->Play(true); // ループ再生
			}
		}

		// キューブの位置に音源を更新
		cubeSpatialAudio_->SetPosition(cubePosition_);

		// 3D空間オーディオを更新（カメラの向きを反映）
		cubeSpatialAudio_->Update(cameraPos, cameraForward);
	}
	else {
		// 3D空間オーディオが無効な場合は直接再生
		if (engine_->IsKeyTriggered(DIK_C)) {
			if (engine_->IsAudioPlaying("cube_bgm")) {
				engine_->StopAudio("cube_bgm");
			}
			else {
				engine_->PlayAudio("cube_bgm", true, 2.5f);
			}
		}
	}

	// 3D空間オーディオシステム全体を更新
	engine_->UpdateSpatialAudio();

	// カメラの更新（先に更新する）
	camera_->Update();
	
	// ライティングモードに応じてマテリアルのライティングフラグを設定
	if (lightingMode_ == LIGHTING_NONE) {
		cubeObject_->SetEnableLighting(false);
		if (domeObject_) domeObject_->SetEnableLighting(false);
	} else {
		cubeObject_->SetEnableLighting(true);
		if (domeObject_) domeObject_->SetEnableLighting(true);
		// enableLightingにモードを設定
		cubeObject_->GetMaterialData()->enableLighting = lightingMode_;
		if (domeObject_) domeObject_->GetMaterialData()->enableLighting = lightingMode_;
	}
	
	// スポットライトをカメラの位置と向きに同期（懐中電灯効果）
	SpotLight spotLight;
	spotLight.position = engine_->GetCameraPosition();
	spotLight.range = 20.0f;  // 懐中電灯の有効範囲
	spotLight.direction = engine_->GetCameraForwardVector();  // カメラの前方向
	spotLight.innerCone = 0.95f;  // cos(18度) - 中心部の完全な明るさ
	spotLight.color = { 1.0f, 0.95f, 0.8f, 1.0f };  // 暖かい白色光（懐中電灯風）
	spotLight.outerCone = 0.82f;  // cos(35度) - 外側の減衰開始角度
	spotLight.intensity = 3.0f;  // 光の強度
	spotLight.attenuation = { 0.09f, 0.032f };  // 線形と1/r^2の減衰係数
	
	// スポットライトを設定
	cubeObject_->SetSpotLight(spotLight);
	if (domeObject_) domeObject_->SetSpotLight(spotLight);
	
	// オブジェクトの更新
	cubeObject_->Update();
	if (domeObject_) domeObject_->Update();
	titleSprite_->Update();
}

void GamePlayScene::Draw() {
	if (!initialized_) return;

	// 3Dオブジェクトの描画
	// domeを先に描画（背景として）
	if (domeObject_) domeObject_->Draw();
	// 切り替え可能なモデルを描画
	cubeObject_->Draw();

	// 2Dスプライトの描画
	titleSprite_->Draw();

	// GamePlayScene用のImGuiウィンドウ
	ImGui::Begin("GamePlayScene - 統合API版");

	ImGui::Text("統合APIを使用したGamePlaySceneです");
	ImGui::Separator();
	
	// ライティングモードの選択
	ImGui::Text("ライティングモード:");
	ImGui::RadioButton("ライティングなし", &lightingMode_, LIGHTING_NONE);
	ImGui::RadioButton("ディレクショナルライトのみ", &lightingMode_, LIGHTING_DIRECTIONAL);
	ImGui::RadioButton("スポットライトのみ（ホラーゲーム風）", &lightingMode_, LIGHTING_SPOTLIGHT);
	ImGui::RadioButton("両方のライト", &lightingMode_, LIGHTING_BOTH);
	ImGui::Separator();

	// モデル情報
	ImGui::Text("モデル情報:");
	ImGui::Text("現在のモデル: %s", 
		(currentModelIndex_ < modelNames_.size()) ? modelNames_[currentModelIndex_].c_str() : "None");
	if (currentModelIndex_ < models_.size() && models_[currentModelIndex_]) {
		ImGui::Text("頂点数: %zu", models_[currentModelIndex_]->GetVertexCount());
	}
	
	// 利用可能なモデルのリスト
	ImGui::Text("");
	ImGui::Text("利用可能なモデル:");
	for (size_t i = 0; i < modelNames_.size(); ++i) {
		bool isLoaded = (i < models_.size() && models_[i]);
		ImGui::Text("%zu. %s %s", i + 1, modelNames_[i].c_str(), 
			isLoaded ? "[読み込み成功]" : "[読み込み失敗]");
	}
	ImGui::Separator();
	
	ImGui::Text("操作方法:");
	ImGui::Text("1-3 - モデル切り替え");
	ImGui::Text("TAB - カメラモード切り替え");
	ImGui::Text("F - パーティクル発生");
	ImGui::Text("B - BGM ON/OFF");
	ImGui::Text("C - bgm.wav再生/停止（3D空間オーディオ）");
	ImGui::Text("WASD/SPACE/SHIFT - カメラ移動");
	ImGui::Text("↑↓←→ - モデル移動");
	ImGui::Text("ESC - タイトルに戻る");

	ImGui::Separator();

	// 衝突判定テスト
	Vector3 playerPos = Vector3{ 0.0f, 0.0f, 0.0f };
	Vector3 enemyPos = Vector3{ 1.0f, 0.0f, 0.0f };
	bool collision = engine_->CheckCollision(playerPos, 0.5f, enemyPos, 0.5f);
	ImGui::Text("衝突判定テスト: %s", collision ? "衝突中" : "衝突していません");

	ImGui::Separator();

	// 3D空間オーディオ情報
	ImGui::Text("3D空間オーディオ:");
	ImGui::Text("cubeSpatialAudio_: %s", cubeSpatialAudio_ ? "有効" : "NULL");

	if (cubeSpatialAudio_) {
		Vector3 cubePos = cubeSpatialAudio_->GetPosition();
		Vector3 cameraPos = engine_->GetCameraPosition();
		float distance = cubeSpatialAudio_->GetDistanceToListener();

		ImGui::Text("キューブ音源位置: (%.1f, %.1f, %.1f)", cubePosition_.x, cubePosition_.y, cubePosition_.z);
		ImGui::Text("カメラ位置: (%.1f, %.1f, %.1f)", cameraPos.x, cameraPos.y, cameraPos.z);
		ImGui::Text("リスナー距離: %.2f", distance);
		ImGui::Text("bgm.wav再生中: %s", cubeSpatialAudio_->IsPlaying() ? "Yes" : "No");
		ImGui::Text("最大距離: %.1f, 最小距離: %.1f", 20.0f, 2.0f);
		ImGui::Text("カメラとキューブの距離が近いほどbgm.wavが大きく聞こえます");
		ImGui::Text("十字キーでキューブを移動して音の位置変化をテスト");
	}
	else {
		ImGui::Text("3D音源なし - 初期化失敗の可能性");
		ImGui::Text("音楽ファイルの読み込みまたは音源作成に失敗");
	}

	ImGui::End();

	// デバッグ情報の表示（統合APIを使用）
	engine_->ShowDebugInfo();
}

void GamePlayScene::Finalize() {
	// 3D空間オーディオの停止と解放
	if (cubeSpatialAudio_) {
		cubeSpatialAudio_->Stop();
		cubeSpatialAudio_.reset();
	}

	cubeObject_.reset();
	cubeModel_.reset();
	titleSprite_.reset();
}
