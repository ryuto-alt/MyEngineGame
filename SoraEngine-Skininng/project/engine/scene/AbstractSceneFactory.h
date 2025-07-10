#pragma once

#include "BaseScene.h"
#include <string>
class AbstractSceneFactory
{

public:
	virtual BaseScene* CreateScene(const std::string& Scenename) = 0;
	virtual ~AbstractSceneFactory() = default;

};

