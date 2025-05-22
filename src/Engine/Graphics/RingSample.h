#pragma once
#include "Ring.h"
#include "AdvancedRing.h"
#include "RingManager.h"
#include "Object3d.h"
#include "DirectXCommon.h"
#include <memory>

/// <summary>
/// Ringプリミティブの使用例を示すサンプルクラス
/// スライドで説明されたような石のエフェクトやParticleSystemとの組み合わせ例
/// </summary>
class RingSample {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    RingSample();

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~RingSample();

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="dxCommon">DirectXCommon</param>
    void Initialize(DirectXCommon* dxCommon);

    /// <summary>
    /// 更新
    /// </summary>
    void Update();

    /// <summary>
    /// 描画
    /// </summary>
    void Draw();

    /// <summary>
    /// 基本的なRingエフェクトの例
    /// </summary>
    void CreateBasicRingEffect();

    /// <summary>
    /// 石のようなエフェクトの例（PlaneとRingの組み合わせ）
    /// </summary>
    void CreateStoneEffect();

    /// <summary>
    /// UVScrollを使った動的エフェクトの例
    /// </summary>
    void CreateScrollingRingEffect();

    /// <summary>
    /// パーティクルシステムとの組み合わせ例
    /// </summary>
    void CreateParticleRingEffect();

    /// <summary>
    /// 様々なRingバリエーションの例
    /// </summary>
    void CreateRingVariations();

private:
    // DirectXCommon
    DirectXCommon* dxCommon_;

    // Ring関連
    std::unique_ptr<Ring> basicRing_;
    std::unique_ptr<AdvancedRing> scrollRing_;
    std::unique_ptr<AdvancedRing> particleRing_;

    // エフェクト用の変数
    float effectTime_;
    float scrollSpeed_;
    bool billboardEnabled_;

    // 使用例の説明文字列
    void LogUsageExamples();
};