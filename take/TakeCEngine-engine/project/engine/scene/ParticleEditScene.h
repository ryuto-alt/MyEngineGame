#pragma once
#include "BaseScene.h"
#include "camera/Camera.h"
#include "SkyBox/SkyBox.h"
#include "2d/Sprite.h"
#include "3d/Particle/ParticleCommon.h"
#include "3d/Particle/ParticleEditor.h"

class ParticleEditScene : public BaseScene {
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

	// カメラ
	std::shared_ptr<Camera> previewCamera_ = nullptr;
	//SkyBox
	std::unique_ptr<SkyBox> skyBox_ = nullptr;

	//ParticleEditor
	std::unique_ptr<ParticleEditor> particleEditor_ = nullptr;
};

