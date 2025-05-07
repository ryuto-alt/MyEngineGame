#include "SceneFactory.h"
#include "TitleScene.h"
#include "GamePlayScene.h"
#include <cassert>

std::unique_ptr<IScene> SceneFactory::CreateScene(const std::string& sceneName) {
    if (sceneName == "Title") {
        return std::make_unique<TitleScene>();
    }
    else if (sceneName == "GamePlay") {
        return std::make_unique<GamePlayScene>();
    }

    assert(0 && "指定されたシーンが存在しません");
    return nullptr;
}