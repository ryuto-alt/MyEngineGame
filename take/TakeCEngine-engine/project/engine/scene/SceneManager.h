#pragma once
#include <memory>
#include "BaseScene.h"
#include "AbstractSceneFactory.h"
#include "scene//LevelData.h"

////////////////////
///	シーン管理クラス
////////////////////
class SceneManager {
public:

	static SceneManager* GetInstance();

	void Finalize();

	void Update();

	void UpdateImGui();

	void Draw();

	void ChangeScene(const std::string& sceneName);

	void ChangeToNextScene();

	void LoadLevelData(const std::string& sceneName);

	//========================================================================
	//	アクセッサ
	//========================================================================

	std::vector<std::unique_ptr<Object3d>>& GetLevelObjects() { return levelObjects_; }

	void SetSceneFactory(AbstractSceneFactory* sceneFactory) { sceneFactory_ = sceneFactory; }

private:

	SceneManager() = default;
	~SceneManager() = default;
	SceneManager(SceneManager&) = delete;
	SceneManager& operator=(SceneManager&) = delete;

private:

	static SceneManager* instance_;
	std::shared_ptr<BaseScene> currentScene_ = nullptr;
	std::shared_ptr<BaseScene> nextScene_ = nullptr;
	//シーンファクトリー(借りてくる)
	AbstractSceneFactory* sceneFactory_ = nullptr;

	//ImGuiCombo用インデックス
	uint32_t itemCurrentIdx = 0;
	// レベルデータの格納
	LevelData* levelData_; 
	// レベル内のオブジェクトのリスト
	std::vector<std::unique_ptr<Object3d>> levelObjects_;
};