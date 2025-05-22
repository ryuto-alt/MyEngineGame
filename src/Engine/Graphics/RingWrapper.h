#pragma once
#include "Ring.h"
#include "AdvancedRing.h"
#include "Object3d.h"
#include "Model.h"
#include "DirectXCommon.h"
#include <memory>

/// <summary>
/// RingをObject3Dとして描画するためのラッパークラス
/// </summary>
class RingWrapper {
public:
    /// <summary>
    /// コンストラクタ
    /// </summary>
    RingWrapper();
    
    /// <summary>
    /// デストラクタ
    /// </summary>
    ~RingWrapper();
    
    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="dxCommon">DirectXCommon</param>
    /// <param name="spriteCommon">SpriteCommon</param>
    void Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon);
    
    /// <summary>
    /// 基本Ringの設定
    /// </summary>
    /// <param name="ring">Ring</param>
    void SetRing(Ring* ring);
    
    /// <summary>
    /// 拡張Ringの設定
    /// </summary>
    /// <param name="ring">AdvancedRing</param>
    void SetAdvancedRing(AdvancedRing* ring);
    
    /// <summary>
    /// 更新
    /// </summary>
    void Update();
    
    /// <summary>
    /// 描画
    /// </summary>
    void Draw();
    
    /// <summary>
    /// カメラの設定
    /// </summary>
    /// <param name="camera">Camera</param>
    void SetCamera(Camera* camera);
    
    // Transform関連
    void SetPosition(const Vector3& position);
    void SetRotation(const Vector3& rotation);
    void SetScale(const Vector3& scale);
    void SetColor(const Vector4& color);
    
    const Vector3& GetPosition() const;
    const Vector3& GetRotation() const;
    const Vector3& GetScale() const;

private:
    /// <summary>
    /// ダミーモデルの作成
    /// </summary>
    void CreateDummyModel();
    
    std::unique_ptr<Object3d> object3d_;
    std::unique_ptr<Model> dummyModel_;
    Ring* ring_;
    AdvancedRing* advancedRing_;
    DirectXCommon* dxCommon_;
    bool hasValidRing_;
};