#pragma once
#include "BaseScene.h"
#include "AbstractSceneFactory.h"

class SceneManager
{
public:

	static SceneManager* GetInstance();
	//シーンの設定

	//現在のシーンを取得
	void SetNextScene(BaseScene* nextScene) { this ->nextScene = nextScene; };
	//シーンの更新
	void Update();
	//シーンの描画
	void Draw();
	//シーンの終了
	void Finalize();

	//sceneFactoryの設定
	void SetSceneFactory(AbstractSceneFactory* sceneFactory) { this->sceneFactory = sceneFactory; }

	void ChangeScene(const std::string &sceneName);
	

private:

	SceneManager() = default;
	~SceneManager() = default;
	SceneManager(SceneManager&) = delete;
	SceneManager& operator=(SceneManager&) = delete;

	

private:
	static SceneManager* instance_;
	BaseScene* currentScene = nullptr;
	BaseScene* nextScene = nullptr;
	AbstractSceneFactory* sceneFactory = nullptr;

};

