#pragma once
#include "AbstractSceneFactory.h"
/// <summary>
/// シーン工場クラス
/// </summary>
class SceneFactory : public AbstractSceneFactory {
public:

	/// <summary>
	/// シーン生成
	/// </summary>
	/// <param name="sceneName">シーン名</param>
	/// <returns>生成したシーン</returns>
	std::shared_ptr<BaseScene> CreateScene(const std::string& sceneName) override;
};

