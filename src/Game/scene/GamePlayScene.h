#pragma once
#include "UnoEngine.h"
#include "btBulletDynamicsCommon.h"
#include "../../../src/Engine/Physics/GameObject.h"

// Bullet物理シミュレーションを使用したゲームプレイシーン
class GamePlayScene : public IScene {
public:
    // コンストラクタ・デストラクタ
    GamePlayScene();
    ~GamePlayScene() override;

    // ISceneの実装
    void Initialize() override;
    void Update() override;
    void Draw() override;
    void Finalize() override;

protected:
    // 初期化済みフラグ
    bool initialized_ = false;

    // Bullet物理シミュレーション関連
    btDefaultCollisionConfiguration* collisionConfiguration_ = nullptr;
    btCollisionDispatcher* dispatcher_ = nullptr;
    btBroadphaseInterface* broadphase_ = nullptr;
    btSequentialImpulseConstraintSolver* solver_ = nullptr;
    btDiscreteDynamicsWorld* dynamicsWorld_ = nullptr;

    // ゲームオブジェクトリスト
    std::vector<GameObject*> gameObjects_;

    // 床のオブジェクト
    GameObject* groundObject_ = nullptr;
    btCollisionShape* groundShape_ = nullptr;
    btRigidBody* groundBody_ = nullptr;

    // モデルリスト（メモリ管理用）
    std::vector<Model*> models_;

    // 物理オブジェクトを追加する関数
    GameObject* AddBox(const Vector3& position, const Vector3& size, float mass);
    GameObject* AddSphere(const Vector3& position, float radius, float mass);

    // カメラ操作関連
    Vector3 cameraPosition_ = {0.0f, 5.0f, -15.0f};
    Vector3 cameraRotation_ = {0.2f, 0.0f, 0.0f};
    float cameraSpeed_ = 0.2f;
};
