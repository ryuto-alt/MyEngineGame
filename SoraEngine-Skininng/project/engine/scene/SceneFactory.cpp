#include "SceneFactory.h"
#include "GamePlayScene.h"
#include "TitleScene.h"
#include "GameClearScene.h"
#include "GameOverScene.h"

BaseScene* SceneFactory::CreateScene(const std::string& sceneName)
{
	BaseScene* newscene = nullptr;

	if (sceneName == "GAMEPLAY") {
		newscene = new GamePlayScene();
	}
	else if (sceneName == "TITELE") {
		newscene = new TitleScene();
	}
	else if (sceneName == "GAMECLEAR") {
		newscene = new GameClearScene();
	}
	else if (sceneName == "GAMEOVER") {
		newscene = new GameOverScene();
	}
	else {
		assert(0);
	}

	return newscene;

}
