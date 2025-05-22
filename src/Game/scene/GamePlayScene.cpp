#include "GamePlayScene.h"
#include "UnoEngine.h"
#include "TextureManager.h"
#include "Vector3.h"
#include "Vector4.h"
#include <cmath>
#include <string>
#include <cassert>

// 定数定義
namespace {
    constexpr float PI = 3.14159265f;
}

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
    camera_->SetTranslate({ 0.0f, 0.0f, -12.0f });
    camera_->SetRotate({ 0.0f, 0.0f, 0.0f });
    camera_->Update();

    // gradationLine.pngテクスチャの事前読み込み
    TextureManager::GetInstance()->LoadTexture("Resources/particle/gradationLine.png");

    // 魔法陣エフェクトの初期化
    magicCircleEffect_ = std::make_unique<MagicCircleEffect>();
    magicCircleEffect_->Initialize(dxCommon_, spriteCommon_);
    magicCircleEffect_->SetCamera(camera_);
    magicCircleEffect_->CreateMagicCircle({-4.0f, 0.0f, 0.0f}); // 左側に配置

    // ポータルエフェクトの初期化
    portalEffect_ = std::make_unique<PortalEffect>();
    portalEffect_->Initialize(dxCommon_, spriteCommon_);
    portalEffect_->SetCamera(camera_);
    portalEffect_->CreatePortal({4.0f, 0.0f, 0.0f}); // 右側に配置
    portalEffect_->SetPortalType(0); // 青いポータル

    // アニメーション設定
    effectTimer_ = 0.0f;
    animationTime_ = 0.0f;
    currentEffectMode_ = 0; // 0: 両方表示, 1: 魔法陣のみ, 2: ポータルのみ
    keyPressed_ = false;
    
    // エフェクト表示状態の初期化
    magicCircleVisible_ = true;  // 初期状態では両方表示
    portalVisible_ = true;

    // 初期化完了
    initialized_ = true;
    
    OutputDebugStringA("GamePlayScene: Epic Effect System Initialized!\n");
    OutputDebugStringA("=== EPIC GAME EFFECTS DEMO ===\n");
    OutputDebugStringA("- Magic Circle: Multi-layer rotating runes\n");
    OutputDebugStringA("- Portal Effect: Interdimensional gateway\n");
    OutputDebugStringA("- Real-time animations and distortions\n");
    OutputDebugStringA("- Game-quality visual effects\n");
    OutputDebugStringA("===============================\n");
}

void GamePlayScene::Update() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;

    // デルタタイム計算（60FPS想定）
    float deltaTime = 1.0f / 60.0f;
    
    // アニメーションタイマー更新
    animationTime_ += deltaTime;
    effectTimer_ += deltaTime;

    // ESCキーでタイトルシーンに戻る
    if (input_->TriggerKey(DIK_ESCAPE)) {
        sceneManager_->ChangeScene("Title");
    }

    // エフェクト制御
    HandleEffectSwitching();
    UpdateDynamicEffects(deltaTime);

    // エフェクト更新
    if (magicCircleEffect_) {
        magicCircleEffect_->Update(deltaTime);
    }
    if (portalEffect_) {
        portalEffect_->Update(deltaTime);
    }

    // カメラの更新（WASD入力対応）
    camera_->UpdateWithInput(input_);
}

void GamePlayScene::Draw() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;

    // エフェクト描画（表示状態フラグに基づいて描画）
    // IsVisible()の条件を削除し、表示フラグのみで制御
    if (magicCircleVisible_ && magicCircleEffect_) {
        magicCircleEffect_->Draw();
        // デバッグ: 描画呼び出しを確認
        static int magicDrawCount = 0;
        if (++magicDrawCount % 60 == 0) { // 60フレームごとに出力
            OutputDebugStringA("Magic Circle Draw() called\n");
        }
    }
    if (portalVisible_ && portalEffect_) {
        portalEffect_->Draw();
        // デバッグ: 描画呼び出しを確認
        static int portalDrawCount = 0;
        if (++portalDrawCount % 60 == 0) { // 60フレームごとに出力
            OutputDebugStringA("Portal Draw() called\n");
        }
    }

    // UI表示
    ShowEffectUI();
}

void GamePlayScene::Finalize() {
    magicCircleEffect_.reset();
    portalEffect_.reset();
    OutputDebugStringA("GamePlayScene: Epic effects finalized\n");
}

void GamePlayScene::HandleEffectSwitching() {
    // 数字キーでエフェクト切り替え
    bool keyPressed = false;

    // 1キー: 魔法陣の表示/非表示切替
    if (input_->TriggerKey(DIK_1)) {
        magicCircleVisible_ = !magicCircleVisible_;
        keyPressed = true;
        OutputDebugStringA(("1 Key: Magic Circle toggled: " + std::string(magicCircleVisible_ ? "ON" : "OFF") + "\n").c_str());
    }
    // 2キー: ポータルの表示/非表示切替
    else if (input_->TriggerKey(DIK_2)) {
        portalVisible_ = !portalVisible_;
        if (portalEffect_) {
            if (portalVisible_) {
                // 表示する時は強制的にアクティブ化
                portalEffect_->SetVisible(true);
                portalEffect_->StartEffect();
            } else {
                // 非表示する時は停止
                portalEffect_->SetVisible(false);
                portalEffect_->StopEffect();
            }
        }
        keyPressed = true;
        OutputDebugStringA(("2 Key: Portal toggled: " + std::string(portalVisible_ ? "ON" : "OFF") + "\n").c_str());
    }
    // 0キー: 両方表示
    else if (input_->TriggerKey(DIK_0)) {
        magicCircleVisible_ = true;
        portalVisible_ = true;
        // ポータルを強制的にアクティブ化
        if (portalEffect_) {
            portalEffect_->SetVisible(true);
            portalEffect_->StartEffect();
        }
        keyPressed = true;
        OutputDebugStringA("0 Key: Both effects enabled\n");
    }

    // Spaceキーで次のモードに切り替え（保持）
    if (input_->TriggerKey(DIK_SPACE)) {
        currentEffectMode_ = (currentEffectMode_ + 1) % 3;
        
        switch (currentEffectMode_) {
            case 0: // 両方表示
                magicCircleVisible_ = true;
                portalVisible_ = true;
                if (portalEffect_) {
                    portalEffect_->SetVisible(true);
                    portalEffect_->StartEffect();
                }
                break;
            case 1: // 魔法陣のみ
                magicCircleVisible_ = true;
                portalVisible_ = false;
                if (portalEffect_) {
                    portalEffect_->SetVisible(false);
                    portalEffect_->StopEffect();
                }
                break;
            case 2: // ポータルのみ
                magicCircleVisible_ = false;
                portalVisible_ = true;
                if (portalEffect_) {
                    portalEffect_->SetVisible(true);
                    portalEffect_->StartEffect();
                }
                break;
        }
        keyPressed = true;
        OutputDebugStringA(("Space Key: Mode switched to: " + std::to_string(currentEffectMode_) + "\n").c_str());
    }

    // Tキーでポータルタイプ切り替え
    if (input_->TriggerKey(DIK_T) && portalEffect_) {
        static int portalType = 0;
        portalType = (portalType + 1) % 2;
        portalEffect_->SetPortalType(portalType);
        OutputDebugStringA(("Portal type switched to: " + std::to_string(portalType) + "\n").c_str());
    }

    keyPressed_ = keyPressed;

    // 自動切り替えを無効化（エフェクトを永続表示）
    // エフェクトをずっと表示しておくため、自動切り替えをコメントアウト
    /*
    if (effectTimer_ >= 10.0f) {
        currentEffectMode_ = (currentEffectMode_ + 1) % 3;
        effectTimer_ = 0.0f;
        OutputDebugStringA("Auto-switched to next effect mode\n");
    }
    */
}

void GamePlayScene::UpdateDynamicEffects(float deltaTime) {
    // 魔法陣の動的効果
    if (magicCircleEffect_) {
        // 周期的なスケール変化
        float scaleModifier = 1.0f + 0.2f * std::sinf(animationTime_ * 1.5f);
        magicCircleEffect_->SetScale(scaleModifier);
        
        // 周期的な強度変化
        float intensityModifier = 0.8f + 0.4f * std::sinf(animationTime_ * 2.0f);
        magicCircleEffect_->SetIntensity(intensityModifier);
        
        // 位置の微細な移動（浮遊感）
        Vector3 magicPos = {-4.0f, 0.3f * std::sinf(animationTime_ * 0.8f), 0.0f};
        magicCircleEffect_->SetPosition(magicPos);
    }

    // ポータルの動的効果
    if (portalEffect_) {
        // 周期的なスケール変化（位相をずらす）
        float scaleModifier = 1.0f + 0.3f * std::sinf(animationTime_ * 1.8f + PI);
        portalEffect_->SetScale(scaleModifier);
        
        // 周期的な強度変化
        float intensityModifier = 0.7f + 0.5f * std::sinf(animationTime_ * 2.5f + PI / 2.0f);
        portalEffect_->SetIntensity(intensityModifier);
        
        // 位置の微細な移動（異なるパターン）
        Vector3 portalPos = {4.0f, 0.2f * std::cosf(animationTime_ * 1.2f), 0.0f};
        portalEffect_->SetPosition(portalPos);
    }
}

void GamePlayScene::ShowEffectUI() {
    // ImGuiウィンドウを中央に配置
    const float windowWidth = 450.0f;
    const float windowHeight = 650.0f;
    const float screenWidth = 1280.0f;  // 想定スクリーン幅
    const float screenHeight = 720.0f;  // 想定スクリーン高
    
    ImGui::SetNextWindowPos(ImVec2((screenWidth - windowWidth) * 0.5f, (screenHeight - windowHeight) * 0.5f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight), ImGuiCond_FirstUseEver);
    
    ImGui::Begin("エピックゲームエフェクトデモ");
    
    ImGui::Text("=== 次世代ビジュアルエフェクト ===");
    ImGui::Text("プロフェッショナルゲーム品質のエフェクト");
    ImGui::Separator();
    
    ImGui::Text("操作方法:");
    ImGui::Text("1キー: 魔法陣表示/非表示切替");
    ImGui::Text("2キー: ポータル表示/非表示切替");
    ImGui::Text("0キー: 両方表示");
    ImGui::Text("Tキー: ポータルタイプ切替");
    ImGui::Text("スペース: モード循環切替");
    ImGui::Text("ESC: タイトルに戻る");
    ImGui::Separator();
    
    ImGui::Text("カメラ操作:");
    ImGui::Text("WASD: カメラ移動 (前後左右)");
    ImGui::Text("QE: 上下移動");
    ImGui::Text("矢印キー: カメラ回転");
    ImGui::Separator();
    
    // 現在の状態表示
    ImGui::Text("現在の表示状態:");
    ImGui::Text("魔法陣: %s", magicCircleVisible_ ? "表示" : "非表示");
    ImGui::Text("ポータル: %s", portalVisible_ ? "表示" : "非表示");
    
    // デバッグ情報追加
    ImGui::Text("デバッグ情報:");
    ImGui::Text("魔法陣オブジェクト: %s", magicCircleEffect_ ? "有効" : "NULL");
    ImGui::Text("ポータルオブジェクト: %s", portalEffect_ ? "有効" : "NULL");
    if (magicCircleEffect_) {
        ImGui::Text("魔法陣IsVisible: %s", magicCircleEffect_->IsVisible() ? "true" : "false");
    }
    if (portalEffect_) {
        ImGui::Text("ポータルIsVisible: %s", portalEffect_->IsVisible() ? "true" : "false");
        ImGui::Text("ポータルIsActive: %s", portalEffect_->IsActive() ? "true" : "false");
    }
    ImGui::Text("自動切替: 無効（永続表示）");
    ImGui::Text("総実行時間: %.1f秒", animationTime_);
    
    ImGui::Separator();
    
    // エフェクト詳細
    ImGui::Text("魔法陣エフェクト:");
    ImGui::Text("- 5層回転リング構造");
    ImGui::Text("- 個別パルスパターン");
    ImGui::Text("- マルチカラールーンシステム");
    ImGui::Text("- 動的スケール&強度変化");
    
    ImGui::Separator();
    
    ImGui::Text("ポータルエフェクト:");
    ImGui::Text("- 8層深度レイヤーリング");
    ImGui::Text("- スパイラル歪みアニメーション");
    ImGui::Text("- 青/赤ポータルバリエーション");
    ImGui::Text("- 収束する3D深度表現");
    
    ImGui::Separator();
    
    // 技術仕様
    ImGui::Text("技術的機能:");
    ImGui::Text("- リアルタイムUVスクロール");
    ImGui::Text("- マルチレイヤー深度ソート");
    ImGui::Text("- 動的カラー変調");
    ImGui::Text("- 手続き型アニメーション");
    ImGui::Text("- アルファブレンディング効果");
    
    ImGui::Separator();
    
    // 手動制御
    ImGui::Text("手動制御:");
    
    // 魔法陣の表示/非表示切替
    if (ImGui::Button(magicCircleVisible_ ? "魔法陣（表示中）" : "魔法陣（非表示）")) {
        magicCircleVisible_ = !magicCircleVisible_;
        OutputDebugStringA(("Magic Circle visibility toggled: " + std::string(magicCircleVisible_ ? "ON" : "OFF") + "\n").c_str());
    }
    ImGui::SameLine();
    
    // ポータルの表示/非表示切替
    if (ImGui::Button(portalVisible_ ? "ポータル（表示中）" : "ポータル（非表示）")) {
        portalVisible_ = !portalVisible_;
        if (portalEffect_) {
            if (portalVisible_) {
                // 表示する時は強制的にアクティブ化
                portalEffect_->SetVisible(true);
                portalEffect_->StartEffect();
            } else {
                // 非表示する時は停止
                portalEffect_->SetVisible(false);
                portalEffect_->StopEffect();
            }
        }
        OutputDebugStringA(("Portal visibility toggled: " + std::string(portalVisible_ ? "ON" : "OFF") + "\n").c_str());
    }
    
    // 両方表示/非表示のボタン
    if (ImGui::Button("両方表示")) {
        magicCircleVisible_ = true;
        portalVisible_ = true;
        // エフェクトを強制的に有効化
        if (portalEffect_) {
            portalEffect_->SetVisible(true);
            portalEffect_->StartEffect();
        }
        OutputDebugStringA("Both effects enabled\n");
    }
    ImGui::SameLine();
    if (ImGui::Button("両方非表示")) {
        magicCircleVisible_ = false;
        portalVisible_ = false;
        // エフェクトを停止
        if (portalEffect_) {
            portalEffect_->SetVisible(false);
            portalEffect_->StopEffect();
        }
        OutputDebugStringA("Both effects disabled\n");
    }
    
    if (ImGui::Button("ポータルタイプ切替") && portalEffect_) {
        static int portalType = 0;
        portalType = (portalType + 1) % 2;
        portalEffect_->SetPortalType(portalType);
    }
    
    // エフェクト表示状態
    static bool persistentEffects = true;
    ImGui::Text("エフェクト表示: 永続的（自動切替無効）");
    ImGui::Text("手動で切り替え可能");
    
    // エフェクト強制有効化ボタン
    if (ImGui::Button("エフェクトを強制有効化")) {
        // エフェクトの可視性を強制的に有効化
        if (magicCircleEffect_) {
            // 魔法陣を再初期化
            magicCircleEffect_->CreateMagicCircle({-4.0f, 0.0f, 0.0f});
            magicCircleVisible_ = true;
        }
        if (portalEffect_) {
            // ポータルを再初期化し、強制的にアクティブ化
            portalEffect_->CreatePortal({4.0f, 0.0f, 0.0f});
            portalEffect_->SetVisible(true);
            portalEffect_->StartEffect();
            portalVisible_ = true;
        }
        OutputDebugStringA("Effects force-enabled and re-initialized\n");
    }
    
    // ポータル専用の強制アクティブ化ボタン
    if (ImGui::Button("ポータルを強制アクティブ化") && portalEffect_) {
        portalEffect_->SetVisible(true);
        portalEffect_->StartEffect();
        portalVisible_ = true;
        OutputDebugStringA("Portal force-activated\n");
    }
    
    ImGui::End();
}
