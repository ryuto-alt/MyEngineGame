#include "BulletCollision.h"
#include <algorithm>

#ifdef USE_BULLET_PHYSICS
namespace Collision {

    // コンストラクタ
    BulletCollisionSystem::BulletCollisionSystem() {
        // Bulletの物理ワールドを初期化
        collisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();
        dispatcher = std::make_unique<btCollisionDispatcher>(collisionConfiguration.get());
        overlappingPairCache = std::make_unique<btDbvtBroadphase>();
        collisionWorld = std::make_unique<btCollisionWorld>(
            dispatcher.get(),
            overlappingPairCache.get(),
            collisionConfiguration.get()
        );
    }

    // デストラクタ
    BulletCollisionSystem::~BulletCollisionSystem() {
        // コリジョンオブジェクトをクリア
        collisionObjects.clear();
        
        // 形状をクリア
        collisionShapes.clear();
    }

    // コリジョンワールドの更新と衝突検出
    void BulletCollisionSystem::Update() {
        // コリジョン検出を実行
        collisionWorld->performDiscreteCollisionDetection();

        // 衝突マニフォールドを取得
        int numManifolds = collisionWorld->getDispatcher()->getNumManifolds();
        
        // すべてのマニフォールドをループ
        for (int i = 0; i < numManifolds; i++) {
            btPersistentManifold* contactManifold = collisionWorld->getDispatcher()->getManifoldByIndexInternal(i);
            const btCollisionObject* objA = contactManifold->getBody0();
            const btCollisionObject* objB = contactManifold->getBody1();

            // 接触点の数
            int numContacts = contactManifold->getNumContacts();
            if (numContacts > 0 && collisionCallback) {
                // 最も深く貫通している点を見つける
                int deepestIndex = 0;
                float deepestDistance = contactManifold->getContactPoint(0).getDistance();

                for (int j = 1; j < numContacts; j++) {
                    float distance = contactManifold->getContactPoint(j).getDistance();
                    if (distance < deepestDistance) {
                        deepestIndex = j;
                        deepestDistance = distance;
                    }
                }

                // 最も深い接触点の情報を取得
                btManifoldPoint& pt = contactManifold->getContactPoint(deepestIndex);
                
                // 衝突情報を構築
                CollisionInfo info;
                info.objectA = objA->getUserPointer();
                info.objectB = objB->getUserPointer();
                info.collisionPoint = BtToVector3(pt.getPositionWorldOnB());
                info.normal = BtToVector3(pt.m_normalWorldOnB);
                info.penetration = -pt.getDistance(); // めり込み量を正の値に変換

                // コールバック呼び出し
                collisionCallback(info);
            }
        }
    }

    // 球のコリジョン追加
    void* BulletCollisionSystem::AddSphere(const Sphere& sphere, void* userData) {
        // 球の形状を作成
        auto shape = std::make_unique<btSphereShape>(sphere.radius);
        collisionShapes.push_back(std::move(shape));

        // コリジョンオブジェクトを作成
        auto obj = std::make_unique<btCollisionObject>();
        obj->setCollisionShape(collisionShapes.back().get());
        
        // 位置を設定
        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(Vector3ToBt(sphere.center));
        obj->setWorldTransform(transform);

        // ユーザーデータの設定
        obj->setUserPointer(userData);

        // コリジョンワールドに追加
        collisionWorld->addCollisionObject(obj.get());
        
        // オブジェクトへのポインタを保存して返す
        void* objPtr = obj.get();
        collisionObjects.push_back(std::move(obj));
        return objPtr;
    }

    // カプセルのコリジョン追加
    void* BulletCollisionSystem::AddCapsule(const Capsule& capsule, void* userData) {
        // カプセルの向きを計算
        Vector3 direction = Utility::Subtract(capsule.segment.end, capsule.segment.start);
        float height = Utility::Length(direction);
        
        // 形状を作成（高さは両端の球を除いた部分）
        auto shape = std::make_unique<btCapsuleShape>(capsule.radius, height);
        collisionShapes.push_back(std::move(shape));

        // コリジョンオブジェクトを作成
        auto obj = std::make_unique<btCollisionObject>();
        obj->setCollisionShape(collisionShapes.back().get());
        
        // 位置と回転を設定
        btTransform transform;
        transform.setIdentity();

        // カプセルの中心位置
        Vector3 center;
        center.x = (capsule.segment.start.x + capsule.segment.end.x) * 0.5f;
        center.y = (capsule.segment.start.y + capsule.segment.end.y) * 0.5f;
        center.z = (capsule.segment.start.z + capsule.segment.end.z) * 0.5f;
        transform.setOrigin(Vector3ToBt(center));

        // カプセルの向きを設定（デフォルトはY軸方向）
        if (height > 0.001f) {
            Vector3 normalizedDir = Utility::Multiply(direction, (1.0f / height));
            Vector3 yAxis = {0.0f, 1.0f, 0.0f};
            
            Vector3 rotationAxis = Utility::Cross(yAxis, normalizedDir);
            float rotationAngle = std::acos(Utility::Dot(yAxis, normalizedDir));
            
            if (Utility::Length(rotationAxis) > 0.001f) {
                rotationAxis = Utility::Normalize(rotationAxis);
                btQuaternion rotation(btVector3(rotationAxis.x, rotationAxis.y, rotationAxis.z), rotationAngle);
                transform.setRotation(rotation);
            }
        }
        
        obj->setWorldTransform(transform);

        // ユーザーデータの設定
        obj->setUserPointer(userData);

        // コリジョンワールドに追加
        collisionWorld->addCollisionObject(obj.get());
        
        // オブジェクトへのポインタを保存して返す
        void* objPtr = obj.get();
        collisionObjects.push_back(std::move(obj));
        return objPtr;
    }

    // ボックスのコリジョン追加
    void* BulletCollisionSystem::AddBox(const Vector3& halfExtents, const Vector3& position, void* userData) {
        // ボックス形状を作成
        auto shape = std::make_unique<btBoxShape>(Vector3ToBt(halfExtents));
        collisionShapes.push_back(std::move(shape));

        // コリジョンオブジェクトを作成
        auto obj = std::make_unique<btCollisionObject>();
        obj->setCollisionShape(collisionShapes.back().get());
        
        // 位置を設定
        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(Vector3ToBt(position));
        obj->setWorldTransform(transform);

        // ユーザーデータの設定
        obj->setUserPointer(userData);

        // コリジョンワールドに追加
        collisionWorld->addCollisionObject(obj.get());
        
        // オブジェクトへのポインタを保存して返す
        void* objPtr = obj.get();
        collisionObjects.push_back(std::move(obj));
        return objPtr;
    }

    // コリジョンオブジェクト削除
    void BulletCollisionSystem::RemoveCollisionObject(void* collisionObject) {
        // オブジェクトを見つける
        auto it = std::find_if(collisionObjects.begin(), collisionObjects.end(), 
            [collisionObject](const std::unique_ptr<btCollisionObject>& obj) {
                return obj.get() == collisionObject;
            });
        
        if (it != collisionObjects.end()) {
            // コリジョンワールドから削除
            collisionWorld->removeCollisionObject(it->get());
            
            // ベクターから削除
            collisionObjects.erase(it);
        }
    }

    // 衝突検出時のコールバック設定
    void BulletCollisionSystem::SetCollisionCallback(const CollisionCallback& callback) {
        collisionCallback = callback;
    }

    // ローワールドで直接レイキャスト
    CollisionResult BulletCollisionSystem::RayCast(const Vector3& rayFrom, const Vector3& rayTo) const {
        // Bullet用のレイキャストを準備
        btVector3 btFrom = Vector3ToBt(rayFrom);
        btVector3 btTo = Vector3ToBt(rayTo);
        
        // レイキャストを実行
        btCollisionWorld::ClosestRayResultCallback rayCallback(btFrom, btTo);
        collisionWorld->rayTest(btFrom, btTo, rayCallback);
        
        // 結果を変換
        CollisionResult result;
        
        if (rayCallback.hasHit()) {
            result.isColliding = true;
            result.collisionPoint = BtToVector3(rayCallback.m_hitPointWorld);
            result.normal = BtToVector3(rayCallback.m_hitNormalWorld);
            
            // めり込み量を計算
            // レイの方向と長さを取得
            Vector3 rayDir = Utility::Normalize(Utility::Subtract(rayTo, rayFrom));
            float rayLength = Utility::Length(Utility::Subtract(rayTo, rayFrom));
            
            // 衝突点までの距離を計算
            Vector3 toHitPoint = Utility::Subtract(result.collisionPoint, rayFrom);
            float distanceToHit = Utility::Dot(toHitPoint, rayDir);
            
            // めり込み量は、レイの長さから衝突点までの距離を引いたもの
            result.penetration = rayLength - distanceToHit;
        }
        
        return result;
    }

    // Bulletの座標系からゲームの座標系への変換
    Vector3 BulletCollisionSystem::BtToVector3(const btVector3& vec) {
        return Vector3{vec.x(), vec.y(), vec.z()};
    }

    // ゲームの座標系からBulletの座標系への変換
    btVector3 BulletCollisionSystem::Vector3ToBt(const Vector3& vec) {
        return btVector3(vec.x, vec.y, vec.z);
    }

} // namespace Collision
#endif // USE_BULLET_PHYSICS