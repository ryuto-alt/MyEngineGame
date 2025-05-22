#pragma once
#include "Ring.h"
#include "AdvancedRing.h"
#include <memory>
#include <unordered_map>
#include <string>

/// <summary>
/// Ringプリミティブの管理クラス
/// 複数のRingインスタンスを効率的に管理し、
/// エフェクトやParticleSystemとの組み合わせを簡単にする
/// </summary>
class RingManager {
public:
    /// <summary>
    /// シングルトンインスタンス取得
    /// </summary>
    /// <returns>RingManagerのインスタンス</returns>
    static RingManager* GetInstance();

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="dxCommon">DirectXCommon</param>
    void Initialize(DirectXCommon* dxCommon);

    /// <summary>
    /// 基本Ringの作成
    /// </summary>
    /// <param name="name">Ring名</param>
    /// <param name="outerRadius">外径</param>
    /// <param name="innerRadius">内径</param>
    /// <param name="divisions">分割数</param>
    /// <returns>作成されたRingのポインタ</returns>
    Ring* CreateRing(
        const std::string& name,
        float outerRadius = 1.0f,
        float innerRadius = 0.2f,
        uint32_t divisions = 32
    );

    /// <summary>
    /// 高機能Ringの作成
    /// </summary>
    /// <param name="name">Ring名</param>
    /// <param name="outerRadius">外径</param>
    /// <param name="innerRadius">内径</param>
    /// <param name="divisions">分割数</param>
    /// <param name="startAngle">開始角度</param>
    /// <param name="endAngle">終了角度</param>
    /// <param name="uvDirection">UV方向</param>
    /// <param name="outerColor">外側の色</param>
    /// <param name="innerColor">内側の色</param>
    /// <returns>作成されたAdvancedRingのポインタ</returns>
    AdvancedRing* CreateAdvancedRing(
        const std::string& name,
        float outerRadius = 1.0f,
        float innerRadius = 0.2f,
        uint32_t divisions = 32,
        float startAngle = 0.0f,
        float endAngle = 2.0f * 3.14159265f,
        AdvancedRing::UVDirection uvDirection = AdvancedRing::UVDirection::Horizontal,
        const Vector4& outerColor = {1.0f, 1.0f, 1.0f, 1.0f},
        const Vector4& innerColor = {1.0f, 1.0f, 1.0f, 1.0f}
    );

    /// <summary>
    /// Ringの取得
    /// </summary>
    /// <param name="name">Ring名</param>
    /// <returns>Ringのポインタ（見つからない場合はnullptr）</returns>
    Ring* GetRing(const std::string& name);

    /// <summary>
    /// AdvancedRingの取得
    /// </summary>
    /// <param name="name">Ring名</param>
    /// <returns>AdvancedRingのポインタ（見つからない場合はnullptr）</returns>
    AdvancedRing* GetAdvancedRing(const std::string& name);

    /// <summary>
    /// よく使用されるRingプリセットの作成
    /// </summary>
    void CreateCommonPresets();

    /// <summary>
    /// 全てのRingをクリア
    /// </summary>
    void Clear();

private:
    RingManager() = default;
    ~RingManager() = default;
    RingManager(const RingManager&) = delete;
    RingManager& operator=(const RingManager&) = delete;

    // 基本Ring管理
    std::unordered_map<std::string, std::unique_ptr<Ring>> rings_;
    // 高機能Ring管理
    std::unordered_map<std::string, std::unique_ptr<AdvancedRing>> advancedRings_;
    // DirectXCommon
    DirectXCommon* dxCommon_ = nullptr;
};