#pragma once

class SceneManager;
class BaseScene
{
public:
	//すべて純粋仮想関数として宣言する


	//ゲームの初期化
	virtual void Initialize() = 0;
	//終了
	virtual void Finalize() = 0;
	//更新
	virtual void Update() = 0;
	//描画
	virtual void Draw() = 0;
	//デストラクタ
	virtual ~BaseScene() = default;

	virtual void SetSceneManager(SceneManager* sceneManager) { sceneManager_ = sceneManager; }

private:
	SceneManager* sceneManager_ = nullptr;

};

