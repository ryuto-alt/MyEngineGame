#pragma once
#pragma warning(push)
//C4023の警告を見なかったことにする
#pragma warning(disable:4023)
//Include
#include "base/DirectXCommon.h"
#include "base/D3DResourceLeakChecker.h"
#include "base/TextureManager.h"
#include "base/ModelManager.h"
#include "base/SrvManager.h"
#include "base/RtvManager.h"
#include "base/WinApp.h"
#include "base/Particle/ParticleManager.h"
#include "3d/Object3dCommon.h"
#include "3d/Primitive/PrimitiveDrawer.h"
#include "3d/Particle/ParticleCommon.h"
#include "3d/Particle/ParticleEditor.h"
#include "2d/SpriteCommon.h"
#include "2d/WireFrame.h"
#include "Animation/Animator.h"
#include "audio/Audio.h"
#include "io/Input.h"
#include "camera/CameraManager.h"
#include "primitive/Sphere.h"
#include "PostEffect/PostEffectManager.h"
#include "PostEffect/RenderTexture.h"
#include "scene/SceneManager.h"
#include "scene/AbstractSceneFactory.h"
#include "Utility/Logger.h"
#include "Utility/ResourceBarrier.h"
#include "Utility/JsonLoader.h"

//DirectXTex
#include <dxgidebug.h>
#pragma comment(lib,"dxguid.lib")
//imgui
#include "ImGuiManager.h"

#include <chrono>

class TakeCFrameWork {
public:

	//デストラクタ
	virtual ~TakeCFrameWork() = default;
	//初期化
	virtual void Initialize(const std::wstring& titleName);
	//終了処理
	virtual void Finalize();
	//更新処理
	virtual void Update();
	//描画処理
	virtual void Draw() = 0;
	//終了フラグの取得
	virtual bool IsEndRequest() const { return isEnd_; }

	//実行処理
	void Run(const std::wstring& titleName);

	static ParticleManager* GetParticleManager();

	static Animator* GetAnimator();

	static JsonLoader* GetJsonLoader();

	static PrimitiveDrawer* GetPrimitiveDrawer();

	static WireFrame* GetWireFrame();

	static float GetGameTime();

	static const float GetDeltaTime() { return kDeltaTime; }

protected:

	std::unique_ptr<WinApp> winApp_ = nullptr;
	std::unique_ptr<DirectXCommon> directXCommon_ = nullptr;
	std::unique_ptr<SrvManager> srvManager_ = nullptr;
	std::unique_ptr<RenderTexture> renderTexture_ = nullptr;
	Input* input_ = nullptr;
	AudioManager* audio_ = nullptr;
	SpriteCommon* spriteCommon_ = nullptr;
	Object3dCommon* object3dCommon_ = nullptr;
	ParticleCommon* particleCommon_ = nullptr;
	SceneManager* sceneManager_ = nullptr;
	ImGuiManager* imguiManager_ = nullptr;
	std::unique_ptr<AbstractSceneFactory> sceneFactory_ = nullptr;
	static std::unique_ptr<Animator> animator_;
	static std::unique_ptr<JsonLoader> jsonLoader_;
	static std::unique_ptr<ParticleManager> particleManager_;
	static std::unique_ptr<PrimitiveDrawer> primitiveDrawer_;
	static std::unique_ptr<PostEffectManager> postEffectManager_;
	static std::unique_ptr<WireFrame> wireFrame_;
	
	// ゲームの起動時間
	static std::chrono::steady_clock::time_point gameTime_;
	// ゲームの経過時間
	static const float kDeltaTime;
	//終了フラグ
	bool isEnd_ = false;
};