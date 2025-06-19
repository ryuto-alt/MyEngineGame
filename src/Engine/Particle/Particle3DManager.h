#pragma once

#include <unordered_map>
#include <string>
#include <list>
#include <random>
#include <memory>
#include <queue>
#include <vector>
#include "DirectXCommon.h"
#include "SRVManager.h"
#include "Vector3.h"
#include "Mymath.h"
#include "Camera.h"
#include "Model.h"
#include "Object3d.h"

// 前方宣言
class SpriteCommon;

// 3Dパーティクル1粒の情報
struct Particle3D {
    // 座標
    Vector3 position = { 0.0f, 0.0f, 0.0f };
    // 速度
    Vector3 velocity = { 0.0f, 0.0f, 0.0f };
    // 加速度
    Vector3 accel = { 0.0f, 0.0f, 0.0f };
    // 回転
    Vector3 rotation = { 0.0f, 0.0f, 0.0f };
    // 回転速度
    Vector3 rotationVelocity = { 0.0f, 0.0f, 0.0f };
    // スケール
    Vector3 scale = { 1.0f, 1.0f, 1.0f };
    // 初期スケール
    Vector3 startScale = { 1.0f, 1.0f, 1.0f };
    // 最終スケール
    Vector3 endScale = { 1.0f, 1.0f, 1.0f };
    // 色
    Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    // 初期色
    Vector4 startColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    // 最終色
    Vector4 endColor = { 1.0f, 1.0f, 1.0f, 1.0f };
    // 経過時間
    float lifeTime = 0.0f;
    // 寿命
    float lifeTimeMax = 1.0f;
    // 生存フラグ
    bool isDead = false;
    // Object3D（プールから借用するため生ポインタ）
    Object3d* object3d = nullptr;
    // プールインデックス（返却時に使用）
    size_t poolIndex = 0;

    // デフォルトコンストラクタ
    Particle3D() = default;

    // コピーコンストラクタは削除
    Particle3D(const Particle3D&) = delete;
    Particle3D& operator=(const Particle3D&) = delete;

    // ムーブコンストラクタとムーブ代入演算子
    Particle3D(Particle3D&& other) noexcept
        : position(std::move(other.position))
        , velocity(std::move(other.velocity))
        , accel(std::move(other.accel))
        , rotation(std::move(other.rotation))
        , rotationVelocity(std::move(other.rotationVelocity))
        , scale(std::move(other.scale))
        , startScale(std::move(other.startScale))
        , endScale(std::move(other.endScale))
        , color(std::move(other.color))
        , startColor(std::move(other.startColor))
        , endColor(std::move(other.endColor))
        , lifeTime(other.lifeTime)
        , lifeTimeMax(other.lifeTimeMax)
        , isDead(other.isDead)
        , object3d(other.object3d)
        , poolIndex(other.poolIndex) {
        other.object3d = nullptr;
    }

    Particle3D& operator=(Particle3D&& other) noexcept {
        if (this != &other) {
            position = other.position;
            velocity = other.velocity;
            accel = other.accel;
            rotation = other.rotation;
            rotationVelocity = other.rotationVelocity;
            scale = other.scale;
            startScale = other.startScale;
            endScale = other.endScale;
            color = other.color;
            startColor = other.startColor;
            endColor = other.endColor;
            lifeTime = other.lifeTime;
            lifeTimeMax = other.lifeTimeMax;
            isDead = other.isDead;
            object3d = other.object3d;
            poolIndex = other.poolIndex;
            other.object3d = nullptr;
        }
        return *this;
    }
};

// 3Dパーティクルグループ
struct Particle3DGroup {
    // モデル
    std::shared_ptr<Model> model;
    // パーティクルのリスト
    std::list<Particle3D> particles;
    // Object3Dのプール（再利用用）
    std::vector<std::unique_ptr<Object3d>> objectPool;
    // 利用可能なObject3Dのインデックス
    std::queue<size_t> availableIndices;
};

// 3Dパーティクルマネージャクラス
class Particle3DManager {
private:
    // DirectXCommon
    DirectXCommon* dxCommon_ = nullptr;

    // SRVマネージャ
    SrvManager* srvManager_ = nullptr;

    // SpriteCommon
    SpriteCommon* spriteCommon_ = nullptr;

    // 乱数生成器
    std::mt19937 randomEngine_;

    // 3Dパーティクルグループコンテナ
    std::unordered_map<std::string, Particle3DGroup> particle3DGroups;

    // コピー禁止
    Particle3DManager(const Particle3DManager&) = delete;
    Particle3DManager& operator=(const Particle3DManager&) = delete;

    // コンストラクタ（シングルトン）
    Particle3DManager() = default;
    // デストラクタ
    ~Particle3DManager() = default;

public:
    // シングルトンインスタンスの取得
    static Particle3DManager* GetInstance() {
        static Particle3DManager instance;
        return &instance;
    }

    // 終了処理
    static void Finalize() {
        // Meyer'sシングルトンのリソースを強制的にクリア
        Particle3DManager* instance = GetInstance();
        instance->ForceReleaseResources();
    }

    // リソースの強制解放
    void ForceReleaseResources() {
        // 全3Dパーティクルグループのクリア
        particle3DGroups.clear();
    }

    // 初期化
    void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager, SpriteCommon* spriteCommon);

    // 更新
    void Update(const Camera* camera);

    // 描画
    void Draw(const Camera* camera);

    // 3Dパーティクルグループの作成
    void CreateParticle3DGroup(const std::string& name, const std::string& modelFilePath);

    // 3Dパーティクルの発生
    void Emit3D(
        const std::string& name,
        const Vector3& position,
        uint32_t count,
        const Vector3& velocityMin = { -1.0f, -1.0f, -1.0f },
        const Vector3& velocityMax = { 1.0f, 1.0f, 1.0f },
        const Vector3& accelMin = { 0.0f, 0.0f, 0.0f },
        const Vector3& accelMax = { 0.0f, -9.8f, 0.0f },
        const Vector3& startScaleMin = { 0.5f, 0.5f, 0.5f },
        const Vector3& startScaleMax = { 1.0f, 1.0f, 1.0f },
        const Vector3& endScaleMin = { 0.0f, 0.0f, 0.0f },
        const Vector3& endScaleMax = { 0.0f, 0.0f, 0.0f },
        const Vector4& startColorMin = { 1.0f, 1.0f, 1.0f, 1.0f },
        const Vector4& startColorMax = { 1.0f, 1.0f, 1.0f, 1.0f },
        const Vector4& endColorMin = { 1.0f, 1.0f, 1.0f, 0.0f },
        const Vector4& endColorMax = { 1.0f, 1.0f, 1.0f, 0.0f },
        const Vector3& rotationMin = { 0.0f, 0.0f, 0.0f },
        const Vector3& rotationMax = { 0.0f, 0.0f, 0.0f },
        const Vector3& rotationVelocityMin = { 0.0f, 0.0f, 0.0f },
        const Vector3& rotationVelocityMax = { 0.0f, 0.0f, 0.0f },
        float lifeTimeMin = 1.0f,
        float lifeTimeMax = 3.0f);

    // デバッグ用：パーティクル数の取得
    uint32_t GetParticle3DCount(const std::string& name) {
        auto it = particle3DGroups.find(name);
        if (it != particle3DGroups.end()) {
            return static_cast<uint32_t>(it->second.particles.size());
        }
        return 0;
    }
};
