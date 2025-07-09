#include "GameClearScene.h"
#include "SceneManager.h"
#include "TakeCFrameWork.h"
#include <cmath>
#include <algorithm>

void GameClearScene::Initialize() {

	// サウンドデータ
	//gameClearBGM = AudioManager::GetInstance()->SoundLoadWave("Resources/audioSources/gameClearBGM.wav");
	isSoundPlay = false;
	// GameCamera
	gameClearCamera_ = std::make_unique<Camera>();
	gameClearCamera_->Initialize(CameraManager::GetInstance()->GetDirectXCommon()->GetDevice());
	gameClearCamera_->SetTranslate({ 0.0f, 20.0f, -60.0f });
	gameClearCamera_->SetRotate({ 0.16f, 0.0f, 0.0f });
	CameraManager::GetInstance()->AddCamera("GameOverCamera", *gameClearCamera_);
	// デフォルトカメラの設定
	Object3dCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());
	ParticleCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());

	//DirectionalLightの輝度を設定
	Object3dCommon::GetInstance()->SetDirectionalLightIntensity(0.5f);
	//pointLightの輝度を設定
	Object3dCommon::GetInstance()->SetPointLightIntensity(2.0f);
	Object3dCommon::GetInstance()->SetPointLightPosition({ 0.0f,2.2f,5.0f });
	Object3dCommon::GetInstance()->SetPointLightColor({ 1.0f,0.8f,0.8f,1.0f });
	Object3dCommon::GetInstance()->SetPointLightRadius(0.0f);

	// Player
	//player_ = std::make_unique<Player>();
	//player_->Initialize(Object3dCommon::GetInstance(), "walk.gltf");

	//SkyBox
	skybox_ = std::make_unique<SkyBox>();
	skybox_->Initialize(Object3dCommon::GetInstance()->GetDirectXCommon(), "skyBox.obj");
	skybox_->SetMaterialColor({ 0.0f,0.0f,0.0f,1.0f });

	//GameClearText
	GameClearText_ = std::make_unique<Object3d>();
	GameClearText_->Initialize(Object3dCommon::GetInstance(), "GameClearText.obj");
	GameClearText_->SetTranslate({ 8.0f, 0.0f, 10.0f });
	GameClearText_->SetRotation({ 0.0f, 3.2f, 0.0f });
	GameClearText_->SetScale({ 2.5f, 2.5f, 1.0f });

	//whiteOutSprite
	whiteOutSprite_ = std::make_unique<Sprite>();
	whiteOutSprite_->Initialize(SpriteCommon::GetInstance(), "backHp.png");
	whiteOutSprite_->SetPosition({ 640.0f, 360.0f });
	whiteOutSprite_->SetSize({ 0.0f, 0.0f });
	whiteOutSprite_->SetAnchorPoint({ 0.5f, 0.5f });

}

void GameClearScene::Finalize() {
	// サウンドデータの解放
	AudioManager::GetInstance()->SoundUnload(&gameClearBGM);
	// カメラの解放
	CameraManager::GetInstance()->ResetCameras();
}

void GameClearScene::Update() {

	// オーディオ再生
	if (!isSoundPlay) {
		//AudioManager::GetInstance()->SoundPlayWave(AudioManager::GetInstance()->GetXAudio2(), gameClearBGM);
		isSoundPlay = true;
	}
	// カメラの更新
	CameraManager::GetInstance()->Update();
	// 天球の更新
	skybox_->Update();

	// プレイヤーの更新
	//player_->Update();
	//クリアテキストの更新
	GameClearText_->Update();
	
	whiteOutSprite_->Update();
	// パーティクルの更新
	TakeCFrameWork::GetParticleManager()->Update();

	switch (phase_) {
	case FIRST:

		lerpTime_ += 0.05f;
		Object3dCommon::GetInstance()->SetPointLightRadius(std::lerp(0.0f, 200.0f, lerpTime_));
		if (lerpTime_ >= 1.0f) {
			lerpTime_ = 0.0f;
			phase_ = Phase::SECOND;
		}
		break;
	case SECOND:

		// シーン遷移
		if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
			
			changePhase_ = true;
		}
		if (changePhase_) {
			lerpTime_ += 0.01f;
			whiteOutSprite_->SetSize({ std::lerp(0.0f, 1280.0f, lerpTime_), std::lerp(0.0f, 720.0f, lerpTime_) });
			if (lerpTime_ >= 0.99f) {
				phase_ = Phase::FINAL;
			}

		}
		break;
	case FINAL:

		// シーン切り替え依頼
		AudioManager::GetInstance()->SoundUnload(&gameClearBGM);
		SceneManager::GetInstance()->ChangeScene("TITLE");
		break;
	default:
		break;
	}
}

void GameClearScene::UpdateImGui() {
	ImGui::Begin("GameClearScene");
	CameraManager::GetInstance()->UpdateImGui();
	Object3dCommon::GetInstance()->UpdateImGui();
	TakeCFrameWork::GetParticleManager()->UpdateImGui();
	ImGui::End();
}

void GameClearScene::Draw() {

	skybox_->Draw(); // 天球の描画

	SpriteCommon::GetInstance()->PreDraw(); // Spriteの描画前処理
	whiteOutSprite_->Draw();
	Object3dCommon::GetInstance()->PreDraw();   //Object3dの描画前処理
	GameClearText_->Draw();
	
	ParticleCommon::GetInstance()->PreDraw(); // パーティクルの描画前処理
	TakeCFrameWork::GetParticleManager()->Draw(); // パーティクルの描画
}
