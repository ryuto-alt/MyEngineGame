#include "GamePlayScene.h"
#include "ParticleManager.h"
#include "Mymath.h"
#include <random>

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
    camera_->SetTranslate({ 0.0f, 0.0f, -10.0f });
    camera_->SetRotate({ 0.0f, 0.0f, 0.0f });
    camera_->Update();

    // パーティクルグループの作成
    ParticleManager::GetInstance()->CreateParticleGroup("basicParticle", "Resources/particle/particle.png");
    ParticleManager::GetInstance()->CreateParticleGroup("hitEffect", "Resources/particle/explosion.png");
    ParticleManager::GetInstance()->CreateParticleGroup("starEffect", "Resources/particle/star.png");
    ParticleManager::GetInstance()->CreateParticleGroup("radialBurst", "Resources/particle/particle.png"); // 新しい放射状エフェクト

    // エフェクトエミッタの作成
    CreateEffectEmitters();

    // 初期化完了
    initialized_ = true;
    OutputDebugStringA("GamePlayScene: 初期化完了\n");
}

void GamePlayScene::CreateEffectEmitters() {
    // 1. 基本パーティクル（サイズ変更 - 縦長のものが発生）
    auto basicEmitter = std::make_unique<ParticleEmitter>(
        "basicParticle",
        Vector3{ -3.0f, 0.0f, 0.0f },  // 左側に配置
        5,    // 発生数
        2.0f, // 発生頻度
        Vector3{ -1.0f, 1.0f, -1.0f },   // 速度最小（上方向）
        Vector3{ 1.0f, 3.0f, 1.0f },     // 速度最大（上方向強め）
        Vector3{ 0.0f, 0.0f, 0.0f },     // 加速度最小
        Vector3{ 0.0f, -9.8f, 0.0f },    // 加速度最大（重力）
        0.1f,   // 開始サイズ最小（縦長特性を強調）
        1.5f,   // 開始サイズ最大
        0.05f,  // 終了サイズ最小（小さくして線形的に）
        0.1f,   // 終了サイズ最大
        Vector4{ 0.3f, 0.7f, 1.0f, 1.0f }, // 開始色最小（淺い青）
        Vector4{ 0.8f, 0.9f, 1.0f, 1.0f }, // 開始色最大（青白）
        Vector4{ 0.0f, 0.0f, 1.0f, 0.0f }, // 終了色最小（透明青）
        Vector4{ 1.0f, 1.0f, 1.0f, 0.0f }, // 終了色最大（透明白）
        0.0f,   // 回転最小
        0.0f,   // 回転最大
        0.0f,   // 回転速度最小
        0.0f,   // 回転速度最大
        1.5f,   // 寿命最小
        3.0f    // 寿命最大
    );
    particleEmitters_.push_back(std::move(basicEmitter));

    // 2. ヒットエフェクト（放射状の光線エフェクト）
    auto hitEmitter = std::make_unique<ParticleEmitter>(
        "hitEffect",
        Vector3{ 0.0f, 0.0f, 0.0f },    // 中央に配置
        32,   // 発生数（放射状にするため多め）
        1000.0f, // 発生頻度（瞬間的に大量発生）
        Vector3{ -1.5f, -1.5f, -0.5f }, // 速度最小（ゆっくり拡散）
        Vector3{ 1.5f, 1.5f, 0.5f },    // 速度最大（ゆっくり拡散）
        Vector3{ 0.0f, 0.0f, 0.0f },    // 加速度最小（減速なし）
        Vector3{ 0.0f, 0.0f, 0.0f },    // 加速度最大（減速なし）
        1.5f,   // 開始サイズ最小（光線の太さ）
        2.5f,   // 開始サイズ最大
        0.1f,   // 終了サイズ最小（細くなる）
        0.3f,   // 終了サイズ最大
        Vector4{ 1.0f, 1.0f, 1.0f, 1.0f }, // 開始色最小（明るい白）
        Vector4{ 1.0f, 1.0f, 0.95f, 1.0f }, // 開始色最大（少し黄色がかった白）
        Vector4{ 1.0f, 0.9f, 0.7f, 0.0f }, // 終了色最小（温かい色でフェード）
        Vector4{ 1.0f, 1.0f, 1.0f, 0.0f }, // 終了色最大（透明白）
        0.0f,   // 回転最小
        0.0f,   // 回転最大
        0.0f,   // 回転速度最小
        0.0f,   // 回転速度最大
        1.5f,   // 寿命最小（ゆっくりフェード）
        2.5f    // 寿命最大
    );
    particleEmitters_.push_back(std::move(hitEmitter));

    // 3. 星型エフェクト（ランダムZ回転）
    auto starEmitter = std::make_unique<ParticleEmitter>(
        "starEffect",
        Vector3{ 3.0f, 0.0f, 0.0f },    // 右側に配置
        8,    // 発生数（星型にするため多め）
        3.0f, // 発生頻度
        Vector3{ -1.5f, -1.5f, -0.5f }, // 速度最小
        Vector3{ 1.5f, 1.5f, 0.5f },    // 速度最大
        Vector3{ 0.0f, 0.0f, 0.0f },    // 加速度最小
        Vector3{ 0.0f, -1.0f, 0.0f },   // 加速度最大（軽い重力）
        0.3f,   // 開始サイズ最小
        0.8f,   // 開始サイズ最大
        0.0f,   // 終了サイズ最小
        0.1f,   // 終了サイズ最大
        Vector4{ 1.0f, 0.8f, 0.0f, 1.0f }, // 開始色最小（黄金色）
        Vector4{ 1.0f, 1.0f, 0.0f, 1.0f }, // 開始色最大（黄色）
        Vector4{ 1.0f, 0.5f, 0.0f, 0.0f }, // 終了色最小（透明オレンジ）
        Vector4{ 1.0f, 1.0f, 1.0f, 0.0f }, // 終了色最大（透明白）
        0.0f,     // 回転最小
        6.28f,    // 回転最大（360度）
        -10.0f,   // 回転速度最小（反時計回り）
        10.0f,    // 回転速度最大（時計回り）
        1.0f,     // 寿命最小
        2.5f      // 寿命最大
    );
    particleEmitters_.push_back(std::move(starEmitter));

    // 4. 放射状バーストエフェクト（画像のような光線エフェクト）
    auto radialEmitter = std::make_unique<ParticleEmitter>(
        "radialBurst",
        Vector3{ -3.0f, 2.0f, 0.0f },   // 左上に配置
        48,   // 発生数（密度を上げて放射状に）
        2000.0f, // 発生頻度（一瞬で全部発生）
        Vector3{ -2.0f, -2.0f, -1.0f }, // 速度最小（さらにゆっくり）
        Vector3{ 2.0f, 2.0f, 1.0f },    // 速度最大（さらにゆっくり）
        Vector3{ 0.0f, 0.0f, 0.0f },     // 加速度最小（減速なし）
        Vector3{ 0.0f, 0.0f, 0.0f },     // 加速度最大（減速なし）
        2.0f,   // 開始サイズ最小（より太い光線）
        3.0f,   // 開始サイズ最大
        0.05f,  // 終了サイズ最小（非常に細くなる）
        0.1f,   // 終了サイズ最大
        Vector4{ 1.0f, 1.0f, 1.0f, 1.0f }, // 開始色最小（純白）
        Vector4{ 1.0f, 1.0f, 0.98f, 1.0f }, // 開始色最大（わずかに温かい白）
        Vector4{ 1.0f, 0.8f, 0.5f, 0.0f }, // 終了色最小（温かいオレンジでフェード）
        Vector4{ 1.0f, 1.0f, 1.0f, 0.0f }, // 終了色最大（透明白）
        0.0f,   // 回転最小
        0.0f,   // 回転最大
        0.0f,   // 回転速度最小
        0.0f,   // 回転速度最大
        2.0f,   // 寿命最小（より長いフェード）
        3.0f    // 寿命最大
    );
    particleEmitters_.push_back(std::move(radialEmitter));

    // 初期状態ではすべてのエミッタを停止
    for (auto& emitter : particleEmitters_) {
        emitter->SetEmitting(false);
    }
}

void GamePlayScene::Update() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;

    // ESCキーでタイトルシーンに戻る
    if (input_->TriggerKey(DIK_ESCAPE)) {
        sceneManager_->ChangeScene("Title");
    }

    // エフェクト切り替え制御
    HandleEffectSwitching();

    // エフェクトの更新
    for (auto& emitter : particleEmitters_) {
        emitter->Update();
    }

    // パーティクルマネージャーの更新
    ParticleManager::GetInstance()->Update(camera_);

    // カメラの更新
    camera_->Update();
}

void GamePlayScene::HandleEffectSwitching() {
    // タイマー更新
    effectTimer_ += 1.0f / 60.0f;

    // 数字キーでエフェクト切り替え
    bool key1Pressed = input_->PushKey(DIK_1);
    bool key2Pressed = input_->PushKey(DIK_2);
    bool key3Pressed = input_->PushKey(DIK_3);
    bool key4Pressed = input_->PushKey(DIK_4);
    bool key5Pressed = input_->PushKey(DIK_5);

    bool anyKeyPressed = key1Pressed || key2Pressed || key3Pressed || key4Pressed || key5Pressed;

    if (anyKeyPressed && !keyPressed_) {
        // すべてのエミッタを停止
        for (auto& emitter : particleEmitters_) {
            emitter->SetEmitting(false);
        }

        if (key1Pressed) {
            // 1キー: 基本パーティクル（サイズ変更）
            currentEffect_ = 0;
            particleEmitters_[0]->SetEmitting(true);
        } else if (key2Pressed) {
            // 2キー: ヒットエフェクト
            currentEffect_ = 1;
            particleEmitters_[1]->SetEmitting(true);
        } else if (key3Pressed) {
            // 3キー: 星型エフェクト
            currentEffect_ = 2;
            particleEmitters_[2]->SetEmitting(true);
        } else if (key4Pressed) {
            // 4キー: 放射状バーストエフェクト
            currentEffect_ = 3;
            particleEmitters_[3]->SetEmitting(true);
        } else if (key5Pressed) {
            // 5キー: すべて同時再生
            currentEffect_ = 4;
            for (auto& emitter : particleEmitters_) {
                emitter->SetEmitting(true);
            }
        }

        effectTimer_ = 0.0f;
    }

    keyPressed_ = anyKeyPressed;

    // 自動切り替え（5秒ごと）
    if (effectTimer_ >= 5.0f) {
        // すべてのエミッタを停止
        for (auto& emitter : particleEmitters_) {
            emitter->SetEmitting(false);
        }

        // 次のエフェクトに切り替え
        currentEffect_ = (currentEffect_ + 1) % 5;

        if (currentEffect_ < 4) {
            particleEmitters_[currentEffect_]->SetEmitting(true);
        } else {
            // すべて同時再生
            for (auto& emitter : particleEmitters_) {
                emitter->SetEmitting(true);
            }
        }

        effectTimer_ = 0.0f;
    }
}

void GamePlayScene::Draw() {
    // 初期化されていない場合はスキップ
    if (!initialized_) return;

    // パーティクルの描画
    ParticleManager::GetInstance()->Draw();

    // UI表示
    ImGui::Begin("パーティクルエフェクトデモ");
    ImGui::Text("操作説明:");
    ImGui::Text("1キー - 基本パーティクル（サイズ変更）");
    ImGui::Text("2キー - ヒットエフェクト");
    ImGui::Text("3キー - 星型エフェクト（ランダム回転）");
    ImGui::Text("4キー - 放射状バースト（光線）");
    ImGui::Text("5キー - すべて同時再生");
    ImGui::Text("ESC - タイトルに戻る");
    ImGui::Separator();
    
    const char* effectNames[] = {
        "基本パーティクル",
        "ヒットエフェクト", 
        "星型エフェクト",
        "放射状バースト",
        "すべて同時"
    };
    ImGui::Text("現在のエフェクト: %s", effectNames[currentEffect_]);
    ImGui::Text("次の切り替えまで: %.1f秒", 5.0f - effectTimer_);
    
    // パーティクル数の表示
    ImGui::Separator();
    ImGui::Text("パーティクル数:");
    ImGui::Text("基本: %d", ParticleManager::GetInstance()->GetParticleCount("basicParticle"));
    ImGui::Text("ヒット: %d", ParticleManager::GetInstance()->GetParticleCount("hitEffect"));
    ImGui::Text("星型: %d", ParticleManager::GetInstance()->GetParticleCount("starEffect"));
    ImGui::Text("放射状: %d", ParticleManager::GetInstance()->GetParticleCount("radialBurst"));
    ImGui::End();
}

void GamePlayScene::Finalize() {
    // パーティクルエミッタのクリア
    particleEmitters_.clear();
    
    OutputDebugStringA("GamePlayScene: 終了処理完了\n");
}