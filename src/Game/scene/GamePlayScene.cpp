#include "GamePlayScene.h"
#include "UnoEngine.h"
#include "RingManager.h"
#include "TextureManager.h"
#include "Vector3.h"
#include "Vector4.h"
#include <cmath>
#include <string>
#include <cassert>
#include <numbers>

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
    camera_->SetTranslate({ 0.0f, 0.0f, -15.0f });
    camera_->SetRotate({ 0.0f, 0.0f, 0.0f });
    camera_->Update();

    // RingManagerの初期化
    RingManager::GetInstance()->Initialize(dxCommon_);
    RingManager::GetInstance()->CreateCommonPresets();

    // gradationLine.pngテクスチャの事前読み込み
    TextureManager::GetInstance()->LoadTexture("Resources/particle/gradationLine.png");

    // エフェクト設定の準備
    effectSettings_ = {
        // 1. 基本的な円形エフェクト
        {
            {-5.0f, 3.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.5f, 1.5f, 1.0f},
            {1.0f, 1.0f, 1.0f, 1.0f}, 1.0f, 0.2f, 32,
            0.0f, 0.0f, false, "BasicRing", "基本的な円形エフェクト"
        },
        // 2. UVスクロール（横方向）
        {
            {0.0f, 3.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {2.0f, 2.0f, 1.0f},
            {0.8f, 1.0f, 1.0f, 1.0f}, 1.5f, 0.3f, 48,
            0.02f, 0.0f, false, "HorizontalScroll", "横方向UVスクロール"
        },
        // 3. UVスクロール（縦方向）
        {
            {5.0f, 3.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.5f, 1.5f, 1.0f},
            {1.0f, 0.8f, 1.0f, 1.0f}, 1.0f, 0.1f, 64,
            0.0f, 0.03f, false, "VerticalScroll", "縦方向UVスクロール"
        },
        // 4. 細いリング（境界線効果）
        {
            {-5.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {2.0f, 2.0f, 1.0f},
            {1.0f, 0.5f, 0.0f, 1.0f}, 1.0f, 0.9f, 64,
            0.01f, 0.0f, false, "ThinRing", "細いリング（境界線効果）"
        },
        // 5. 回転アニメーション
        {
            {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {3.0f, 3.0f, 1.0f},
            {1.0f, 1.0f, 0.5f, 0.8f}, 1.2f, 0.4f, 32,
            0.015f, 0.01f, true, "RotatingRing", "回転+UVスクロールエフェクト"
        },
        // 6. グラデーション効果
        {
            {5.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.8f, 1.8f, 1.0f},
            {0.5f, 1.0f, 0.5f, 1.0f}, 1.5f, 0.2f, 48,
            0.0f, -0.02f, false, "GradientRing", "グラデーション効果"
        },
        // 7. 大きなエフェクト（背景用）
        {
            {-5.0f, -3.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {4.0f, 4.0f, 1.0f},
            {0.3f, 0.3f, 1.0f, 0.4f}, 2.0f, 0.1f, 16,
            0.005f, 0.005f, true, "LargeBackground", "大きな背景エフェクト（脈動）"
        },
        // 8. 高速スクロール
        {
            {0.0f, -3.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.5f, 1.5f, 1.0f},
            {1.0f, 0.2f, 0.2f, 1.0f}, 0.8f, 0.3f, 32,
            0.05f, 0.0f, false, "HighSpeedScroll", "高速スクロールエフェクト"
        },
        // 9. 複合エフェクト
        {
            {5.0f, -3.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {2.0f, 2.0f, 1.0f},
            {1.0f, 1.0f, 1.0f, 1.0f}, 1.0f, 0.2f, 64,
            0.02f, 0.02f, true, "CombinedEffect", "複合エフェクト（色変化+回転）"
        }
    };

    // 円形エフェクトの作成
    CreateCircleEffects();

    // 初期化完了
    initialized_ = true;
    
    OutputDebugStringA("GamePlayScene: 円形エフェクト初期化完了\n");
    OutputDebugStringA("=== Circle Effect Gameplay Demo ===\n");
    OutputDebugStringA("- gradationLine.png使用の真の円形エフェクト\n");
    OutputDebugStringA("- AddressV=CLAMP設定で中心の白線を回避\n");
    OutputDebugStringA("- 9種類の異なるエフェクトパターン\n");
    OutputDebugStringA("- リアルタイムUVスクロールアニメーション\n");
    OutputDebugStringA("=====================================\n");
}

void GamePlayScene::Update() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;

    // アニメーションタイマー更新
    animationTime_ += 1.0f / 60.0f; // 60FPS想定

    // ESCキーでタイトルシーンに戻る
    if (input_->TriggerKey(DIK_ESCAPE)) {
        sceneManager_->ChangeScene("Title");
    }

    // エフェクト制御
    HandleEffectSwitching();
    UpdateCircleEffects();

    // 全てのエフェクトを更新
    for (auto& effect : circleEffects_) {
        if (effect) {
            effect->Update();
        }
    }

    // カメラの更新
    camera_->Update();
}

void GamePlayScene::Draw() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;

    // 全ての円形エフェクトを描画
    for (auto& effect : circleEffects_) {
        if (effect && effect->IsVisible()) {
            effect->Draw();
        }
    }

    // UI表示
    ShowEffectUI();
}

void GamePlayScene::Finalize() {
    circleEffects_.clear();
    OutputDebugStringA("GamePlayScene: 円形エフェクト終了処理完了\n");
}

void GamePlayScene::CreateCircleEffects() {
    circleEffects_.clear();

    // 設定に基づいてエフェクトを作成
    for (size_t i = 0; i < effectSettings_.size(); ++i) {
        const auto& settings = effectSettings_[i];

        auto effect = std::make_unique<CircleEffect>();
        effect->Initialize(dxCommon_, spriteCommon_);
        effect->SetCamera(camera_);

        // エフェクトの作成
        effect->CreateCircleEffect(settings.outerRadius, settings.innerRadius, settings.divisions);

        // Transform設定
        effect->SetPosition(settings.position);
        effect->SetRotation(settings.rotation);
        effect->SetScale(settings.scale);
        effect->SetColor(settings.color);

        // UVアニメーション設定
        if (settings.uvScrollSpeedU != 0.0f || settings.uvScrollSpeedV != 0.0f) {
            effect->StartUVAnimation(settings.uvScrollSpeedU, settings.uvScrollSpeedV);
        }

        circleEffects_.push_back(std::move(effect));

        OutputDebugStringA(("GamePlayScene - Created: " + settings.description + "\n").c_str());
    }

    OutputDebugStringA(("GamePlayScene::CreateCircleEffects - " + 
                      std::to_string(circleEffects_.size()) + " effects created\n").c_str());
}

void GamePlayScene::UpdateCircleEffects() {
    // 回転アニメーション（インデックス4の回転エフェクト）
    if (circleEffects_.size() > 4 && circleEffects_[4] && effectSettings_[4].enableAnimation) {
        float rotationSpeed = 1.0f; // ラジアン/秒
        Vector3 rotation = circleEffects_[4]->GetRotation();
        rotation.z += rotationSpeed * (1.0f / 60.0f);
        circleEffects_[4]->SetRotation(rotation);
    }

    // 脈動効果（インデックス6の大きなエフェクト）
    if (circleEffects_.size() > 6 && circleEffects_[6] && effectSettings_[6].enableAnimation) {
        float pulseScale = 1.0f + 0.3f * std::sin(animationTime_ * 2.0f);
        Vector3 baseScale = effectSettings_[6].scale;
        circleEffects_[6]->SetScale({baseScale.x * pulseScale, baseScale.y * pulseScale, baseScale.z});
    }

    // 色変化効果（インデックス8の複合エフェクト）
    if (circleEffects_.size() > 8 && circleEffects_[8] && effectSettings_[8].enableAnimation) {
        float colorPhase = animationTime_ * 3.0f;
        Vector4 color = {
            0.5f + 0.5f * std::sin(colorPhase),
            0.5f + 0.5f * std::sin(colorPhase + 2.094f), // 120度位相差
            0.5f + 0.5f * std::sin(colorPhase + 4.188f), // 240度位相差
            0.8f
        };
        circleEffects_[8]->SetColor(color);

        // 同時に回転も
        Vector3 rotation = circleEffects_[8]->GetRotation();
        rotation.z += 0.5f * (1.0f / 60.0f);
        circleEffects_[8]->SetRotation(rotation);
    }

    // 全エフェクトの可視性制御
    for (size_t i = 0; i < circleEffects_.size(); ++i) {
        if (circleEffects_[i]) {
            // 現在のエフェクト、または全表示モードの場合に表示
            bool shouldShow = (currentEffect_ == static_cast<int>(i)) || (currentEffect_ == static_cast<int>(circleEffects_.size()));
            circleEffects_[i]->SetVisible(shouldShow);
        }
    }
}

void GamePlayScene::HandleEffectSwitching() {
    // タイマー更新
    effectTimer_ += 1.0f / 60.0f;

    // 数字キーでエフェクト切り替え
    bool keyPressed = false;
    int newEffect = currentEffect_;

    for (int i = 0; i < 9; ++i) {
        if (input_->PushKey(DIK_1 + i)) {
            newEffect = i;
            keyPressed = true;
            break;
        }
    }

    // 0キーで全表示
    if (input_->PushKey(DIK_0)) {
        newEffect = static_cast<int>(circleEffects_.size()); // 全表示モード
        keyPressed = true;
    }

    // Spaceキーで次のエフェクトに切り替え
    if (input_->TriggerKey(DIK_SPACE)) {
        newEffect = (currentEffect_ + 1) % (static_cast<int>(circleEffects_.size()) + 1);
        keyPressed = true;
    }

    if (keyPressed && !keyPressed_) {
        currentEffect_ = newEffect;
        effectTimer_ = 0.0f;
        
        OutputDebugStringA(("GamePlayScene - Switched to effect: " + std::to_string(currentEffect_) + "\n").c_str());
    }

    keyPressed_ = keyPressed;

    // 自動切り替え（8秒ごと）
    if (effectTimer_ >= 8.0f) {
        currentEffect_ = (currentEffect_ + 1) % (static_cast<int>(circleEffects_.size()) + 1);
        effectTimer_ = 0.0f;
        
        OutputDebugStringA("GamePlayScene - Auto-switched to next effect\n");
    }
}

void GamePlayScene::ShowEffectUI() {
    ImGui::Begin("円形エフェクト - GamePlay Demo");
    
    ImGui::Text("=== Circle Effect Demonstration ===");
    ImGui::Text("gradationLine.png + Ring Primitive");
    ImGui::Separator();
    
    ImGui::Text("操作説明:");
    ImGui::Text("1-9キー: 個別エフェクト表示");
    ImGui::Text("0キー: 全エフェクト同時表示");
    ImGui::Text("SPACEキー: 次のエフェクトに切り替え");
    ImGui::Text("ESCキー: タイトルに戻る");
    ImGui::Separator();
    
    // 現在のエフェクト表示
    if (currentEffect_ < static_cast<int>(effectSettings_.size())) {
        const auto& settings = effectSettings_[currentEffect_];
        ImGui::Text("現在のエフェクト: %s", settings.description.c_str());
        ImGui::Text("- 外径: %.2f, 内径: %.2f", settings.outerRadius, settings.innerRadius);
        ImGui::Text("- 分割数: %d", settings.divisions);
        ImGui::Text("- UVスクロール: U=%.3f, V=%.3f", settings.uvScrollSpeedU, settings.uvScrollSpeedV);
        ImGui::Text("- アニメーション: %s", settings.enableAnimation ? "有効" : "無効");
    } else {
        ImGui::Text("現在のエフェクト: 全エフェクト同時表示");
    }
    
    ImGui::Text("次の切り替えまで: %.1f秒", 8.0f - effectTimer_);
    ImGui::Text("総動作時間: %.1f秒", animationTime_);
    
    ImGui::Separator();
    
    // エフェクトリスト
    ImGui::Text("エフェクト一覧:");
    for (size_t i = 0; i < effectSettings_.size(); ++i) {
        bool isSelected = (currentEffect_ == static_cast<int>(i));
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); // 黄色
        }
        
        ImGui::Text("%zu. %s", i + 1, effectSettings_[i].description.c_str());
        
        if (isSelected) {
            ImGui::PopStyleColor();
        }
    }
    
    ImGui::Separator();
    
    // 手動ボタン
    ImGui::Text("手動切り替え:");
    for (size_t i = 0; i < effectSettings_.size(); ++i) {
        if (i % 3 != 0) ImGui::SameLine();
        
        if (ImGui::Button(std::to_string(i + 1).c_str())) {
            currentEffect_ = static_cast<int>(i);
            effectTimer_ = 0.0f;
        }
    }
    
    if (ImGui::Button("全表示")) {
        currentEffect_ = static_cast<int>(circleEffects_.size());
        effectTimer_ = 0.0f;
    }
    
    ImGui::Separator();
    ImGui::Text("技術詳細:");
    ImGui::Text("- SamplerのAddressV = CLAMP");
    ImGui::Text("- UV座標: 外側v=0.0, 内側v=1.0");
    ImGui::Text("- UVScrollによる動的アニメーション");
    ImGui::Text("- Ringプリミティブによる効率的描画");
    
    ImGui::End();
}
