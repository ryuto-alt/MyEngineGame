#include "TakeCFrameWork.h"
#include <cassert>

//Clockの宣言
using Clock = std::chrono::high_resolution_clock;

std::unique_ptr<Animator> TakeCFrameWork::animator_ = nullptr;
std::unique_ptr<JsonLoader> TakeCFrameWork::jsonLoader_ = nullptr;
std::unique_ptr<ParticleManager> TakeCFrameWork::particleManager_ = nullptr;
std::unique_ptr<PrimitiveDrawer> TakeCFrameWork::primitiveDrawer_ = nullptr;
std::unique_ptr<PostEffectManager> TakeCFrameWork::postEffectManager_= nullptr;
std::unique_ptr<WireFrame> TakeCFrameWork::wireFrame_ = nullptr;
std::chrono::steady_clock::time_point TakeCFrameWork::gameTime_ = Clock::now();
const float TakeCFrameWork::kDeltaTime = 0.016f; // 60FPSを基準にしたデルタタイム

//====================================================================
//			初期化
//====================================================================

void TakeCFrameWork::Initialize(const std::wstring& titleName) {

	//タイトルバーの名前の入力
	winApp_ = std::make_unique<WinApp>();
	winApp_->Initialize(titleName.c_str());

	////DirectX初期化
	directXCommon_ = std::make_unique<DirectXCommon>();
	directXCommon_->Initialize(winApp_.get());
	//SrvManager
	srvManager_ = std::make_unique<SrvManager>();
	srvManager_->Initialize(directXCommon_.get());

	//ResourceBarrier
	ResourceBarrier::GetInstance()->Initialize(directXCommon_.get());

	
	//入力初期化
	input_ = Input::GetInstance();
	input_->Initialize(winApp_.get());

	//Audio
	audio_ = AudioManager::GetInstance();
	audio_->Initialize();

	//JsonLoader
	jsonLoader_ = std::make_unique<JsonLoader>();

	//SpriteCommon
	spriteCommon_ = SpriteCommon::GetInstance();
	spriteCommon_->Initialize(directXCommon_.get());

	//Object3dCommon
	object3dCommon_ = Object3dCommon::GetInstance();
	object3dCommon_->Initialize(directXCommon_.get());

	//ParticleCommon
	particleCommon_ = ParticleCommon::GetInstance();
	particleCommon_->Initialize(directXCommon_.get(), srvManager_.get());

	//Animator
	animator_ = std::make_unique<Animator>();

	//CameraManager
	CameraManager::GetInstance()->Initialize(directXCommon_.get());

	//ModelManager
	ModelManager::GetInstance()->Initialize(directXCommon_.get(), srvManager_.get());

	//TextureManager
	TextureManager::GetInstance()->Initialize(directXCommon_.get(), srvManager_.get());

	//ParticleManager
	particleManager_ = std::make_unique<ParticleManager>();

	
	//PrimitiveDrawer
	primitiveDrawer_ = std::make_unique<PrimitiveDrawer>();
	primitiveDrawer_->Initialize(directXCommon_.get(), srvManager_.get());

	//PostEffectManager
	postEffectManager_ = std::make_unique<PostEffectManager>();
	postEffectManager_->Initialize(directXCommon_.get(), srvManager_.get());

	//RenderTexture
	renderTexture_ = std::make_unique<RenderTexture>();
	renderTexture_->Initialize(directXCommon_.get(), srvManager_.get(),postEffectManager_.get());

	//WireFrame
	wireFrame_ = std::make_unique<WireFrame>();
	wireFrame_->Initialize(directXCommon_.get());


#ifdef _DEBUG
	imguiManager_ = new ImGuiManager();
	imguiManager_->Initialize(winApp_.get(), directXCommon_.get(), srvManager_.get());
#endif

	//sceneManager
	sceneManager_ = SceneManager::GetInstance();

}

//====================================================================
//			終了処理
//====================================================================

void TakeCFrameWork::Finalize() {
#ifdef _DEBUG
	imguiManager_->Finalize();
#endif

	wireFrame_->Finalize();
	animator_->Finalize();
	TextureManager::GetInstance()->Finalize();
	ModelManager::GetInstance()->Finalize();
	CameraManager::GetInstance()->Finalize();
	ResourceBarrier::GetInstance()->Finalize();
	postEffectManager_->Finalize();
	renderTexture_.reset();
	particleManager_->Finalize();
	primitiveDrawer_->Finalize();
	particleCommon_->Finalize();
	object3dCommon_->Finalize();
	spriteCommon_->Finalize();
	sceneFactory_.reset();
	//Audioの開放
	audio_->Finalize();
	//入力の開放
	input_->Finalize();
	//SrvManagerの開放
	srvManager_.reset();
	//directXCommonの開放
	directXCommon_->Finalize();
	directXCommon_.reset();

	//winAppの開放
	winApp_->Finalize();
	winApp_.reset();
}

//====================================================================
//			更新処理
//====================================================================

void TakeCFrameWork::Update() {

	//メッセージ処理
	if (winApp_->ProcessMessage()) {
		//ウィンドウの×ボタンが押されたらループを抜ける
		isEnd_ = true;
	}

#ifdef _DEBUG
	imguiManager_->Begin();
#endif

	//入力の更新
	input_->Update();

	//シーンの更新
	sceneManager_->Update();
	sceneManager_->UpdateImGui();
	postEffectManager_->UpdateImGui();

	//ImGuiのRenderTextureのSRVインデックスを設定
	imguiManager_->SetRenderTextureIndex(postEffectManager_->GetFinalOutputSrvIndex()); 

	imguiManager_->DrawDebugScreen();
#ifdef _DEBUG
	imguiManager_->End();
#endif // DEBUG
}

//====================================================================
//			実行処理
//====================================================================

void TakeCFrameWork::Run(const std::wstring& titleName) {
	//ゲームクラスの生成と初期化
	Initialize(titleName);

	//ウィンドウの×ボタンが押されるまでループ
	while (true) {
		if (IsEndRequest() == true) {
			break;
		}
		Update(); //更新処理
		Draw();   //描画処理
	}

	Finalize();   //終了処理
}

//====================================================================
//			パーティクルマネージャの取得
//====================================================================

ParticleManager* TakeCFrameWork::GetParticleManager() {
	assert(particleManager_ != nullptr);
	return particleManager_.get();
}

//====================================================================
//			アニメーターの取得
//====================================================================

Animator* TakeCFrameWork::GetAnimator() {
	assert(animator_ != nullptr);
	return animator_.get();
}
//====================================================================
//			JSONローダーの取得
//====================================================================
JsonLoader* TakeCFrameWork::GetJsonLoader() {
	assert(jsonLoader_ != nullptr);
	return jsonLoader_.get();
}

PrimitiveDrawer* TakeCFrameWork::GetPrimitiveDrawer() {
	assert(primitiveDrawer_ != nullptr);
	return primitiveDrawer_.get();
}

//====================================================================
//			ワイヤーフレーム管理クラスの取得
//====================================================================

WireFrame* TakeCFrameWork::GetWireFrame() {
	assert(wireFrame_ != nullptr);
	return wireFrame_.get();
}

float TakeCFrameWork::GetGameTime() {
	
	//現在の時間を取得
	auto now = Clock::now();

	//経過時間をミリ秒単位で計算
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - gameTime_).count();

	//秒に変換
	return duration / 1000.0f;
}
