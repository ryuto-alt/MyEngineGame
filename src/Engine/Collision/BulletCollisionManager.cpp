#include "BulletCollisionManager.h"
#include <algorithm>

namespace Collision {

    // 静的メンバ変数の初期化
    BulletCollisionManager* BulletCollisionManager::instance = nullptr;

    // シングルトンインスタンス取得
    BulletCollisionManager* BulletCollisionManager::GetInstance() {
        return instance;
    }

    // シングルトンインスタンス生成
    void BulletCollisionManager::Create() {
        if (!instance) {
            instance = new BulletCollisionManager();
        }
    }

    // シングルトンインスタンス破棄
    void BulletCollisionManager::Destroy() {
        if (instance) {
            delete instance;
            instance = nullptr;
        }
    }

    // コンストラクタ
    BulletCollisionManager::BulletCollisionManager() {
        // Bullet3の衝突システムを初期化
        bulletSystem = std::make_unique<BulletCollisionSystem>();
        
        // 衝突コールバックを設定
        bulletSystem->SetCollisionCallback([this](const CollisionInfo& info) {
            this->OnCollision(info);
        });
    }

    // デストラクタ
    BulletCollisionManager::~BulletCollisionManager() {
        // コリジョンオブジェクトとハンドラをクリア
        collisionObjectToUserData.clear();
        collisionHandlers.clear();
        lastFrameCollisions.clear();
    }

    // 更新処理
    void BulletCollisionManager::Update() {
        // 現在フレームの衝突情報をクリア
        std::vector<CollisionPair> currentFrameCollisions;
        
        // Bullet3の衝突検出を実行（この中でOnCollisionコールバックが呼ばれる）
        bulletSystem->Update();
        
        // 今回のフレームで衝突しなかったペアに対してExitイベントを発火
        for (const auto& lastPair : lastFrameCollisions) {
            // 現在のフレームで衝突していないか確認
            auto it = std::find(currentFrameCollisions.begin(), currentFrameCollisions.end(), lastPair);
            if (it == currentFrameCollisions.end()) {
                // Exitイベントを発火する処理を追加できます
                // ここでは単純化のため省略
            }
        }
        
        // 次のフレーム用に現在の衝突情報を保存
        lastFrameCollisions = std::move(currentFrameCollisions);
    }

    // 球コリジョン登録
    void* BulletCollisionManager::RegisterSphere(const Sphere& sphere, void* userData) {
        void* collisionObject = bulletSystem->AddSphere(sphere, userData);
        collisionObjectToUserData[collisionObject] = userData;
        return collisionObject;
    }

    // カプセルコリジョン登録
    void* BulletCollisionManager::RegisterCapsule(const Capsule& capsule, void* userData) {
        void* collisionObject = bulletSystem->AddCapsule(capsule, userData);
        collisionObjectToUserData[collisionObject] = userData;
        return collisionObject;
    }

    // ボックスコリジョン登録
    void* BulletCollisionManager::RegisterBox(const Vector3& halfExtents, const Vector3& position, void* userData) {
        void* collisionObject = bulletSystem->AddBox(halfExtents, position, userData);
        collisionObjectToUserData[collisionObject] = userData;
        return collisionObject;
    }

    // コリジョンオブジェクト削除
    void BulletCollisionManager::UnregisterCollisionObject(void* collisionObject) {
        if (collisionObject) {
            // ユーザーデータのマッピングを削除
            collisionObjectToUserData.erase(collisionObject);
            
            // Bulletのコリジョンオブジェクトを削除
            bulletSystem->RemoveCollisionObject(collisionObject);
            
            // このオブジェクトが関連するハンドラーを削除
            // （本来はもう少し効率的な方法を使うべきですが、例として単純化）
            auto it = collisionHandlers.begin();
            while (it != collisionHandlers.end()) {
                if (it->first.objectA == collisionObject || it->first.objectB == collisionObject) {
                    it = collisionHandlers.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    // 衝突イベントハンドラー登録
    void BulletCollisionManager::RegisterCollisionHandler(void* objectA, void* objectB, 
                                                        std::function<void(const CollisionInfo&)> handler) {
        CollisionPair pair;
        // objectAとobjectBの順序を一貫させるためにポインタ値で比較して小さい方をobjectAに
        if (objectA < objectB) {
            pair.objectA = objectA;
            pair.objectB = objectB;
        } else {
            pair.objectA = objectB;
            pair.objectB = objectA;
        }
        
        collisionHandlers[pair] = handler;
    }

    // 衝突イベントハンドラー削除
    void BulletCollisionManager::UnregisterCollisionHandler(void* objectA, void* objectB) {
        CollisionPair pair;
        // RegisterCollisionHandlerと同じロジックでペアを構築
        if (objectA < objectB) {
            pair.objectA = objectA;
            pair.objectB = objectB;
        } else {
            pair.objectA = objectB;
            pair.objectB = objectA;
        }
        
        collisionHandlers.erase(pair);
    }

    // レイキャスト実行
    CollisionResult BulletCollisionManager::RayCast(const Vector3& rayFrom, const Vector3& rayTo) const {
        return bulletSystem->RayCast(rayFrom, rayTo);
    }

    // Bullet衝突イベントのコールバック
    void BulletCollisionManager::OnCollision(const CollisionInfo& info) {
        // 衝突したオブジェクトのユーザーデータを取得
        void* userDataA = info.objectA;
        void* userDataB = info.objectB;
        
        // ハンドラーを呼び出すためのペアを作成
        CollisionPair pair;
        if (userDataA < userDataB) {
            pair.objectA = userDataA;
            pair.objectB = userDataB;
        } else {
            pair.objectA = userDataB;
            pair.objectB = userDataA;
        }
        
        // 該当するハンドラーがあれば呼び出す
        auto it = collisionHandlers.find(pair);
        if (it != collisionHandlers.end()) {
            it->second(info);
        }
        
        // 現在フレームの衝突リストに追加（Enter/Exitイベント用）
        // この行は実際のExitイベント実装時にコメントを外す
        // lastFrameCollisions.push_back(pair);
    }

} // namespace Collision