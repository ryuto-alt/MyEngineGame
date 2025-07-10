#pragma once

#include "AbstractSceneFactory.h"
class SceneFactory
	: public AbstractSceneFactory
{

public:
	/// <summary>
	/// シーンの生成
	/// </summary>
	BaseScene* CreateScene(const std::string&sceneName)override;





};

