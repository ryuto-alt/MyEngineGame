#pragma once
#include "Vector3.h"
#include "Mymath.h"
#include <vector>
#include <memory>
#include <functional>

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

// 前方宣言
class Model;
class Object3d;

// バウンディングボックス構造体
struct BoundingBox {
    Vector3 minPoint;
    Vector3 maxPoint;
    
    BoundingBox() : minPoint({0, 0, 0}), maxPoint({0, 0, 0}) {}
    BoundingBox(const Vector3& minPt, const Vector3& maxPt) : minPoint(minPt), maxPoint(maxPt) {}
};

// バウンディングスフィア構造体
struct BoundingSphere {
    Vector3 center;
    float radius;
    
    BoundingSphere() : center({0, 0, 0}), radius(0.0f) {}
    BoundingSphere(const Vector3& centerPoint, float sphereRadius) : center(centerPoint), radius(sphereRadius) {}
};

// 当たり判定結果構造体
struct CollisionInfo {
    bool isHit;
    Vector3 hitPoint;
    Vector3 normal;
    float penetration;
    Object3d* hitObject;
    
    CollisionInfo() : isHit(false), hitPoint({0, 0, 0}), normal({0, 0, 0}), penetration(0.0f), hitObject(nullptr) {}
};

// 衝突判定システムクラス
class CollisionSystem {
public:
    static CollisionSystem* GetInstance();
    static void Create();
    static void Destroy();
    
    // モデルからバウンディングボックスを生成
    BoundingBox CreateBoundingBox(const Model* model);
    
    // モデルからバウンディングスフィアを生成
    BoundingSphere CreateBoundingSphere(const Model* model);
    
    // オブジェクトの当たり判定を有効にする
    void EnableCollision(Object3d* object, bool useBox = true);
    
    // オブジェクトの当たり判定を無効にする
    void DisableCollision(Object3d* object);
    
    // 当たり判定の更新
    void Update();
    
    // ボックス同士の当たり判定
    bool CheckBoxToBox(const BoundingBox& box1, const Vector3& pos1, 
                      const BoundingBox& box2, const Vector3& pos2);
    
    // スフィア同士の当たり判定
    bool CheckSphereToSphere(const BoundingSphere& sphere1, const Vector3& pos1,
                           const BoundingSphere& sphere2, const Vector3& pos2);
    
    // ボックスとスフィアの当たり判定
    bool CheckBoxToSphere(const BoundingBox& box, const Vector3& boxPos,
                         const BoundingSphere& sphere, const Vector3& spherePos);
    
    // レイキャスト
    CollisionInfo RayCast(const Vector3& rayOrigin, const Vector3& rayDirection, float maxDistance = 1000.0f);
    
    // コールバック設定
    void SetCollisionCallback(std::function<void(Object3d*, Object3d*, const CollisionInfo&)> callback);

private:
    CollisionSystem() = default;
    ~CollisionSystem() = default;
    CollisionSystem(const CollisionSystem&) = delete;
    CollisionSystem& operator=(const CollisionSystem&) = delete;
    
    static CollisionSystem* instance_;
    
    // 当たり判定オブジェクト情報
    struct CollisionObject {
        Object3d* object;
        BoundingBox boundingBox;
        BoundingSphere boundingSphere;
        bool useBox; // true: ボックス, false: スフィア
        bool isEnabled;
        
        CollisionObject() : object(nullptr), useBox(true), isEnabled(false) {}
    };
    
    std::vector<std::unique_ptr<CollisionObject>> collisionObjects_;
    std::function<void(Object3d*, Object3d*, const CollisionInfo&)> collisionCallback_;
    
    // ヘルパー関数
    CollisionObject* FindCollisionObject(Object3d* object);
    void RemoveCollisionObject(Object3d* object);
};