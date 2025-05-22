// src/Engine/Graphics/RingSample.cpp
#include "RingSample.h"
#include <cmath>
#include <numbers>

RingSample::RingSample() 
    : dxCommon_(nullptr)
    , effectTime_(0.0f)
    , scrollSpeed_(1.0f)
    , billboardEnabled_(false) {
}

RingSample::~RingSample() {}

void RingSample::Initialize(DirectXCommon* dxCommon) {
    dxCommon_ = dxCommon;

    // RingManagerの初期化
    RingManager::GetInstance()->Initialize(dxCommon);

    // 共通プリセットの作成
    RingManager::GetInstance()->CreateCommonPresets();

    // サンプルエフェクトの作成
    CreateBasicRingEffect();
    CreateStoneEffect();
    CreateScrollingRingEffect();
    CreateParticleRingEffect();
    CreateRingVariations();

    // 使用例の説明をログに出力
    LogUsageExamples();
}

void RingSample::Update() {
    effectTime_ += 1.0f / 60.0f; // 60FPS想定

    // UVScrollの更新（エフェクトの動的変化）
    // 実際の実装では、シェーダーやマテリアルの設定で行う
}

void RingSample::Draw() {
    // 実際の描画処理
    // Object3Dやレンダリングパイプラインを使用して描画
    // この例では構造のみ示す
}

void RingSample::CreateBasicRingEffect() {
    // 基本的なRingエフェクト
    basicRing_ = std::make_unique<Ring>();
    basicRing_->Initialize(dxCommon_);
    basicRing_->Generate(1.0f, 0.2f, 32);

    // RingManagerからも取得可能
    Ring* managedRing = RingManager::GetInstance()->GetRing("effect_basic");
    if (managedRing) {
        // managedRingを使用した処理
    }
}

void RingSample::CreateStoneEffect() {
    // 石のようなエフェクト（前回のPlaneと組み合わせる）
    // ビルボード有りとビルボード無しの組み合わせ
    
    // 1. ビルボード有りのRing（常にカメラを向く）
    auto billboardRing = RingManager::GetInstance()->CreateAdvancedRing(
        "stone_effect_billboard",
        1.5f,  // 外径
        0.8f,  // 内径
        64,    // 高解像度
        0.0f, 2.0f * std::numbers::pi_v<float>,
        AdvancedRing::UVDirection::Horizontal,
        {1.0f, 1.0f, 1.0f, 0.8f}, // 半透明
        {1.0f, 1.0f, 1.0f, 0.8f}
    );

    // 2. ビルボード無しのRing（地面に固定）
    auto fixedRing = RingManager::GetInstance()->CreateAdvancedRing(
        "stone_effect_fixed",
        2.0f,  // より大きなサイズ
        0.5f,
        32,
        0.0f, 2.0f * std::numbers::pi_v<float>,
        AdvancedRing::UVDirection::Horizontal,
        {0.8f, 0.6f, 0.4f, 1.0f}, // 石の色
        {0.6f, 0.4f, 0.2f, 1.0f}
    );

    billboardEnabled_ = true;
}

void RingSample::CreateScrollingRingEffect() {
    // UVScrollを使った動的エフェクト
    scrollRing_ = std::make_unique<AdvancedRing>();
    scrollRing_->Initialize(dxCommon_);
    
    // 縦方向スクロール用のRing
    scrollRing_->Generate(
        1.0f, 0.3f, 48,
        0.0f, 2.0f * std::numbers::pi_v<float>,
        AdvancedRing::UVDirection::Vertical, // v方向スクロール
        {1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f}
    );

    // gradationLine.pngテクスチャを使用することを想定
    // シェーダーでUVScrollを適用:
    // staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    // または WRAP を使用してスクロール効果を実現
}

void RingSample::CreateParticleRingEffect() {
    // ParticleSystemとの組み合わせ例
    particleRing_ = std::make_unique<AdvancedRing>();
    particleRing_->Initialize(dxCommon_);

    // Emitterのタイミングに合わせた設定
    particleRing_->Generate(
        0.8f, 0.1f, 16, // 粗い分割でパフォーマンス重視
        0.0f, 2.0f * std::numbers::pi_v<float>,
        AdvancedRing::UVDirection::Horizontal,
        {1.0f, 0.5f, 0.0f, 1.0f}, // オレンジ色（炎のエフェクト）
        {1.0f, 1.0f, 0.0f, 1.0f}  // 黄色
    );

    // ParticleSystemでは以下のような設定を想定:
    // - 点の動きを制御するもの
    // - その点の位置や姿勢で何を描画しても良い応用が効く
    // - Primitiveを含めてParticleSystemと呼ばれることも多い
    // - Modelを適用したものはModelParticleなどと呼ばれる
}

void RingSample::CreateRingVariations() {
    // 様々なRingバリエーションの作成例

    // 1. 細いRing（境界線効果）
    RingManager::GetInstance()->CreateAdvancedRing(
        "thin_ring",
        1.0f, 0.95f, 64,
        0.0f, 2.0f * std::numbers::pi_v<float>,
        AdvancedRing::UVDirection::Horizontal,
        {0.0f, 1.0f, 1.0f, 1.0f}, // シアン
        {0.0f, 1.0f, 1.0f, 1.0f}
    );

    // 2. 部分円Ring（角度制限）
    RingManager::GetInstance()->CreateAdvancedRing(
        "partial_ring",
        1.2f, 0.4f, 24,
        std::numbers::pi_v<float> / 4.0f,    // 45度開始
        std::numbers::pi_v<float> * 7.0f / 4.0f, // 315度終了（270度の弧）
        AdvancedRing::UVDirection::Horizontal,
        {1.0f, 0.0f, 1.0f, 1.0f}, // マゼンタ
        {1.0f, 0.0f, 1.0f, 1.0f}
    );

    // 3. グラデーション効果Ring
    RingManager::GetInstance()->CreateAdvancedRing(
        "gradient_ring",
        1.5f, 0.2f, 48,
        0.0f, 2.0f * std::numbers::pi_v<float>,
        AdvancedRing::UVDirection::Horizontal,
        {1.0f, 0.0f, 0.0f, 1.0f}, // 外側: 赤
        {0.0f, 0.0f, 1.0f, 0.5f}  // 内側: 半透明青
    );

    // 4. 高解像度Ring（滑らかなアニメーション用）
    RingManager::GetInstance()->CreateAdvancedRing(
        "smooth_ring",
        1.0f, 0.3f, 128, // 高分割
        0.0f, 2.0f * std::numbers::pi_v<float>,
        AdvancedRing::UVDirection::Horizontal,
        {1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f}
    );
}

void RingSample::LogUsageExamples() {
    // 使用例の説明をデバッグログに出力
    OutputDebugStringA("=== Ring Primitive Usage Examples ===\n");
    OutputDebugStringA("1. Basic Ring: 基本的なリング形状でエフェクトに最適\n");
    OutputDebugStringA("2. Stone Effect: PlaneとRingの組み合わせで石のようなエフェクト\n");
    OutputDebugStringA("3. UV Scroll: gradationLine.pngと組み合わせてスクロール効果\n");
    OutputDebugStringA("4. Particle System: EmitterやParticleと組み合わせて複雑なエフェクト\n");
    OutputDebugStringA("5. Billboard: カメラに常に向くビルボード効果\n");
    OutputDebugStringA("6. Variations: 細いリング、部分円、グラデーションなど\n");
    OutputDebugStringA("\n--- Technical Notes ---\n");
    OutputDebugStringA("・UVScrollはEffectに欠かせない機能\n");
    OutputDebugStringA("・市販のゲームでもよく利用されている\n");
    OutputDebugStringA("・AddressVをCLAMPにすると中心に白い丸が見える\n");
    OutputDebugStringA("・各種パラメータ（簡単なAnimation）に組み合わせて応用\n");
    OutputDebugStringA("・ゲーム制作でうまく応用してください\n");
    OutputDebugStringA("=======================================\n");
}