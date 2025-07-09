#pragma once
#include "BaseScene.h"

#include "Audio.h"
#include "Input.h"
#include "Camera.h"
#include "CameraManager.h"
#include "ModelManager.h"
#include "SkyBox/SkyBox.h"
#include "Sprite.h"
#include "Object3d.h"
#include "Object3dCommon.h"
#include "SpriteCommon.h"
#include "3d/Particle/Particle3d.h"
#include "3d/Particle/ParticleCommon.h"
#include "3d/Particle/GPUParticle.h"
#include "base/Particle/ParticleManager.h"
#include "3d/Particle/ParticleEmitter.h"

#include "Quaternion.h"

//app
#include "application/Ground/Ground.h"
#include "application/HPBar/HPBar.h"
#include "application/Entity/SampleCharacter.h"
#include "application/Entity/Player/Player.h"
#include "application/Entity/Enemy/Enemy.h"


class GamePlayScene : public BaseScene {
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

	void CheckAllCollisions();

private:

	//サウンドデータ
	//AudioManager::SoundData BGM;
	bool isSoundPlay = false;
	// カメラ
	std::shared_ptr<Camera> gameCamera_ = nullptr;
	std::shared_ptr<Camera> debugCamera_ = nullptr;
	//SkyBox
	std::unique_ptr<SkyBox> skyBox_ = nullptr;

	//スプライト
	std::unique_ptr<Sprite> sprite_ = nullptr;


	//player
	std::unique_ptr<Player> player_ = nullptr;
	//enemy
	std::unique_ptr<Enemy> enemy_ = nullptr;

	std::unique_ptr<BulletManager> bulletManager_ = nullptr;

	std::vector<std::unique_ptr<Object3d>> levelObjects_;
};