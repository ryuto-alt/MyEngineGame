#include "BulletCollisionTest.h"
#include "../../Engine/Graphics/GraphicsManager.h"
#include "../../Engine/Input/InputManager.h"
#include "../../Engine/Core/SceneManager.h"
#include "../../Engine/Core/Time.h"

// コンストラクタ
BulletCollisionTest::BulletCollisionTest() : camera(nullptr), collisionManager(nullptr) {
}

// デストラクタ
BulletCollisionTest::~BulletCollisionTest() {
}

// 初期化
void BulletCollisionTest::Initialize() {
    // カメラの初期化
    camera = std::make_shared<Camera>();
    camera->SetPosition({0.0f, 5.0f, -15.0f});
    camera->SetTarget({0.0f, 0.0f, 0.0f});
    
    // Bullet衝突マネージャを初期化
    Collision::BulletCollisionManager::Create();
    collisionManager = Collision::BulletCollisionManager::GetInstance();
    
    // いくつかのテストオブジェクトを作成
    CreateSphereObject({-5.0f, 5.0f, 0.0f}, 1.0f, {5.0f, 0.0f, 0.0f});
    CreateSphereObject({5.0f, 5.0f, 0.0f}, 1.0f, {-5.0f, 0.0f, 0.0f});
    CreateSphereObject({0.0f, 5.0f, -5.0f}, 1.0f, {0.0f, 0.0f, 5.0f});
    
    // 静的なオブジェクト（床と壁）を作成
    CreateStaticBox({0.0f, -1.0f, 0.0f}, {10.0f, 1.0f, 10.0f});   // 床
    CreateStaticBox({-10.0f, 5.0f, 0.0f}, {1.0f, 5.0f, 10.0f});    // 左壁
    CreateStaticBox({10.0f, 5.0f, 0.0f}, {1.0f, 5.0f, 10.0f});     // 右壁
    CreateStaticBox({0.0f, 5.0f, 10.0f}, {10.0f, 5.0f, 1.0f});     // 奥壁
    CreateStaticBox({0.0f, 5.0f, -10.0f}, {10.0f, 5.0f, 1.0f});    // 手前壁
    
    // 静的な球を追加
    CreateStaticSphere({0.0f, 0.0f, 0.0f}, 2.0f);
}

// 更新処理
void BulletCollisionTest::Update() {
    // カメラの更新
    camera->Update();
    
    // 物理シミュレーションの更新
    UpdatePhysics(Time::DeltaTime());
    
    // 衝突検出の更新
    collisionManager->Update();
    
    // 入力処理（例: ESCキーでシーン終了）
    if (Input::InputManager::GetKeyDown(Input::Keys::Escape)) {
        SceneManager::GetInstance()->ChangeScene("Title");
    }
}

// 描画処理
void BulletCollisionTest::Draw() {
    // カメラの設定
    Graphics::GraphicsManager::SetCamera(camera);
    
    // 動的な球オブジェクトの描画
    for (const auto& obj : sphereObjects) {
        // 衝突中なら色を変える
        if (obj.isColliding) {
            obj.model->SetColor({1.0f, 0.0f, 0.0f, 1.0f}); // 赤色
        } else {
            obj.model->SetColor({0.0f, 1.0f, 0.0f, 1.0f}); // 緑色
        }
        
        obj.model->Draw();
    }
    
    // 静的なオブジェクトの描画
    for (const auto& obj : staticObjects) {
        obj.model->Draw();
    }
}

// 終了処理
void BulletCollisionTest::Finalize() {
    // すべてのオブジェクトのコリジョンを削除
    for (auto& obj : sphereObjects) {
        if (obj.collisionObject) {
            collisionManager->UnregisterCollisionObject(obj.collisionObject);
            obj.collisionObject = nullptr;
        }
    }
    
    for (auto& obj : staticObjects) {
        if (obj.collisionObject) {
            collisionManager->UnregisterCollisionObject(obj.collisionObject);
            obj.collisionObject = nullptr;
        }
    }
    
    // 球オブジェクトとモデルをクリア
    sphereObjects.clear();
    staticObjects.clear();
    
    // Bullet衝突マネージャを破棄
    Collision::BulletCollisionManager::Destroy();
    collisionManager = nullptr;
}

// 衝突判定コールバック
void BulletCollisionTest::OnCollision(const Collision::CollisionInfo& info) {
    // 衝突したオブジェクトを探して衝突フラグを設定
    for (auto& obj : sphereObjects) {
        if (obj.collisionObject == info.objectA || obj.collisionObject == info.objectB) {
            obj.isColliding = true;
            
            // 衝突応答（簡易的な跳ね返り）
            Vector3 normal = info.normal;
            if (obj.collisionObject == info.objectB) {
                normal.x = -normal.x;
                normal.y = -normal.y;
                normal.z = -normal.z;
            }
            
            // 反射ベクトルを計算（簡易版）
            float dotProduct = Collision::Utility::Dot(obj.velocity, normal);
            if (dotProduct < 0) {
                // 反射方向に速度を変更
                Vector3 reflectionVelocity;
                Vector3 doubleNormalDotProduct = {
                    normal.x * 2.0f * dotProduct,
                    normal.y * 2.0f * dotProduct,
                    normal.z * 2.0f * dotProduct
                };
                reflectionVelocity.x = obj.velocity.x - doubleNormalDotProduct.x;
                reflectionVelocity.y = obj.velocity.y - doubleNormalDotProduct.y;
                reflectionVelocity.z = obj.velocity.z - doubleNormalDotProduct.z;
                
                // 若干の速度減衰
                obj.velocity.x = reflectionVelocity.x * 0.8f;
                obj.velocity.y = reflectionVelocity.y * 0.8f;
                obj.velocity.z = reflectionVelocity.z * 0.8f;
            }
        }
    }
}

// 物理シミュレーション更新
void BulletCollisionTest::UpdatePhysics(float deltaTime) {
    // 重力の設定
    const Vector3 gravity = {0.0f, -9.8f, 0.0f};
    
    // 球オブジェクトの更新
    for (auto& obj : sphereObjects) {
        // 前回の衝突フラグをリセット
        obj.isColliding = false;
        
        // 重力を適用
        obj.velocity.x += gravity.x * deltaTime;
        obj.velocity.y += gravity.y * deltaTime;
        obj.velocity.z += gravity.z * deltaTime;
        
        // 位置を更新
        obj.sphere.center.x += obj.velocity.x * deltaTime;
        obj.sphere.center.y += obj.velocity.y * deltaTime;
        obj.sphere.center.z += obj.velocity.z * deltaTime;
        
        // モデルの位置を更新
        obj.model->SetPosition(obj.sphere.center);
        
        // Bulletのコリジョンオブジェクトの位置を更新
        // 注: 実際にはBulletCollisionManagerに位置更新メソッドを追加する必要があります
        collisionManager->UnregisterCollisionObject(obj.collisionObject);
        obj.collisionObject = collisionManager->RegisterSphere(obj.sphere, &obj);
    }
}

// 動的な球オブジェクト生成
void BulletCollisionTest::CreateSphereObject(const Vector3& position, float radius, const Vector3& velocity) {
    PhysicsObject obj;
    
    // 球の設定
    obj.sphere.center = position;
    obj.sphere.radius = radius;
    obj.velocity = velocity;
    obj.isColliding = false;
    
    // モデルの作成
    obj.model = std::make_shared<Graphics::Model>();
    obj.model->CreateSphere(radius, 16, 16);
    obj.model->SetPosition(position);
    obj.model->SetColor({0.0f, 1.0f, 0.0f, 1.0f}); // 緑色
    
    // Bulletのコリジョンオブジェクト登録
    obj.collisionObject = collisionManager->RegisterSphere(obj.sphere, &obj);
    
    // コリジョンハンドラー登録
    collisionManager->RegisterCollisionHandler(&obj, nullptr, 
        [this](const Collision::CollisionInfo& info) {
            this->OnCollision(info);
        });
    
    // リストに追加
    sphereObjects.push_back(obj);
}

// 静的な球オブジェクト生成
void BulletCollisionTest::CreateStaticSphere(const Vector3& position, float radius) {
    PhysicsObject obj;
    
    // 球の設定
    obj.sphere.center = position;
    obj.sphere.radius = radius;
    obj.velocity = {0.0f, 0.0f, 0.0f};
    obj.isColliding = false;
    
    // モデルの作成
    obj.model = std::make_shared<Graphics::Model>();
    obj.model->CreateSphere(radius, 16, 16);
    obj.model->SetPosition(position);
    obj.model->SetColor({0.5f, 0.5f, 1.0f, 1.0f}); // 青っぽい色
    
    // Bulletのコリジョンオブジェクト登録
    obj.collisionObject = collisionManager->RegisterSphere(obj.sphere, &obj);
    
    // リストに追加
    staticObjects.push_back(obj);
}

// 静的なボックスオブジェクト生成
void BulletCollisionTest::CreateStaticBox(const Vector3& position, const Vector3& halfExtents) {
    PhysicsObject obj;
    
    // 簡易的に球としてコライダーを設定（本来はボックスコライダーが必要）
    float maxExtent = std::max(std::max(halfExtents.x, halfExtents.y), halfExtents.z);
    obj.sphere.center = position;
    obj.sphere.radius = maxExtent;
    obj.velocity = {0.0f, 0.0f, 0.0f};
    obj.isColliding = false;
    
    // モデルの作成
    obj.model = std::make_shared<Graphics::Model>();
    obj.model->CreateBox(halfExtents.x * 2.0f, halfExtents.y * 2.0f, halfExtents.z * 2.0f);
    obj.model->SetPosition(position);
    obj.model->SetColor({0.8f, 0.8f, 0.8f, 1.0f}); // 灰色
    
    // Bulletのボックスコリジョンオブジェクト登録
    obj.collisionObject = collisionManager->RegisterBox(halfExtents, position, &obj);
    
    // リストに追加
    staticObjects.push_back(obj);
}
