#pragma once

// Bullet Physics のヘッダー
#include "bullet3/btBulletCollisionCommon.h"
#include "bullet3/btBulletDynamicsCommon.h"

// STLのインクルード
#include <vector>
#include <memory>
#include <functional>

// シンプルなベクトル構造体（既存のものがあれば置き換え）
struct SimpleVector3 {
    float x, y, z;
};

// Bullet3テストクラス
class BulletTest {
private:
    // Bullet Physicsのコンポーネント
    btDefaultCollisionConfiguration* collisionConfiguration;
    btCollisionDispatcher* dispatcher;
    btBroadphaseInterface* overlappingPairCache;
    btCollisionWorld* collisionWorld;
    
    // 形状とオブジェクトのコンテナ
    std::vector<btCollisionShape*> collisionShapes;
    std::vector<btCollisionObject*> collisionObjects;
    
    // 衝突時のコールバック関数
    std::function<void(void*, void*, const SimpleVector3&)> collisionCallback;

public:
    // コンストラクタ・デストラクタ
    BulletTest();
    ~BulletTest();
    
    // 更新処理
    void Update();
    
    // 球の追加
    void* AddSphere(const SimpleVector3& center, float radius, void* userData = nullptr);
    
    // ボックスの追加
    void* AddBox(const SimpleVector3& halfExtents, const SimpleVector3& position, void* userData = nullptr);
    
    // オブジェクトの移動
    void SetPosition(void* object, const SimpleVector3& position);
    
    // コールバック設定
    void SetCollisionCallback(std::function<void(void*, void*, const SimpleVector3&)> callback);
    
    // Bulletのベクトル変換
    static btVector3 ToBtVector(const SimpleVector3& v);
    static SimpleVector3 ToSimpleVector(const btVector3& v);
};
