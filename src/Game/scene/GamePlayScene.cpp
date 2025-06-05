#include "GamePlayScene.h"
#include "TextureManager.h"

GamePlayScene::GamePlayScene() {
    // コンストラクタでは特に何もしない
}

GamePlayScene::~GamePlayScene() {
    // デストラクタでも特に何もしない
}

void GamePlayScene::Initialize() {
    // 必要なリソースの取得確認
    assert(dxCommon_);
    assert(input_);
    assert(spriteCommon_);
    assert(camera_);

    // カメラの初期設定
    camera_->SetTranslate({ 0.0f, 5.0f, -20.0f });
    camera_->SetRotate({ 0.1f, 0.0f, 0.0f });
    camera_->Update();
    
    // デバッグモードでない場合のみ独自のスパイダーを作成
    if (!isDebugMode_) {
        // アニメーションパイプラインの初期化
        animatedPipeline_ = std::make_unique<AnimatedRenderingPipeline>();
        animatedPipeline_->Initialize(dxCommon_);
        
        // Spider_3.fbxモデルの読み込み
        spiderModel_ = std::make_shared<FBXModel>();
        spiderModel_->Initialize(dxCommon_);
        if (spiderModel_->LoadFromFile("Resources/Models/spider/Spider_3.fbx")) {
            OutputDebugStringA("Spider_3.fbx loaded successfully\n");
            
            // スパイダーオブジェクトの作成と初期化
            spiderObject_ = std::make_unique<AnimatedObject3d>();
            spiderObject_->Initialize(dxCommon_, spriteCommon_);
            spiderObject_->SetFBXModel(spiderModel_);
            spiderObject_->SetPosition({ 0.0f, 0.0f, 0.0f });
            spiderObject_->SetScale({ 1.0f, 1.0f, 1.0f });
            spiderObject_->SetCamera(camera_);
            
            // デフォルトテクスチャを事前にロード
            OutputDebugStringA("Pre-loading default white texture\n");
            TextureManager::GetInstance()->LoadTexture("Resources/white.png");
            
            // スパイダーのテクスチャを事前にロード
            const auto& materials = spiderModel_->GetMaterials();
            for (const auto& material : materials) {
                if (!material.diffuseTexture.empty()) {
                    OutputDebugStringA(("Pre-loading texture: " + material.diffuseTexture + "\n").c_str());
                    TextureManager::GetInstance()->LoadTexture(material.diffuseTexture);
                }
            }
            
            // アニメーションの再生開始
            spiderObject_->PlayAnimation("Walk", true);
        } else {
            OutputDebugStringA("Failed to load Spider_3.fbx\n");
        }
    }
    
    // ステージエディターの初期化
    if (isDebugMode_) {
        OutputDebugStringA("GamePlayScene: Debug mode is ON, initializing StageEditor\n");
        stageEditor_ = std::make_unique<StageEditor>();
        stageEditor_->Initialize(camera_, input_, dxCommon_, spriteCommon_);
    } else {
        OutputDebugStringA("GamePlayScene: Debug mode is OFF\n");
    }

    // 初期化完了
    initialized_ = true;
    OutputDebugStringA("GamePlayScene: 初期化完了\n");
}

void GamePlayScene::Update() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;
    
    // F1キーでエディターモード切り替え
    if (isDebugMode_ && input_->TriggerKey(DIK_F1)) {
        if (stageEditor_) {
            bool newState = !stageEditor_->IsEnabled();
            stageEditor_->SetEnabled(newState);
            if (newState) {
                OutputDebugStringA("StageEditor: ENABLED\n");
            } else {
                OutputDebugStringA("StageEditor: DISABLED\n");
            }
        }
    }
    
    // エディターモードでない場合のみESCキーでタイトルに戻る
    if (!stageEditor_ || !stageEditor_->IsEnabled()) {
        if (input_->TriggerKey(DIK_ESCAPE)) {
            sceneManager_->ChangeScene("Title");
        }
    }
    
    // スパイダーアニメーションの更新
    if (spiderObject_) {
        spiderObject_->Update();
    }
    
    // ステージエディターの更新
    if (stageEditor_) {
        stageEditor_->Update();
    }

    // カメラの更新
    camera_->Update();
}

void GamePlayScene::Draw() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;
    
    // ステージエディターのオブジェクト描画
    if (stageEditor_) {
        stageEditor_->DrawObjects();
    }
    
    // デバッグモードでない場合のみGamePlaySceneのスパイダーを描画
    if (!isDebugMode_ && animatedPipeline_ && spiderObject_) {
        animatedPipeline_->Bind();
        spiderObject_->Draw();
    }

    // ImGuiでシーン情報のみ表示
    ImGui::Begin("GamePlayScene情報");
    
    ImGui::Text("シーン名: GamePlayScene");
    ImGui::Text("状態: 初期化済み");
    if (isDebugMode_) {
        ImGui::Text("Debug Mode: ON");
        ImGui::Text("F1: Toggle Stage Editor");
        if (stageEditor_ && stageEditor_->IsEnabled()) {
            ImGui::Text("Stage Editor: ACTIVE");
        }
    }
    ImGui::Separator();
    
    ImGui::Text("カメラ情報:");
    Vector3 camPos = camera_->GetTranslate();
    Vector3 camRot = camera_->GetRotate();
    ImGui::Text("位置: (%.2f, %.2f, %.2f)", camPos.x, camPos.y, camRot.z);
    ImGui::Text("回転: (%.2f, %.2f, %.2f)", camRot.x, camRot.y, camRot.z);
    ImGui::Separator();
    
    ImGui::Text("スパイダーアニメーション:");
    if (spiderObject_) {
        ImGui::Text("状態: %s", spiderObject_->IsAnimationPlaying() ? "再生中" : "停止");
    }
    ImGui::Separator();
    
    ImGui::Text("操作説明:");
    if (!stageEditor_ || !stageEditor_->IsEnabled()) {
        ImGui::Text("ESC - タイトルに戻る");
    }
    
    ImGui::End();
    
    // ステージエディターのImGui描画
    if (stageEditor_) {
        stageEditor_->DrawImGui();
    }
}

void GamePlayScene::Finalize() {
    spiderObject_.reset();
    spiderModel_.reset();
    animatedPipeline_.reset();
    stageEditor_.reset();
    OutputDebugStringA("GamePlayScene: 終了処理完了\n");
}