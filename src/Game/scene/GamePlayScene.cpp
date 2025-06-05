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

    // カメラの初期設定（より近くから見る）
    camera_->SetTranslate({ 0.0f, 50.0f, -100.0f });  // より遠くから見る
    camera_->SetRotate({ 0.3f, 0.0f, 0.0f });  // 少し下向きに
    camera_->Update();
    
    // デバッグモードでない場合のみ独自のスパイダーを作成
    if (!isDebugMode_) {
        // アニメーションパイプラインの初期化
        animatedPipeline_ = std::make_unique<AnimatedRenderingPipeline>();
        animatedPipeline_->Initialize(dxCommon_);
        
        // Spider.fbxモデルの読み込み
        spiderModel_ = std::make_shared<FBXModel>();
        spiderModel_->Initialize(dxCommon_);
        if (spiderModel_->LoadFromFile("Resources/Models/spider/Spider_2.fbx")) {
            OutputDebugStringA("Spider_2.fbx loaded successfully\n");
            
            // メッシュとアニメーション情報のログ出力
            OutputDebugStringA(("GamePlayScene: Spider model - Meshes: " + std::to_string(spiderModel_->GetMeshes().size()) + 
                               ", Animations: " + std::to_string(spiderModel_->GetAnimations().size()) + "\n").c_str());
            
            // スパイダーオブジェクトの作成と初期化
            spiderObject_ = std::make_unique<AnimatedObject3d>();
            spiderObject_->Initialize(dxCommon_, spriteCommon_);
            spiderObject_->SetFBXModel(spiderModel_);
            spiderObject_->SetPosition({ 0.0f, 0.0f, 0.0f });  // カメラの正面に配置
            spiderObject_->SetScale({ 5.0f, 5.0f, 5.0f });  // スケールを大きく
            spiderObject_->SetCamera(camera_);
            
            // デフォルトテクスチャを事前にロード
            OutputDebugStringA("Pre-loading default white texture\n");
            TextureManager::GetInstance()->LoadTexture("Resources/white.png");
            
            // スパイダーのテクスチャを事前にロード
            const auto& materials = spiderModel_->GetMaterials();
            for (const auto& material : materials) {
                if (!material.diffuseTexture.empty()) {
                    OutputDebugStringA(("Pre-loading texture: " + material.diffuseTexture + "\n").c_str());
                    // テクスチャパスを修正（バックスラッシュをフォワードスラッシュに変換）
                    std::string texturePath = material.diffuseTexture;
                    std::replace(texturePath.begin(), texturePath.end(), '\\', '/');
                    
                    // Resources/Models/spider/以下のテクスチャを探す
                    std::string basePath = "Resources/Models/spider/";
                    size_t pos = texturePath.find_last_of("/");
                    if (pos != std::string::npos) {
                        texturePath = basePath + texturePath.substr(pos + 1);
                    } else {
                        texturePath = basePath + texturePath;
                    }
                    
                    OutputDebugStringA(("Trying to load texture from: " + texturePath + "\n").c_str());
                    TextureManager::GetInstance()->LoadTexture(texturePath);
                }
            }
            
            // アニメーションの再生開始
            // FBXファイルにアニメーションが含まれていれば自動的に再生される
            if (!spiderModel_->IsAnimationPlaying()) {
                // アニメーションが自動再生されていない場合、手動で開始
                const auto& animations = spiderModel_->GetAnimations();
                if (!animations.empty()) {
                    // 歩行アニメーションを優先的に選択
                    std::string animName = "";
                    if (animations.find("walk_ani_vor") != animations.end()) {
                        animName = "walk_ani_vor";
                    } else if (animations.find("walk_ani_back") != animations.end()) {
                        animName = "walk_ani_back";
                    } else if (animations.find("walk_left") != animations.end()) {
                        animName = "walk_left";
                    } else if (animations.find("walk_right") != animations.end()) {
                        animName = "walk_right";
                    } else {
                        animName = animations.begin()->first;
                    }
                    spiderObject_->PlayAnimation(animName, true);
                    OutputDebugStringA(("GamePlayScene: Playing animation '" + animName + "'\n").c_str());
                } else {
                    OutputDebugStringA("GamePlayScene: No animations found in FBX file\n");
                }
            }
        } else {
            OutputDebugStringA("Failed to load Spider_2.fbx\n");
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