#pragma once
#include <numbers>
#include <algorithm>
#include <fstream>
#include <sstream>

#include "Input.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include"D3DResourceLeakChecker.h"
#include "Logger.h"
#include "SpriteCommon.h"


#include "Object3DCommon.h"

#include "RenderingData.h"

#include "ModelManager.h"
#include "TextureManager.h"
#include"ImGuiManager.h"
#include <imgui.h>
#include "Audio.h"
#include "SrvManager.h"
#include "SceneManager.h"
#include <SceneFactory.h>
#include "OfscreenRenderManager.h"

#include "Linecommon.h"
#include "Line.h"


class Framework
{
public:
	//ゲームの初期化
	virtual void Initialize();
	//終了
	virtual void Finalize();
	//更新
	virtual void Update();
	//描画
	virtual void Draw()=0;

	void Run();

	//ゲーム終了フラグの取得
	virtual bool IsEndRequest()const { return endRequst_; }

public:

	//ゲーム終了フラグ	
	bool endRequst_ = false;

	//WinAppのポインタ
	std::unique_ptr<WinApp> winApp;
	//DirectXCommonのポインタ
	std::unique_ptr<DirectXCommon> dxCommon;
	//SrvManagerのポインタ
	std::unique_ptr<SrvManager> srvManager;
	//ImGuiManagerのポインタ
	std::unique_ptr<ImGuiManager> imGuiMnager;
	//SceneManagerのポインタ
	std::unique_ptr<AbstractSceneFactory> sceneFactory;
	std::unique_ptr<OfscreenRenderManager> ofscreenRenderManager;


};

