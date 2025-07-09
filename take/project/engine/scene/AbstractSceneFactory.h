#pragma once
#include <string>
#include <memory>

//自作クラス
#include "BaseScene.h"

/// <summary>
/// シーン工場の抽象クラス(ルールの提供者)
/// </summary>
class AbstractSceneFactory {
public:

	//デストラクタ
	virtual ~AbstractSceneFactory() = default;
	//シーン生成
	virtual std::shared_ptr<BaseScene> CreateScene(const std::string& sceneName) = 0;
};