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

	// パーティクルと音声の読み込みを削除

	// 3Dオブジェクトの作成
	cubeObject_ = engine_->CreateObject3D();
	// GLBファイルを読み込む
	cubeModel_ = engine_->LoadModel("Resources/Models/cube/cube.glb");
	if (cubeModel_) {
		OutputDebugStringA("GamePlayScene: cube.glb loaded successfully\n");
		cubeObject_->SetModel(cubeModel_.get());
		
		// デバッグ用：モデルの頂点数を表示
		OutputDebugStringA(("GamePlayScene: Model vertex count: " + std::to_string(cubeModel_->GetVertexCount()) + "\n").c_str());
	} else {
		OutputDebugStringA("ERROR: Failed to load cube.glb\n");
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

	// 2Dスプライトの作成を削除

	// 3D空間オーディオの作成を削除

	// 初期化完了
	initialized_ = true;
}

void GamePlayScene::Update() {
	if (!initialized_) return;

	// ESCキーでタイトルシーンに戻る（統合APIを使用）
	if (engine_->IsKeyTriggered(DIK_ESCAPE)) {
		engine_->ChangeScene("Title");
	}

	// キューブの回転機能を削除しました

	// パーティクルとBGM制御を削除

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

	// 3D空間オーディオの操作を削除

	// カメラの更新（先に更新する）
	camera_->Update();
	
	// ライティングモードに応じてマテリアルのライティングフラグを設定
	if (lightingMode_ == LIGHTING_NONE) {
		cubeObject_->SetEnableLighting(false);
	} else {
		cubeObject_->SetEnableLighting(true);
		// enableLightingにモードを設定
		cubeObject_->GetMaterialData()->enableLighting = lightingMode_;
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
	
	// オブジェクトの更新
	cubeObject_->Update();
	// スプライトの更新を削除
}

void GamePlayScene::Draw() {
	if (!initialized_) return;

	// 3Dオブジェクトの描画
	cubeObject_->Draw();

	// 2Dスプライトの描画を削除

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

	ImGui::Text("操作方法:");
	ImGui::Text("TAB - カメラモード切り替え");
	ImGui::Text("WASD - カメラ移動");
	ImGui::Text("↑↓←→ - キューブ移動");
	ImGui::Text("ESC - タイトルに戻る");

	ImGui::Separator();

	// 衝突判定テスト
	Vector3 playerPos = Vector3{ 0.0f, 0.0f, 0.0f };
	Vector3 enemyPos = Vector3{ 1.0f, 0.0f, 0.0f };
	bool collision = engine_->CheckCollision(playerPos, 0.5f, enemyPos, 0.5f);
	ImGui::Text("衝突判定テスト: %s", collision ? "衝突中" : "衝突していません");

	ImGui::Separator();

	// 3D空間オーディオ情報を削除

	ImGui::End();

	// デバッグ情報の表示（統合APIを使用）
	engine_->ShowDebugInfo();
}

void GamePlayScene::Finalize() {
	cubeObject_.reset();
	cubeModel_.reset();
}
