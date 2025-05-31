#pragma once
// Bullet3ヘッダーの条件付きインクルード
#ifdef USE_BULLET_PHYSICS
#include "../../bullet3/btBulletCollisionCommon.h"
#include "../../bullet3/btBulletDynamicsCommon.h"
#endif
#include "../Math/Vector3.h"
#include "CollisionPrimitive.h"
#include "CollisionUtility.h"
#include "Collision.h"
#include <memory>
#include <vector>
#include <functional>

namespace Collision {
    
    // 衝突情報構造体
    struct CollisionInfo {
        void* objectA;           // 衝突オブジェクトA
        void* objectB;           // 衝突オブジェクトB
        Vector3 collisionPoint;  // 衝突点
        Vector3 normal;          // 衝突法線（Aに対するBの法線）
        float penetration;       // めり込み量
    };

    // 衝突コールバック定義
    using CollisionCallback = std::function<void(const CollisionInfo&)>;

#ifdef USE_BULLET_PHYSICS
    // Bullet3を使用した衝突検出クラス
    class BulletCollisionSystem {
    private:
        // Bullet Physics用のコンポーネント
        std::unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration;
        std::unique_ptr<btCollisionDispatcher> dispatcher;
        std::unique_ptr<btBroadphaseInterface> overlappingPairCache;
        std::unique_ptr<btCollisionWorld> collisionWorld;

        // 形状とオブジェクトの管理
        std::vector<std::unique_ptr<btCollisionShape>> collisionShapes;
        std::vector<std::unique_ptr<btCollisionObject>> collisionObjects;

        // コールバック関数
        CollisionCallback collisionCallback;

    public:
        // コンストラクタ・デストラクタ
        BulletCollisionSystem();
        ~BulletCollisionSystem();

        // コリジョンワールドの更新
        void Update();

        // 球のコリジョン追加
        void* AddSphere(const Sphere& sphere, void* userData = nullptr);

        // カプセルのコリジョン追加
        void* AddCapsule(const Capsule& capsule, void* userData = nullptr);

        // ボックスのコリジョン追加
        void* AddBox(const Vector3& halfExtents, const Vector3& position, void* userData = nullptr);

        // コリジョンオブジェクト削除
        void RemoveCollisionObject(void* collisionObject);

        // 衝突検出時のコールバック設定
        void SetCollisionCallback(const CollisionCallback& callback);

        // ローワールドで直接レイキャスト
        CollisionResult RayCast(const Vector3& rayFrom, const Vector3& rayTo) const;

        // Bulletの座標系からゲームの座標系への変換
        static Vector3 BtToVector3(const btVector3& vec);

        // ゲームの座標系からBulletの座標系への変換
        static btVector3 Vector3ToBt(const Vector3& vec);
    };
#endif // USE_BULLET_PHYSICS

} // namespace Collision