#include "CollisionSystem.h"
#include "Model.h"
#include "Object3d.h"
#include <algorithm>
#include <cmath>

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

CollisionSystem* CollisionSystem::instance_ = nullptr;

CollisionSystem* CollisionSystem::GetInstance() {
    return instance_;
}

void CollisionSystem::Create() {
    if (!instance_) {
        instance_ = new CollisionSystem();
    }
}

void CollisionSystem::Destroy() {
    if (instance_) {
        delete instance_;
        instance_ = nullptr;
    }
}

BoundingBox CollisionSystem::CreateBoundingBox(const Model* model) {
    if (!model) {
        return BoundingBox();
    }
    
    const auto& vertices = model->GetVertices();
    if (vertices.empty()) {
        return BoundingBox();
    }
    
    Vector3 minPoint = {vertices[0].position.x, vertices[0].position.y, vertices[0].position.z};
    Vector3 maxPoint = minPoint;
    
    for (const auto& vertex : vertices) {
        Vector3 pos = {vertex.position.x, vertex.position.y, vertex.position.z};
        
        minPoint.x = std::min(minPoint.x, pos.x);
        minPoint.y = std::min(minPoint.y, pos.y);
        minPoint.z = std::min(minPoint.z, pos.z);
        
        maxPoint.x = std::max(maxPoint.x, pos.x);
        maxPoint.y = std::max(maxPoint.y, pos.y);
        maxPoint.z = std::max(maxPoint.z, pos.z);
    }
    
    return BoundingBox(minPoint, maxPoint);
}

BoundingSphere CollisionSystem::CreateBoundingSphere(const Model* model) {
    if (!model) {
        return BoundingSphere();
    }
    
    const auto& vertices = model->GetVertices();
    if (vertices.empty()) {
        return BoundingSphere();
    }
    
    // 中心点を計算
    Vector3 center = {0, 0, 0};
    for (const auto& vertex : vertices) {
        center.x += vertex.position.x;
        center.y += vertex.position.y;
        center.z += vertex.position.z;
    }
    center.x /= static_cast<float>(vertices.size());
    center.y /= static_cast<float>(vertices.size());
    center.z /= static_cast<float>(vertices.size());
    
    // 最大距離を計算
    float maxDistanceSquared = 0.0f;
    for (const auto& vertex : vertices) {
        Vector3 pos = {vertex.position.x, vertex.position.y, vertex.position.z};
        float dx = pos.x - center.x;
        float dy = pos.y - center.y;
        float dz = pos.z - center.z;
        float distanceSquared = dx * dx + dy * dy + dz * dz;
        maxDistanceSquared = std::max(maxDistanceSquared, distanceSquared);
    }
    
    float radius = std::sqrt(maxDistanceSquared);
    return BoundingSphere(center, radius);
}

void CollisionSystem::EnableCollision(Object3d* object, bool useBox) {
    if (!object || !object->GetModel()) {
        return;
    }
    
    // 既存のオブジェクトを探す
    CollisionObject* existing = FindCollisionObject(object);
    if (existing) {
        existing->isEnabled = true;
        existing->useBox = useBox;
        return;
    }
    
    // 新しいコリジョンオブジェクトを作成
    auto collisionObj = std::make_unique<CollisionObject>();
    collisionObj->object = object;
    collisionObj->useBox = useBox;
    collisionObj->isEnabled = true;
    
    // バウンディング情報を生成
    collisionObj->boundingBox = CreateBoundingBox(object->GetModel());
    collisionObj->boundingSphere = CreateBoundingSphere(object->GetModel());
    
    collisionObjects_.push_back(std::move(collisionObj));
}

void CollisionSystem::DisableCollision(Object3d* object) {
    CollisionObject* existing = FindCollisionObject(object);
    if (existing) {
        existing->isEnabled = false;
    }
}

void CollisionSystem::Update() {
    // 有効な当たり判定オブジェクト同士をチェック
    for (size_t i = 0; i < collisionObjects_.size(); ++i) {
        if (!collisionObjects_[i]->isEnabled) continue;
        
        for (size_t j = i + 1; j < collisionObjects_.size(); ++j) {
            if (!collisionObjects_[j]->isEnabled) continue;
            
            CollisionObject* obj1 = collisionObjects_[i].get();
            CollisionObject* obj2 = collisionObjects_[j].get();
            
            bool isColliding = false;
            
            // 当たり判定の種類に応じて計算
            if (obj1->useBox && obj2->useBox) {
                // ボックス同士
                isColliding = CheckBoxToBox(
                    obj1->boundingBox, obj1->object->GetPosition(),
                    obj2->boundingBox, obj2->object->GetPosition()
                );
            }
            else if (!obj1->useBox && !obj2->useBox) {
                // スフィア同士
                isColliding = CheckSphereToSphere(
                    obj1->boundingSphere, obj1->object->GetPosition(),
                    obj2->boundingSphere, obj2->object->GetPosition()
                );
            }
            else {
                // ボックスとスフィア
                if (obj1->useBox) {
                    isColliding = CheckBoxToSphere(
                        obj1->boundingBox, obj1->object->GetPosition(),
                        obj2->boundingSphere, obj2->object->GetPosition()
                    );
                } else {
                    isColliding = CheckBoxToSphere(
                        obj2->boundingBox, obj2->object->GetPosition(),
                        obj1->boundingSphere, obj1->object->GetPosition()
                    );
                }
            }
            
            if (isColliding && collisionCallback_) {
                CollisionInfo info;
                info.isHit = true;
                info.hitObject = obj2->object;
                collisionCallback_(obj1->object, obj2->object, info);
            }
        }
    }
}

bool CollisionSystem::CheckBoxToBox(const BoundingBox& box1, const Vector3& pos1,
                                   const BoundingBox& box2, const Vector3& pos2) {
    // ワールド座標での最小・最大値を計算
    Vector3 min1 = {box1.minPoint.x + pos1.x, box1.minPoint.y + pos1.y, box1.minPoint.z + pos1.z};
    Vector3 max1 = {box1.maxPoint.x + pos1.x, box1.maxPoint.y + pos1.y, box1.maxPoint.z + pos1.z};
    Vector3 min2 = {box2.minPoint.x + pos2.x, box2.minPoint.y + pos2.y, box2.minPoint.z + pos2.z};
    Vector3 max2 = {box2.maxPoint.x + pos2.x, box2.maxPoint.y + pos2.y, box2.maxPoint.z + pos2.z};
    
    // AABB同士の当たり判定
    return (min1.x <= max2.x && max1.x >= min2.x) &&
           (min1.y <= max2.y && max1.y >= min2.y) &&
           (min1.z <= max2.z && max1.z >= min2.z);
}

bool CollisionSystem::CheckSphereToSphere(const BoundingSphere& sphere1, const Vector3& pos1,
                                         const BoundingSphere& sphere2, const Vector3& pos2) {
    // ワールド座標での中心点を計算
    Vector3 center1 = {sphere1.center.x + pos1.x, sphere1.center.y + pos1.y, sphere1.center.z + pos1.z};
    Vector3 center2 = {sphere2.center.x + pos2.x, sphere2.center.y + pos2.y, sphere2.center.z + pos2.z};
    
    // 中心間の距離の2乗
    float dx = center2.x - center1.x;
    float dy = center2.y - center1.y;
    float dz = center2.z - center1.z;
    float distanceSquared = dx * dx + dy * dy + dz * dz;
    
    // 半径の合計の2乗
    float radiusSum = sphere1.radius + sphere2.radius;
    float radiusSumSquared = radiusSum * radiusSum;
    
    return distanceSquared <= radiusSumSquared;
}

bool CollisionSystem::CheckBoxToSphere(const BoundingBox& box, const Vector3& boxPos,
                                      const BoundingSphere& sphere, const Vector3& spherePos) {
    // ワールド座標でのボックスの最小・最大値
    Vector3 boxMin = {box.minPoint.x + boxPos.x, box.minPoint.y + boxPos.y, box.minPoint.z + boxPos.z};
    Vector3 boxMax = {box.maxPoint.x + boxPos.x, box.maxPoint.y + boxPos.y, box.maxPoint.z + boxPos.z};
    
    // ワールド座標でのスフィアの中心
    Vector3 sphereCenter = {sphere.center.x + spherePos.x, sphere.center.y + spherePos.y, sphere.center.z + spherePos.z};
    
    // ボックスに最も近いスフィア上の点を計算
    Vector3 closestPoint;
    closestPoint.x = std::max(boxMin.x, std::min(sphereCenter.x, boxMax.x));
    closestPoint.y = std::max(boxMin.y, std::min(sphereCenter.y, boxMax.y));
    closestPoint.z = std::max(boxMin.z, std::min(sphereCenter.z, boxMax.z));
    
    // スフィアの中心と最近接点の距離の2乗
    float dx = sphereCenter.x - closestPoint.x;
    float dy = sphereCenter.y - closestPoint.y;
    float dz = sphereCenter.z - closestPoint.z;
    float distanceSquared = dx * dx + dy * dy + dz * dz;
    
    return distanceSquared <= (sphere.radius * sphere.radius);
}

CollisionInfo CollisionSystem::RayCast(const Vector3& rayOrigin, const Vector3& rayDirection, float maxDistance) {
    CollisionInfo result;
    float closestDistance = maxDistance;
    
    for (const auto& collisionObj : collisionObjects_) {
        if (!collisionObj->isEnabled) continue;
        
        Object3d* object = collisionObj->object;
        Vector3 objPos = object->GetPosition();
        
        if (collisionObj->useBox) {
            // ボックスとのレイキャスト
            Vector3 boxMin = {collisionObj->boundingBox.minPoint.x + objPos.x,
                             collisionObj->boundingBox.minPoint.y + objPos.y,
                             collisionObj->boundingBox.minPoint.z + objPos.z};
            Vector3 boxMax = {collisionObj->boundingBox.maxPoint.x + objPos.x,
                             collisionObj->boundingBox.maxPoint.y + objPos.y,
                             collisionObj->boundingBox.maxPoint.z + objPos.z};
            
            // レイとAABBの交差判定（簡単な実装）
            Vector3 invDir = {1.0f / rayDirection.x, 1.0f / rayDirection.y, 1.0f / rayDirection.z};
            
            float t1 = (boxMin.x - rayOrigin.x) * invDir.x;
            float t2 = (boxMax.x - rayOrigin.x) * invDir.x;
            float t3 = (boxMin.y - rayOrigin.y) * invDir.y;
            float t4 = (boxMax.y - rayOrigin.y) * invDir.y;
            float t5 = (boxMin.z - rayOrigin.z) * invDir.z;
            float t6 = (boxMax.z - rayOrigin.z) * invDir.z;
            
            float tmin = std::max({std::min(t1, t2), std::min(t3, t4), std::min(t5, t6)});
            float tmax = std::min({std::max(t1, t2), std::max(t3, t4), std::max(t5, t6)});
            
            if (tmax >= 0 && tmin <= tmax && tmin < closestDistance && tmin >= 0) {
                result.isHit = true;
                result.hitPoint = {rayOrigin.x + rayDirection.x * tmin,
                                 rayOrigin.y + rayDirection.y * tmin,
                                 rayOrigin.z + rayDirection.z * tmin};
                result.hitObject = object;
                closestDistance = tmin;
            }
        }
        else {
            // スフィアとのレイキャスト
            Vector3 sphereCenter = {collisionObj->boundingSphere.center.x + objPos.x,
                                   collisionObj->boundingSphere.center.y + objPos.y,
                                   collisionObj->boundingSphere.center.z + objPos.z};
            
            Vector3 oc = {rayOrigin.x - sphereCenter.x, rayOrigin.y - sphereCenter.y, rayOrigin.z - sphereCenter.z};
            float a = rayDirection.x * rayDirection.x + rayDirection.y * rayDirection.y + rayDirection.z * rayDirection.z;
            float b = 2.0f * (oc.x * rayDirection.x + oc.y * rayDirection.y + oc.z * rayDirection.z);
            float c = oc.x * oc.x + oc.y * oc.y + oc.z * oc.z - collisionObj->boundingSphere.radius * collisionObj->boundingSphere.radius;
            
            float discriminant = b * b - 4 * a * c;
            if (discriminant >= 0) {
                float t = (-b - std::sqrt(discriminant)) / (2.0f * a);
                if (t >= 0 && t < closestDistance) {
                    result.isHit = true;
                    result.hitPoint = {rayOrigin.x + rayDirection.x * t,
                                     rayOrigin.y + rayDirection.y * t,
                                     rayOrigin.z + rayDirection.z * t};
                    result.hitObject = object;
                    closestDistance = t;
                }
            }
        }
    }
    
    return result;
}

void CollisionSystem::SetCollisionCallback(std::function<void(Object3d*, Object3d*, const CollisionInfo&)> callback) {
    collisionCallback_ = callback;
}

CollisionSystem::CollisionObject* CollisionSystem::FindCollisionObject(Object3d* object) {
    for (auto& collisionObj : collisionObjects_) {
        if (collisionObj->object == object) {
            return collisionObj.get();
        }
    }
    return nullptr;
}

void CollisionSystem::RemoveCollisionObject(Object3d* object) {
    collisionObjects_.erase(
        std::remove_if(collisionObjects_.begin(), collisionObjects_.end(),
            [object](const std::unique_ptr<CollisionObject>& obj) {
                return obj->object == object;
            }),
        collisionObjects_.end()
    );
}