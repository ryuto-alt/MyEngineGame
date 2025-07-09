#include "GameOverScene.h"
#include "SceneManager.h"
#include "TakeCFrameWork.h"
#include <cmath>
#include <algorithm>

void GameOverScene::Initialize() {

	// サウンドデータ
	//gameOverBGM = AudioManager::GetInstance()->SoundLoadWave("Resources/audioSources/gameOverBGM.wav");
	isSoundPlay = false;
	// GameCamera
	gameOverCamera_ = std::make_unique<Camera>();
	gameOverCamera_->Initialize(CameraManager::GetInstance()->GetDirectXCommon()->GetDevice());
	gameOverCamera_->SetTranslate({ 0.0f, 20.0f, -60.0f });
	gameOverCamera_->SetRotate({ 0.16f, 0.0f, 0.0f });
	CameraManager::GetInstance()->AddCamera("GameOverCamera", *gameOverCamera_);
	// デフォルトカメラの設定
	Object3dCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());
	ParticleCommon::GetInstance()->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());

	//DirectionalLightの輝度を設定
	Object3dCommon::GetInstance()->SetDirectionalLightIntensity(1.0f);
	//pointLightの輝度を設定
	Object3dCommon::GetInstance()->SetPointLightIntensity(0.0f);
	Object3dCommon::GetInstance()->SetPointLightPosition({ 0.0f,2.2f,5.0f });
	Object3dCommon::GetInstance()->SetPointLightColor({ 1.0f,0.8f,0.8f,1.0f });
	Object3dCommon::GetInstance()->SetPointLightRadius(0.0f);

	// Player
	//player_ = std::make_unique<Player>();
	//player_->Initialize(Object3dCommon::GetInstance(), "walk.gltf");

	//SkyBox
	skybox_ = std::make_unique<SkyBox>();
	skybox_->Initialize(Object3dCommon::GetInstance()->GetDirectXCommon(), "skyBox.obj");
	skybox_->SetMaterialColor({ 0.0f,0.0f,0.0f,0.0f });
	// ground

	GameOverText_ = std::make_unique<Object3d>();
	GameOverText_->Initialize(Object3dCommon::GetInstance(), "GameOverText.obj");
	GameOverText_->SetTranslate({ -15.0f, 0.0f, 0.0f });
	GameOverText_->SetRotation({ 3.14f, 0.0f, 0.0f });
	GameOverText_->SetScale({ 2.5f, 2.5f, 1.0f });
}

void GameOverScene::Finalize() {
	
	// サウンドデータの解放
	AudioManager::GetInstance()->SoundUnload(&gameOverBGM);
	// カメラの解放
	CameraManager::GetInstance()->ResetCameras();
}

void GameOverScene::Update() {

	if (!isSoundPlay) {
		//AudioManager::GetInstance()->SoundPlayWave(AudioManager::GetInstance()->GetXAudio2(), gameOverBGM);
		isSoundPlay = true;
	}
	
	// カメラの更新
	CameraManager::GetInstance()->Update();
	// 天球の更新
	skybox_->Update();

	// プレイヤーの更新
	//player_->Update();
	// 敵の更新
	
	GameOverText_->Update();

	switch (phase_) {
	case FIRST:

		lerpTime_ += 0.01f;
		Object3dCommon::GetInstance()->SetPointLightRadius(std::lerp(0.0f, 30.0f, lerpTime_));
		if (lerpTime_ >= 1.0f) {
			lerpTime_ = 0.0f;
			phase_ = Phase::SECOND;
		}
		break;
	case SECOND:

		// シーン遷移
		if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
			lerpTime_ = 0.0f;
			changePhase_ = true;
		}
		if (changePhase_) {
			lerpTime_ += 0.01f;
			Object3dCommon::GetInstance()->SetPointLightRadius(std::lerp(30.0f, 0.0f, lerpTime_));
			if (lerpTime_ >= 0.99f) {
				phase_ = Phase::FINAL;
			}
		
		}
		break;
	case FINAL:

		// シーン切り替え依頼
		AudioManager::GetInstance()->SoundUnload(&gameOverBGM);
		SceneManager::GetInstance()->ChangeScene("TITLE");
		break;
	default:
		break;
	}

	// パーティクルの更新
	TakeCFrameWork::GetParticleManager()->Update();

}

void GameOverScene::UpdateImGui() {
	ImGui::Begin("GameOverScene");
	CameraManager::GetInstance()->UpdateImGui();
	Object3dCommon::GetInstance()->UpdateImGui();	
	TakeCFrameWork::GetParticleManager()->UpdateImGui();
	ImGui::End();
}

void GameOverScene::Draw() {
	skybox_->Draw();

	SpriteCommon::GetInstance()->PreDraw(); // Spriteの描画前処理
	Object3dCommon::GetInstance()->PreDraw(); // Object3dの描画前処理
	GameOverText_->Draw();
	
	ParticleCommon::GetInstance()->PreDraw(); // パーティクルの描画前処理
	TakeCFrameWork::GetParticleManager()->Draw(); // パーティクルの描画
}
