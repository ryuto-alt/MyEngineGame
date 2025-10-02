#include "AABBCollision.h"
#include "Object3d.h"
#include "Model.h"
#include "AnimatedModel.h"
#include "../externals/tinygltf/tiny_gltf.h"
#include <algorithm>
#include <cassert>

namespace Collision {

    // 静的メンバの初期化
    AABBCollisionManager* AABBCollisionManager::instance_ = nullptr;

    // AABBの実装
    AABB::AABB() {
        min = Vector3{ 0.0f, 0.0f, 0.0f };
        max = Vector3{ 0.0f, 0.0f, 0.0f };
    }

    AABB::AABB(const Vector3& minPoint, const Vector3& maxPoint) {
        min = minPoint;
        max = maxPoint;
    }

    Vector3 AABB::GetCenter() const {
        Vector3 result;
        result.x = (min.x + max.x) * 0.5f;
        result.y = (min.y + max.y) * 0.5f;
        result.z = (min.z + max.z) * 0.5f;
        return result;
    }

    Vector3 AABB::GetSize() const {
        Vector3 result;
        result.x = max.x - min.x;
        result.y = max.y - min.y;
        result.z = max.z - min.z;
        return result;
    }

    Vector3 AABB::GetHalfSize() const {
        Vector3 result;
        result.x = (max.x - min.x) * 0.5f;
        result.y = (max.y - min.y) * 0.5f;
        result.z = (max.z - min.z) * 0.5f;
        return result;
    }

    // トランスフォーム適用後のAABBを計算
    AABB TransformAABB(const AABB& aabb, const Vector3& position, const Vector3& scale) {
        // スケールを適用
        Vector3 scaledMin = {
            aabb.min.x * scale.x,
            aabb.min.y * scale.y,
            aabb.min.z * scale.z
        };
        Vector3 scaledMax = {
            aabb.max.x * scale.x,
            aabb.max.y * scale.y,
            aabb.max.z * scale.z
        };

        // スケールが負の場合、min/maxが入れ替わる
        if (scale.x < 0) std::swap(scaledMin.x, scaledMax.x);
        if (scale.y < 0) std::swap(scaledMin.y, scaledMax.y);
        if (scale.z < 0) std::swap(scaledMin.z, scaledMax.z);

        // 位置を適用
        AABB result;
        result.min = {
            scaledMin.x + position.x,
            scaledMin.y + position.y,
            scaledMin.z + position.z
        };
        result.max = {
            scaledMax.x + position.x,
            scaledMax.y + position.y,
            scaledMax.z + position.z
        };

        return result;
    }

    // AABB同士の衝突判定
    bool CheckAABBCollision(const AABB& a, const AABB& b) {
        return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
               (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
               (a.min.z <= b.max.z && a.max.z >= b.min.z);
    }

    // GLTFモデルのアクセサーからAABBを抽出
    AABB AABBExtractor::ExtractFromGLTF(const void* gltfModelPtr, int meshIndex, int primitiveIndex) {
        const tinygltf::Model* gltfModel = static_cast<const tinygltf::Model*>(gltfModelPtr);

        if (!gltfModel || meshIndex >= gltfModel->meshes.size()) {
            return AABB();  // デフォルトAABB
        }

        const auto& mesh = gltfModel->meshes[meshIndex];
        if (primitiveIndex >= mesh.primitives.size()) {
            return AABB();
        }

        const auto& primitive = mesh.primitives[primitiveIndex];
        auto posIt = primitive.attributes.find("POSITION");
        if (posIt == primitive.attributes.end()) {
            return AABB();
        }

        const auto& accessor = gltfModel->accessors[posIt->second];

        // min/maxがない場合はデフォルト
        if (accessor.minValues.size() < 3 || accessor.maxValues.size() < 3) {
            return AABB();
        }

        AABB result;
        result.min = {
            static_cast<float>(accessor.minValues[0]),
            static_cast<float>(accessor.minValues[1]),
            static_cast<float>(accessor.minValues[2])
        };
        result.max = {
            static_cast<float>(accessor.maxValues[0]),
            static_cast<float>(accessor.maxValues[1]),
            static_cast<float>(accessor.maxValues[2])
        };

        return result;
    }

    // ModelクラスからAABBを抽出
    AABB AABBExtractor::ExtractFromModel(const Model* model) {
        if (!model) {
            return AABB();
        }

        // ModelクラスからGLTFデータを取得する必要がある
        // ここではモデルの頂点データから計算する簡易実装
        const auto& vertices = model->GetVertices();
        if (vertices.empty()) {
            return AABB();
        }

        Vector3 minPoint = {
            vertices[0].position.x,
            vertices[0].position.y,
            vertices[0].position.z
        };
        Vector3 maxPoint = minPoint;

        for (const auto& vertex : vertices) {
            Vector3 pos = { vertex.position.x, vertex.position.y, vertex.position.z };

            minPoint.x = std::min(minPoint.x, pos.x);
            minPoint.y = std::min(minPoint.y, pos.y);
            minPoint.z = std::min(minPoint.z, pos.z);

            maxPoint.x = std::max(maxPoint.x, pos.x);
            maxPoint.y = std::max(maxPoint.y, pos.y);
            maxPoint.z = std::max(maxPoint.z, pos.z);
        }

        return AABB(minPoint, maxPoint);
    }

    // AnimatedModelクラスからAABBを抽出
    AABB AABBExtractor::ExtractFromAnimatedModel(const AnimatedModel* model) {
        if (!model) {
            return AABB();
        }

        // AnimatedModelはModelを継承しているので同じ方法で取得
        return ExtractFromModel(static_cast<const Model*>(model));
    }

    // CollisionObject3Dの実装
    CollisionObject3D::CollisionObject3D(Object3d* object, const AABB& localAABB)
        : object_(object), localAABB_(localAABB), worldAABB_(), enabled_(true) {
        Update();
    }

    void CollisionObject3D::Update() {
        if (!object_) return;

        // Object3dから位置とスケールを取得
        Vector3 position = object_->GetPosition();
        Vector3 scale = object_->GetScale();

        // 回転は今回は考慮しない(AABBは軸並行のため)
        // より正確な衝突判定が必要な場合はOBB(Oriented Bounding Box)を使用する
        worldAABB_ = TransformAABB(localAABB_, position, scale);
    }

    // AABBCollisionManagerの実装
    AABBCollisionManager* AABBCollisionManager::GetInstance() {
        return instance_;
    }

    void AABBCollisionManager::Create() {
        if (!instance_) {
            instance_ = new AABBCollisionManager();
        }
    }

    void AABBCollisionManager::Destroy() {
        if (instance_) {
            delete instance_;
            instance_ = nullptr;
        }
    }

    void AABBCollisionManager::RegisterObject(Object3d* object, const AABB& localAABB, bool enabled) {
        if (!object) return;

        // 既に登録されているか確認
        auto existing = FindCollisionObject(object);
        if (existing) {
            existing->SetLocalAABB(localAABB);
            existing->SetEnabled(enabled);
            return;
        }

        // 新規登録
        auto collisionObj = std::make_shared<CollisionObject3D>(object, localAABB);
        collisionObj->SetEnabled(enabled);
        collisionObjects_.push_back(collisionObj);
    }

    void AABBCollisionManager::UnregisterObject(Object3d* object) {
        collisionObjects_.erase(
            std::remove_if(collisionObjects_.begin(), collisionObjects_.end(),
                [object](const std::shared_ptr<CollisionObject3D>& obj) {
                    return obj->GetObject() == object;
                }),
            collisionObjects_.end()
        );
    }

    void AABBCollisionManager::Clear() {
        collisionObjects_.clear();
    }

    void AABBCollisionManager::Update() {
        // 全オブジェクトのワールドAABBを更新
        for (auto& obj : collisionObjects_) {
            if (obj->IsEnabled()) {
                obj->Update();
            }
        }

        // 衝突判定
        if (collisionCallback_) {
            for (size_t i = 0; i < collisionObjects_.size(); ++i) {
                if (!collisionObjects_[i]->IsEnabled()) continue;

                for (size_t j = i + 1; j < collisionObjects_.size(); ++j) {
                    if (!collisionObjects_[j]->IsEnabled()) continue;

                    if (CheckAABBCollision(
                        collisionObjects_[i]->GetWorldAABB(),
                        collisionObjects_[j]->GetWorldAABB())) {

                        collisionCallback_(
                            collisionObjects_[i]->GetObject(),
                            collisionObjects_[j]->GetObject()
                        );
                    }
                }
            }
        }
    }

    std::shared_ptr<CollisionObject3D> AABBCollisionManager::FindCollisionObject(Object3d* object) {
        auto it = std::find_if(collisionObjects_.begin(), collisionObjects_.end(),
            [object](const std::shared_ptr<CollisionObject3D>& obj) {
                return obj->GetObject() == object;
            });

        return (it != collisionObjects_.end()) ? *it : nullptr;
    }

} // namespace Collision
