#pragma once
#include "BulletCollision.h"
#include "CollisionManager.h"
#include <memory>
#include <map>
#include <vector>

#ifdef USE_BULLET_PHYSICS
namespace Collision {

    // BulletPhysicsを使用した衝突管理クラス
    class BulletCollisionManager {
    private:
        // Bullet3の衝突システム
        std::unique_ptr<BulletCollisionSystem> bulletSystem;
        
        // 衝突イベントハンドラーを登録するためのマップ
        struct CollisionPair {
            void* objectA;
            void* objectB;
            
            // 比較演算子のオーバーロード
            bool operator<(const CollisionPair& other) const {
                if (objectA != other.objectA) return objectA < other.objectA;
                return objectB < other.objectB;
            }
            
            // 等価演算子のオーバーロード
            bool operator==(const CollisionPair& other) const {
                return (objectA == other.objectA && objectB == other.objectB) ||
                       (objectA == other.objectB && objectB == other.objectA);
            }
        };
        
        std::map<CollisionPair, std::function<void(const CollisionInfo&)>> collisionHandlers;
        
        // 登録されたコリジョンオブジェクトとそのユーザーデータのマッピング
        std::map<void*, void*> collisionObjectToUserData;
        
        // 前回のフレームで衝突したペアを記録（Enter/Exitイベント用）
        std::vector<CollisionPair> lastFrameCollisions;
        
        // シングルトンインスタンス
        static BulletCollisionManager* instance;
        
    public:
        // コンストラクタ・デストラクタ
        BulletCollisionManager();
        ~BulletCollisionManager();
        
        // シングルトンアクセサ
        static BulletCollisionManager* GetInstance();
        static void Create();
        static void Destroy();
        
        // 更新処理
        void Update();
        
        // 球コリジョン登録
        void* RegisterSphere(const Sphere& sphere, void* userData = nullptr);
        
        // カプセルコリジョン登録
        void* RegisterCapsule(const Capsule& capsule, void* userData = nullptr);
        
        // ボックスコリジョン登録
        void* RegisterBox(const Vector3& halfExtents, const Vector3& position, void* userData = nullptr);
        
        // コリジョンオブジェクト削除
        void UnregisterCollisionObject(void* collisionObject);
        
        // 衝突イベントハンドラー登録
        void RegisterCollisionHandler(void* objectA, void* objectB, 
            std::function<void(const CollisionInfo&)> handler);
        
        // 衝突イベントハンドラー削除
        void UnregisterCollisionHandler(void* objectA, void* objectB);
        
        // レイキャスト実行
        CollisionResult RayCast(const Vector3& rayFrom, const Vector3& rayTo) const;
        
    private:
        // Bullet衝突イベントのコールバック
        void OnCollision(const CollisionInfo& info);
    };

} // namespace Collision
#endif // USE_BULLET_PHYSICS