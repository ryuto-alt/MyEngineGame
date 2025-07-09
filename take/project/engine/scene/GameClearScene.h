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
#include "3d/Particle/Particle3d.h"
#include "3d/Particle/ParticleCommon.h"
#include "base/Particle/ParticleManager.h"
#include "3d/Particle/ParticleEmitter.h"
#include "SkyBox/SkyBox.h"

class GameClearScene : public BaseScene {
public:
	enum Phase {
		FIRST,
		SECOND,
		FINAL,
	};
	//初期化
	void Initialize() override;
	//終了処理
	void Finalize() override;
	//更新処理
	void Update() override;

	void UpdateImGui() override;
	//描画処理
	void Draw() override;

private:

	//サウンドデータ
	AudioManager::SoundData gameClearBGM;
	bool isSoundPlay = false;
	std::unique_ptr<Camera> gameClearCamera_ = nullptr;

	// 天
	std::unique_ptr<SkyBox> skybox_ = nullptr;

	//白飛びスプライト
	std::unique_ptr<Sprite> whiteOutSprite_ = nullptr;

	std::unique_ptr<Object3d> GameClearText_ = nullptr;

	float lerpTime_ = 0.0f;

	Phase phase_;

	bool changePhase_ = false;

};