#include "BulletTest.h"
#include <iostream>
#include <vector>
#include <string>

// 物理オブジェクト管理用構造体
struct TestObject {
    SimpleVector3 position;
    SimpleVector3 velocity;
    float radius;
    bool isColliding;
    void* bulletObject;
    
    TestObject() : radius(1.0f), isColliding(false), bulletObject(nullptr) {
        position = {0.0f, 0.0f, 0.0f};
        velocity = {0.0f, 0.0f, 0.0f};
    }
};

// テスト関数
void RunBulletTest() {
    std::cout << "Starting Bullet Physics Test..." << std::endl;
    
    // Bullet初期化
    BulletTest bulletTest;
    
    // オブジェクト管理
    std::vector<TestObject> dynamicObjects;
    std::vector<TestObject> staticObjects;
    
    // 衝突コールバック設定
    bulletTest.SetCollisionCallback([&](void* objA, void* objB, const SimpleVector3& point) {
        // 衝突したオブジェクトを探して衝突フラグを設定
        for (auto& obj : dynamicObjects) {
            if (&obj == objA || &obj == objB) {
                obj.isColliding = true;
                std::cout << "Collision detected at: " << point.x << ", " << point.y << ", " << point.z << std::endl;
                
                // 簡易的な反射の処理
                // ここで物理演算を行うことができます
            }
        }
    });
    
    // 動的な球を追加
    TestObject sphere1;
    sphere1.position = {-5.0f, 5.0f, 0.0f};
    sphere1.velocity = {0.5f, 0.0f, 0.0f};
    sphere1.radius = 1.0f;
    sphere1.bulletObject = bulletTest.AddSphere(sphere1.position, sphere1.radius, &sphere1);
    dynamicObjects.push_back(sphere1);
    
    TestObject sphere2;
    sphere2.position = {5.0f, 5.0f, 0.0f};
    sphere2.velocity = {-0.5f, 0.0f, 0.0f};
    sphere2.radius = 1.0f;
    sphere2.bulletObject = bulletTest.AddSphere(sphere2.position, sphere2.radius, &sphere2);
    dynamicObjects.push_back(sphere2);
    
    // 静的な床を追加
    TestObject floor;
    floor.position = {0.0f, -1.0f, 0.0f};
    floor.bulletObject = bulletTest.AddBox({10.0f, 1.0f, 10.0f}, floor.position, &floor);
    staticObjects.push_back(floor);
    
    // シミュレーションループ
    const int MAX_FRAMES = 100;
    const float TIME_STEP = 0.016f; // 60FPS
    
    for (int frame = 0; frame < MAX_FRAMES; frame++) {
        std::cout << "Frame: " << frame << std::endl;
        
        // 動的オブジェクトの更新
        for (auto& obj : dynamicObjects) {
            // 前回の衝突フラグをリセット
            obj.isColliding = false;
            
            // 重力適用（簡易版）
            obj.velocity.y -= 0.1f * TIME_STEP;
            
            // 位置の更新
            obj.position.x += obj.velocity.x;
            obj.position.y += obj.velocity.y;
            obj.position.z += obj.velocity.z;
            
            // Bulletのオブジェクト位置を更新
            bulletTest.SetPosition(obj.bulletObject, obj.position);
            
            // 位置の出力
            std::cout << "  Object position: " << obj.position.x << ", " << obj.position.y << ", " << obj.position.z;
            std::cout << (obj.isColliding ? " (Colliding)" : "") << std::endl;
        }
        
        // 衝突検出更新
        bulletTest.Update();
    }
    
    std::cout << "Bullet Physics Test completed." << std::endl;
}

// メイン関数（テスト用）
// 実際のプロジェクトでは必要に応じて変更または削除してください
int main() {
    RunBulletTest();
    return 0;
}
