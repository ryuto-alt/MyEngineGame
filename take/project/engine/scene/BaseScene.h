#pragma once
#include "LevelData.h"
class SceneManager;
class BaseScene {
public:
	
	virtual ~BaseScene() = default;

	virtual void Initialize() = 0;

	virtual void Finalize() = 0;

	virtual void Update() = 0;

	virtual void UpdateImGui() = 0;

	virtual void Draw() = 0;

	virtual void SetSceneManager(SceneManager* sceneManager) {sceneManager_ = sceneManager; }

protected:

	//シーンマネージャー(このクラスで解放しないこと)
	SceneManager* sceneManager_ = nullptr;
};

