#include "UnoEngine.h"
#include "GameSceneFactory.h" // 具象クラスはcppファイルでインクルード
#include <cassert>
#include <algorithm>
#include <cctype>

// 静的メンバ変数の実体化
UnoEngine* UnoEngine::instance_ = nullptr;

UnoEngine* UnoEngine::GetInstance() {
    if (!instance_) {
        instance_ = new UnoEngine();
    }
    return instance_;
}

void UnoEngine::Initialize() {
    try {
        // WinAppの初期化
        winApp_ = std::make_unique<WinApp>();
        winApp_->Initialize();

        // DirectXCommonの初期化
        dxCommon_ = std::make_unique<DirectXCommon>();
        dxCommon_->Initialize(winApp_.get());

        // SRVマネージャの初期化
        srvManager_ = std::make_unique<SrvManager>();
        srvManager_->Initialize(dxCommon_.get());

        // ここで明示的にPreDrawを呼び出し、ディスクリプタヒープを設定
        srvManager_->PreDraw();

        // テクスチャマネージャの初期化
        TextureManager::GetInstance()->Initialize(dxCommon_.get(), srvManager_.get());

        // デフォルトテクスチャの事前読み込み
        TextureManager::GetInstance()->LoadDefaultTexture();

        // ImGuiの初期化
        InitializeImGui();

        // 入力初期化
        input_ = std::make_unique<Input>();
        input_->Initialize(winApp_.get());

        // オーディオマネージャの初期化
        AudioManager::GetInstance()->Initialize();

        // スプライト共通部分の初期化
        spriteCommon_ = std::make_unique<SpriteCommon>();
        spriteCommon_->Initialize(dxCommon_.get());

        // カメラの作成と初期化
        camera_ = std::make_unique<Camera>();
        camera_->SetTranslate({ 0.0f, 0.0f, -5.0f });
        // Object3dCommonは存在しないためコメントアウト
        // Object3dCommon::SetDefaultCamera(camera_.get());

        // パーティクルマネージャの初期化
        ParticleManager::GetInstance()->Initialize(dxCommon_.get(), srvManager_.get());

        // 基本的なパーティクルグループの作成
        ParticleManager::GetInstance()->CreateParticleGroup("smoke", "Resources/particle/smoke.png");

        // 3Dパーティクルマネージャの初期化
        Particle3DManager::GetInstance()->Initialize(dxCommon_.get(), srvManager_.get(), spriteCommon_.get());

        // 3Dエフェクトマネージャの初期化
        EffectManager3D::GetInstance()->Initialize();

        // 衝突判定マネージャの初期化（特別な初期化は不要）
        // すでにシングルトンパターンで実装されているため、呼び出すだけで初期化される
        
        // 3D空間オーディオリスナーの初期化
        audioListener_ = std::make_unique<SpatialAudioListener>();
        audioListener_->SetPosition(Vector3{0.0f, 0.0f, 0.0f});

        // シーンマネージャーの取得と初期化
        SceneManager* sceneManager = SceneManager::GetInstance();
        sceneManager->SetDirectXCommon(dxCommon_.get());
        sceneManager->SetInput(input_.get());
        sceneManager->SetSpriteCommon(spriteCommon_.get());
        sceneManager->SetSrvManager(srvManager_.get());
        sceneManager->SetCamera(camera_.get());
        sceneManager->SetWinApp(winApp_.get());

        // シーンファクトリーが設定されていれば初期化
        if (sceneFactory_) {
            sceneManager->Initialize(sceneFactory_.get());
        }

    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Exception in UnoEngine::Initialize: " + std::string(e.what()) + "\n").c_str());
    }
}

void UnoEngine::Update() {
    try {
        // Windowsのメッセージ処理
        if (winApp_->ProcessMessage()) {
            endRequest_ = true;
            return;
        }

        // 入力更新
        input_->Update();

        // SRVヒープを描画前に明示的に設定
        if (srvManager_) {
            srvManager_->PreDraw();
        }

        // カメラの更新
        camera_->Update();

        // パーティクルマネージャの更新
        ParticleManager::GetInstance()->Update(camera_.get());

        // 3Dパーティクルマネージャの更新
        Particle3DManager::GetInstance()->Update(camera_.get());

        // 3Dエフェクトマネージャの更新
        EffectManager3D::GetInstance()->Update();

        // 衝突判定マネージャの更新
        Collision::CollisionManager::GetInstance()->Update(1.0f / 60.0f); // 60FPS想定
        
        // 3D空間オーディオの更新
        UpdateSpatialAudio();

        // シーンマネージャーの更新
        SceneManager::GetInstance()->Update();
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Exception in UnoEngine::Update: " + std::string(e.what()) + "\n").c_str());
    }
}

void UnoEngine::Draw() {
    try {
        // DirectXの描画準備
        dxCommon_->Begin();

        // SRVヒープを描画前に明示的に設定
        if (srvManager_) {
            srvManager_->PreDraw();
        }

        // シーンマネージャーの描画
        SceneManager::GetInstance()->Draw();

        // パーティクルの描画
        ParticleManager::GetInstance()->Draw();

        // 3Dパーティクルの描画
        Particle3DManager::GetInstance()->Draw(camera_.get());

        // ImGuiの準備と描画
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon_->GetCommandList());

        // 描画終了
        dxCommon_->End();
    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Exception in UnoEngine::Draw: " + std::string(e.what()) + "\n").c_str());
    }
}

void UnoEngine::Finalize() {
    try {
        // シーンマネージャーの終了処理
        SceneManager::GetInstance()->Finalize();

        // パーティクルマネージャーの終了処理
        ParticleManager::GetInstance()->Finalize();

        // 3Dパーティクルマネージャの終了処理
        Particle3DManager::Finalize();

        // 衝突判定マネージャの終了処理
        Collision::CollisionManager::GetInstance()->ClearColliders();

        // ImGuiの解放
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        // テクスチャマネージャの解放
        TextureManager::GetInstance()->Finalize();

        // オーディオマネージャの解放（必要に応じて）
        AudioManager::GetInstance()->Finalize();
        
        // 3D空間オーディオの解放
        spatialAudioSources_.clear();
        audioListener_.reset();

        // 各リソースはunique_ptrにより自動的に解放される
        // 明示的にnullptrを設定
        camera_.reset();
        spriteCommon_.reset();
        input_.reset();
        srvManager_.reset();
        sceneFactory_.reset();
        dxCommon_.reset();
        winApp_.reset();

        // シングルトンインスタンスの解放
        delete instance_;
        instance_ = nullptr;

    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Exception in UnoEngine::Finalize: " + std::string(e.what()) + "\n").c_str());
    }
}

void UnoEngine::Run() {
    // ゲームループ
    while (!IsEndRequested()) {
        // ImGuiの新しいフレーム
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // 更新
        Update();

        // 描画
        Draw();
    }

    // 終了処理
    Finalize();
}

void UnoEngine::SetSceneFactory(SceneFactory* sceneFactory) {
    // nullptrチェック
    if (sceneFactory == nullptr) {
        sceneFactory_.reset();
        return;
    }

    // unique_ptrにセット（所有権を移譲）
    sceneFactory_ = std::unique_ptr<SceneFactory>(sceneFactory);

    // シーンマネージャーの初期化
    SceneManager::GetInstance()->Initialize(sceneFactory_.get());
}

// === 統合API実装 ===

// === オーディオシステム ===
bool UnoEngine::LoadAudio(const std::string& name, const std::string& filePath) {
    auto* audioManager = AudioManager::GetInstance();
    
    // ファイル拡張子を確認して適切な読み込み関数を選択
    std::string extension = filePath.substr(filePath.find_last_of(".") + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    if (extension == "wav") {
        return audioManager->LoadWAV(name, filePath);
    } else if (extension == "mp3") {
        return audioManager->LoadMP3(name, filePath);
    }
    
    OutputDebugStringA(("警告: サポートされていないオーディオ形式です: " + filePath + "\n").c_str());
    return false;
}

void UnoEngine::PlayAudio(const std::string& name, bool loop) {
    AudioManager::GetInstance()->Play(name, loop);
}

void UnoEngine::StopAudio(const std::string& name) {
    AudioManager::GetInstance()->Stop(name);
}

void UnoEngine::SetAudioVolume(const std::string& name, float volume) {
    AudioManager::GetInstance()->SetVolume(name, volume);
}

bool UnoEngine::IsAudioPlaying(const std::string& name) {
    return AudioManager::GetInstance()->IsPlaying(name);
}

// === パーティクルシステム ===
bool UnoEngine::CreateParticleEffect(const std::string& name, const std::string& texturePath) {
    ParticleManager::GetInstance()->CreateParticleGroup(name, texturePath);
    return true; // TODO: エラーハンドリングの改善
}

void UnoEngine::PlayParticle(const std::string& name, const Vector3& position, int count) {
    ParticleManager::GetInstance()->Emit(name, position, count);
}

void UnoEngine::PlayParticle(const std::string& name, const Vector3& position, int count,
                            const Vector3& velocity, float lifeTime) {
    // より詳細なパラメータでパーティクルを発生
    ParticleManager::GetInstance()->Emit(
        name, position, count,
        velocity, velocity, // 速度の最小・最大を同じに
        Vector3{0.0f, 0.0f, 0.0f}, Vector3{0.0f, 0.0f, 0.0f}, // 加速度なし
        0.5f, 1.0f, // サイズ
        0.0f, 0.3f, // 終了サイズ
        Vector4{1.0f, 1.0f, 1.0f, 1.0f}, Vector4{1.0f, 1.0f, 1.0f, 1.0f}, // 開始色
        Vector4{1.0f, 1.0f, 1.0f, 0.0f}, Vector4{1.0f, 1.0f, 1.0f, 0.0f}, // 終了色
        0.0f, 0.0f, // 回転
        0.0f, 0.0f, // 回転速度
        lifeTime, lifeTime // 寿命
    );
}

// === 3Dオブジェクト作成システム ===
std::unique_ptr<Object3d> UnoEngine::CreateObject3D() {
    auto object = std::make_unique<Object3d>();
    object->Initialize(dxCommon_.get(), spriteCommon_.get());
    object->SetCamera(camera_.get());
    return object;
}

std::unique_ptr<Model> UnoEngine::LoadModel(const std::string& modelPath) {
    auto model = std::make_unique<Model>();
    
    // Modelを初期化（DirectXCommonを渡す）
    model->Initialize(dxCommon_.get());
    
    // パスの解析
    size_t lastSlash = modelPath.find_last_of("/\\");
    std::string directoryPath = (lastSlash != std::string::npos) ? 
        modelPath.substr(0, lastSlash + 1) : "";
    std::string filename = (lastSlash != std::string::npos) ? 
        modelPath.substr(lastSlash + 1) : modelPath;
    
    model->LoadFromObj(directoryPath, filename);
    return model;
}

// === 2Dスプライト作成システム ===
std::unique_ptr<Sprite> UnoEngine::CreateSprite(const std::string& texturePath) {
    // テクスチャを読み込み
    LoadTexture(texturePath);
    
    auto sprite = std::make_unique<Sprite>();
    sprite->Initialize(spriteCommon_.get(), texturePath);
    return sprite;
}

// === テクスチャ読み込み ===
uint32_t UnoEngine::LoadTexture(const std::string& filePath) {
    // テクスチャを読み込んでSRVインデックスを取得
    TextureManager::GetInstance()->LoadTexture(filePath);
    return TextureManager::GetInstance()->GetSrvIndex(filePath);
}

// === 衝突判定システム ===
bool UnoEngine::CheckCollision(const Vector3& pos1, float radius1, const Vector3& pos2, float radius2) {
    // 2つの球の衝突判定
    Collision::Sphere sphere1;
    sphere1.center = pos1;
    sphere1.radius = radius1;
    
    Collision::Sphere sphere2;
    sphere2.center = pos2;
    sphere2.radius = radius2;
    
    auto result = Collision::CollisionDetector::CheckSphereToSphere(sphere1, sphere2);
    return result.isColliding;
}

// === シーン管理 ===
void UnoEngine::ChangeScene(const std::string& sceneName) {
    SceneManager::GetInstance()->ChangeScene(sceneName);
}

// === デバッグ情報 ===
void UnoEngine::ShowDebugInfo() {
    ImGui::Begin("UnoEngine デバッグ情報");
    
    // FPS情報
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    
    // カメラ情報
    Vector3 cameraPos = camera_->GetTranslate();
    ImGui::Text("カメラ位置: (%.2f, %.2f, %.2f)", cameraPos.x, cameraPos.y, cameraPos.z);
    
    // メモリ使用量（概算）
    // 全パーティクルグループのパーティクル数を合計
    uint32_t totalParticles = 0;
    auto* particleManager = ParticleManager::GetInstance();
    // 基本的なパーティクルグループの数をチェック
    totalParticles += particleManager->GetParticleCount("explosion");
    totalParticles += particleManager->GetParticleCount("smoke");
    ImGui::Text("アクティブパーティクル数: %d", totalParticles);
    
    // 入力状態
    if (ImGui::TreeNode("入力状態")) {
        ImGui::Text("ESC: %s", input_->PushKey(DIK_ESCAPE) ? "押下中" : "未押下");
        ImGui::Text("SPACE: %s", input_->PushKey(DIK_SPACE) ? "押下中" : "未押下");
        ImGui::Text("W: %s", input_->PushKey(DIK_W) ? "押下中" : "未押下");
        ImGui::Text("A: %s", input_->PushKey(DIK_A) ? "押下中" : "未押下");
        ImGui::Text("S: %s", input_->PushKey(DIK_S) ? "押下中" : "未押下");
        ImGui::Text("D: %s", input_->PushKey(DIK_D) ? "押下中" : "未押下");
        ImGui::TreePop();
    }
    
    ImGui::End();
}

// === 3D空間オーディオシステム実装 ===
std::unique_ptr<SpatialAudioSource> UnoEngine::CreateSpatialAudioSource(const std::string& audioName, const Vector3& position) {
    auto spatialSource = std::make_unique<SpatialAudioSource>();
    
    if (spatialSource->Initialize(audioName, position)) {
        return spatialSource;
    }
    
    return nullptr;
}

void UnoEngine::SetAudioListenerPosition(const Vector3& position) {
    if (audioListener_) {
        audioListener_->SetPosition(position);
    }
}

void UnoEngine::SetAudioListenerOrientation(const Vector3& forward, const Vector3& up) {
    if (audioListener_) {
        audioListener_->SetOrientation(forward, up);
    }
}

void UnoEngine::UpdateSpatialAudio() {
    if (!audioListener_) return;
    
    // 全ての3D空間オーディオソースを更新
    for (auto& spatialSource : spatialAudioSources_) {
        if (spatialSource) {
            spatialSource->Update(audioListener_->GetPosition(), audioListener_->GetForward());
        }
    }
}

void UnoEngine::InitializeImGui() {
    try {
        // ImGui初期化
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        // ImGuiのIO設定を取得
        ImGuiIO& io = ImGui::GetIO();

        // 日本語フォント設定
        ImFontConfig fontConfig;
        fontConfig.MergeMode = true;  // 既存のフォントと統合

        // 日本語フォントのパス（例：MS Gothic）
        // Windows標準フォントを使用
        const char* fontPath = "C:\\Windows\\Fonts\\msgothic.ttc";

        // 日本語の文字範囲を指定
        static const ImWchar japaneseFontRanges[] = {
            0x0020, 0x00FF, // 基本ラテン文字
            0x3000, 0x30FF, // 日本語（平仮名、カタカナ）
            0x31F0, 0x31FF, // カタカナ拡張
            0xFF00, 0xFFEF, // 全角文字
            0x4E00, 0x9FAF, // CJK統合漢字
            0,
        };

        // デフォルトフォント読み込み
        io.Fonts->AddFontDefault();

        // 日本語フォント読み込み
        io.Fonts->AddFontFromFileTTF(fontPath, 16.0f, &fontConfig, japaneseFontRanges);

        // ファイルが見つからない場合のフォールバック処理
        if (!io.Fonts->Fonts.Size || io.Fonts->Fonts.Size <= 1) {
            // デフォルトフォントのみの場合は警告を出力
            OutputDebugStringA("WARNING: 日本語フォントが読み込めませんでした。フォールバックフォントを使用します。\n");

            // フォールバックフォントとしてWindows標準のフォントを試行
            const char* fallbackFonts[] = {
                "C:\\Windows\\Fonts\\meiryo.ttc",
                "C:\\Windows\\Fonts\\msgothic.ttc",
                "C:\\Windows\\Fonts\\YuGothM.ttc"
            };

            for (const char* fallbackFont : fallbackFonts) {
                io.Fonts->AddFontFromFileTTF(fallbackFont, 16.0f, &fontConfig, japaneseFontRanges);
                if (io.Fonts->Fonts.Size > 1) {
                    OutputDebugStringA(("日本語フォールバックフォントを読み込みました: " + std::string(fallbackFont) + "\n").c_str());
                    break;
                }
            }
        }

        // フォントテクスチャをビルド
        io.Fonts->Build();

        ImGui_ImplWin32_Init(winApp_->GetHwnd());

        // SrvManagerのディスクリプタヒープを使用
        ImGui_ImplDX12_Init(
            dxCommon_->GetDevice(),
            2, // SwapChainのバッファ数
            DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
            srvManager_->GetDescriptorHeap().Get(),
            srvManager_->GetCPUDescriptorHandle(0), // ImGui用に0番を使用
            srvManager_->GetGPUDescriptorHandle(0)
        );

    }
    catch (const std::exception& e) {
        OutputDebugStringA(("ERROR: Failed to initialize ImGui: " + std::string(e.what()) + "\n").c_str());
    }
}