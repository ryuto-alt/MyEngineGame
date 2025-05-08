#pragma once
#include "../../Engine/Collision/BulletCollisionManager.h"
#include "../../Engine/Graphics/Model.h"
#include "../../Engine/Camera/Camera.h"
#include "../../Engine/Core/Scene.h"
#include <vector>
#include <memory>

// Bullet3による当たり判定テストシーン
class BulletCollisionTest : public Scene {
private:
    // テスト用物理オブジェクト構造体
    struct PhysicsObject {
        std::shared_ptr<Graphics::Model> model;  // 描画用モデル
        Collision::Sphere sphere;                // 衝突判定用球
        Vector3 velocity;                        // 速度
        void* collisionObject;                   // Bulletのコリジョンオブジェクト
        bool isColliding;                        // 衝突中フラグ
    };

    // テスト用のオブジェクト
    std::vector<PhysicsObject> sphereObjects;
    std::vector<PhysicsObject> staticObjects;

    // カメラ
    std::shared_ptr<Camera> camera;

    // Bullet衝突マネージャ
    Collision::BulletCollisionManager* collisionManager;

public:
    BulletCollisionTest();
    ~BulletCollisionTest();

    // シーン管理
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

private:
    // 衝突判定コールバック
    void OnCollision(const Collision::CollisionInfo& info);

    // 物理シミュレーション更新
    void UpdatePhysics(float deltaTime);

    // オブジェクト生成
    void CreateSphereObject(const Vector3& position, float radius, const Vector3& velocity);
    void CreateStaticSphere(const Vector3& position, float radius);
    void CreateStaticBox(const Vector3& position, const Vector3& halfExtents);
};
