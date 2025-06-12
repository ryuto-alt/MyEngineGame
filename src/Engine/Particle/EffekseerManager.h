#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <d3d12.h>
#include <wrl/client.h>
#include "DirectXCommon.h"
#include "Camera.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "ParticleManager.h"

// Effekseer includes - 実際のライブラリを使用
// Note: これらのヘッダーは外部ライブラリのため、コメントアウトしています
// 実際に使用するときは以下をアンコメントしてください：
/*
#include <Effekseer.h>
#include <EffekseerRendererDX12.h>
*/

// プレースホルダー定義 - 実際のライブラリ使用時は削除
namespace Effekseer {
    class Manager;
    using ManagerRef = std::shared_ptr<Manager>;
    class Effect;
    using EffectRef = std::shared_ptr<Effect>;
    using Handle = int32_t;
    struct Vector3D { float X, Y, Z; };
    struct Matrix44 { float Values[4][4]; };
}

namespace EffekseerRenderer {
    class Renderer;
    using RendererRef = std::shared_ptr<Renderer>;
}

// Effekseer統合マネージャクラス
class EffekseerManager {
private:
    // DirectXCommon
    DirectXCommon* dxCommon_ = nullptr;

    // Effekseerマネージャー
    ::Effekseer::ManagerRef efkManager_;

    // Effekseerレンダラー
    ::EffekseerRenderer::RendererRef efkRenderer_;

    // エフェクトコンテナ
    std::unordered_map<std::string, ::Effekseer::EffectRef> effects_;

    // アクティブなエフェクトハンドル
    std::unordered_map<std::string, ::Effekseer::Handle> activeEffects_;

    // 初期化フラグ
    bool isInitialized_ = false;

    // フォールバック用：ParticleManagerを使用した簡易エフェクト
    bool useParticleFallback_ = true;
    int nextHandle_ = 1;  // エフェクトハンドル用のカウンタ

    // コピー禁止
    EffekseerManager(const EffekseerManager&) = delete;
    EffekseerManager& operator=(const EffekseerManager&) = delete;

    // コンストラクタ（シングルトン）
    EffekseerManager() = default;
    // デストラクタ
    ~EffekseerManager();

public:
    // シングルトンインスタンスの取得
    static EffekseerManager* GetInstance() {
        static EffekseerManager instance;
        return &instance;
    }

    // 終了処理
    static void Finalize() {
        EffekseerManager* instance = GetInstance();
        instance->ForceReleaseResources();
    }

    // リソースの強制解放
    void ForceReleaseResources();

    // 初期化
    bool Initialize(DirectXCommon* dxCommon);

    // 更新
    void Update(const Camera* camera);

    // 描画
    void Draw(const Camera* camera);

    // エフェクトの読み込み
    bool LoadEffect(const std::string& name, const std::string& filePath);

    // エフェクトの再生
    ::Effekseer::Handle PlayEffect(const std::string& name, const Vector3& position);

    // エフェクトの停止
    void StopEffect(::Effekseer::Handle handle);

    // エフェクトの一括停止
    void StopAllEffects();

    // エフェクトの位置設定
    void SetEffectPosition(::Effekseer::Handle handle, const Vector3& position);

    // エフェクトの回転設定
    void SetEffectRotation(::Effekseer::Handle handle, const Vector3& rotation);

    // エフェクトのスケール設定
    void SetEffectScale(::Effekseer::Handle handle, const Vector3& scale);

    // エフェクトが再生中かチェック
    bool IsPlaying(::Effekseer::Handle handle);

    // デバッグ用：アクティブなエフェクト数の取得
    int GetActiveEffectCount() const;

    // 初期化状態の確認
    bool IsInitialized() const { return isInitialized_; }
};