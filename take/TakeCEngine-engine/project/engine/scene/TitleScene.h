#pragma once
#include "BaseScene.h"

#include "Audio.h"
#include "Camera.h"
#include "CameraManager.h"
#include "Input.h"
#include "ModelManager.h"
#include "Sprite.h"
#include "Object3d.h"
#include "Object3dCommon.h"
#include "SpriteCommon.h"

class TitleScene : public BaseScene {
public:


	//初期化
	void Initialize() override;

	//終了処理
	void Finalize() override;

	//更新処理
	void Update() override;

	//ImGuiの更新処理
	void UpdateImGui() override;

	//描画処理
	void Draw() override;

private:


	//サウンドデータ
	//AudioManager::SoundData soundData1;
	// カメラ
	std::shared_ptr<Camera> camera0_ = nullptr;
	std::shared_ptr<Camera> camera1_ = nullptr;
	//スプライト
	std::shared_ptr<Sprite> sprite_ = nullptr;
	std::shared_ptr<Object3d> titleObject = nullptr;
	std::unique_ptr<Object3d> startObject = nullptr;

};

