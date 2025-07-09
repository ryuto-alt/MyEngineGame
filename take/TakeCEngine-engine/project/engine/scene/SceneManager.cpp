#include "SceneManager.h"
#include "base/ImGuiManager.h"
#include "base//ModelManager.h"
#include "base/TakeCFrameWork.h"
#include "3d/Object3dCommon.h"
#include <cassert>

SceneManager* SceneManager::instance_ = nullptr;

//========================================================================
//	インスタンス取得
//========================================================================

SceneManager* SceneManager::GetInstance() {
	
	if (instance_ == nullptr) {
		instance_ = new SceneManager();
	}
	return instance_;
}

//========================================================================
//	終了処理
//========================================================================

void SceneManager::Finalize() {
	currentScene_->Finalize();
	currentScene_.reset();
	instance_ = nullptr;
}

//========================================================================
//	更新処理
//========================================================================

void SceneManager::Update() {
	// シーン切り替え
	if (nextScene_) {
		ChangeToNextScene();
	}

	// 実行中のシーンの更新
	if (currentScene_) {
		currentScene_->Update();
	}
}

void SceneManager::UpdateImGui() {
#ifdef _DEBUG

	//コンボボックスの項目
	std::vector<std::string> items = { "GAMEPLAY","TITLE", "GAMEOVER","GAMECLEAR","PARTICLEEDITOR"};
	//現在の項目
	std::string& currentItem = items[itemCurrentIdx];
	//変更があったかどうか
	bool changed = false;

	//コンボボックスの表示
	ImGui::Begin("SceneManager");
	ImGui::Text("currentScene : %s", currentItem.c_str());
	if (ImGui::BeginCombo("changeScene", currentItem.c_str())) {
		for (int n = 0; n < items.size(); n++) {
			const bool is_selected = (currentItem == items[n]);
			if (ImGui::Selectable(items[n].c_str(), is_selected)) {
				currentItem = items[itemCurrentIdx];
				itemCurrentIdx = n;
				changed = true;
			}
			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected) {
				ImGui::SetItemDefaultFocus();
			}
		}

		currentItem = items[itemCurrentIdx];
		ImGui::EndCombo();
	}
	currentScene_->UpdateImGui();
	ImGui::End();

	//変更があった場合
	if (changed) {
		switch (itemCurrentIdx) {
		case 0:
			nextScene_ = sceneFactory_->CreateScene("GAMEPLAY");
			break;
		case 1:
			nextScene_ = sceneFactory_->CreateScene("TITLE");
			break;
		case 2:
			nextScene_ = sceneFactory_->CreateScene("GAMEOVER");
			break;
		case 3:
			nextScene_ = sceneFactory_->CreateScene("GAMECLEAR");
			break;
		case 4:
			nextScene_ = sceneFactory_->CreateScene("PARTICLEEDITOR");
			break;
		}
	}
#endif // _DEBUG
}

//================================================================
//	次のシーンに切り替え
//================================================================

void SceneManager::ChangeToNextScene() {
	// 旧シーンの終了
	if (currentScene_) {
		currentScene_->Finalize();
		currentScene_.reset();
	}

	// 次のシーンに切り替え
	currentScene_ = nextScene_;
	nextScene_ = nullptr;

	// シーンマネージャーのセット
	currentScene_->SetSceneManager(this);

	// 次のシーンの初期化
	currentScene_->Initialize();
}

void SceneManager::LoadLevelData(const std::string& sceneName) {

	levelData_ = TakeCFrameWork::GetJsonLoader()->LoadLevelFile(sceneName);

	for (auto& objectData : levelData_->objects) {
		Model* model = nullptr;
		model = ModelManager::GetInstance()->FindModel(objectData.file_name);

		std::unique_ptr<Object3d> newObject = std::make_unique<Object3d>();
		newObject->Initialize(Object3dCommon::GetInstance(), objectData.file_name);

		newObject->SetTranslate(objectData.translation);
		newObject->SetRotation(objectData.rotation);
		newObject->SetScale(objectData.scale);

		levelObjects_.push_back(std::move(newObject));
	}
}

//========================================================================
//	描画処理
//========================================================================

void SceneManager::Draw() {
	currentScene_->Draw();
}

//========================================================================
//	シーンの変更
//========================================================================

void SceneManager::ChangeScene(const std::string& sceneName) {
	assert(sceneFactory_ != nullptr);
	assert(nextScene_ == nullptr);

	// 既存のオブジェクトをクリア
	levelObjects_.clear();
	// レベルデータの読み込み
	LoadLevelData("levelData_gameScene");
	// 新しいシーンを作成
	nextScene_ = sceneFactory_->CreateScene(sceneName);
}
