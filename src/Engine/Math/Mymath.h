#pragma once
#include "Matrix4x4.h"
#include "Matrix3x3.h"
#include "Vector4.h"
#include "Vector3.h"
#include "Vector2.h"
#include <assert.h>
#include <cmath>
#include <stdio.h>
#include <vector>
#include <string>
#include <map>
#include <AnimationUtility.h>

//float Cot(float theta);

Matrix4x4 MakeIdentity4x4();
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);
Matrix4x4 MakeRotateMatrix(const Vector3& rotate);
Matrix4x4 MakeRotateMatrix(const Vector4& rotate);
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);
Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector4& rotate, const Vector3& translate);
Matrix4x4 Inverse(const Matrix4x4& m);
//Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);
//Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearclip, float farclip);
//Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);
Matrix4x4 MakeScaleMatrix(const Vector3& scale);
Matrix4x4 MakeRotateXMatrix(float radian);
Matrix4x4 MakeRotateYMatrix(float radian);
Matrix4x4 MakeRotateZMatrix(float radian);
Matrix4x4 MakeTranslateMatrix(const Vector3& translate);
Matrix4x4 Transpose(const Matrix4x4& m);

// 補間関数
Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);
Vector4 Slerp(const Vector4& q1, const Vector4& q2, float t);

struct VertexData {
    Vector4 position;
    Vector2 texcoord;
    Vector3 normal;
};

struct Material {
    Vector4 color;
    int32_t enableLighting;
    int32_t enableEnvironmentMap;
    float padding[2];
    Matrix4x4 uvTransform;
};

struct TransformationMatrix {
    Matrix4x4 WVP;
    Matrix4x4 World;
};

struct DirectionalLight {
    Vector4 color;
    Vector3 direction;
    float intensity;
};

struct SpotLight {
    Vector4 color;        // ライトの色（RGBAで、Aは未使用）
    Vector3 position;     // スポットライトの位置
    float intensity;      // ライトの強度
    Vector3 direction;    // スポットライトの方向
    float innerCone;      // 内側コーン角度（cos値）
    Vector3 attenuation;  // 減衰パラメータ（定数、線形、二次）
    float outerCone;      // 外側コーン角度（cos値）
};

struct Transform {
    Vector3 scale;
    Vector3 rotate;
    Vector3 translate;
};

// マテリアルデータ構造体の定義
struct MaterialData {
    std::string textureFilePath;  // テクスチャファイルパス
    Vector4 ambient = { 0.1f, 0.1f, 0.1f, 1.0f };  // 環境光(Ka)
    Vector4 diffuse = { 0.8f, 0.8f, 0.8f, 1.0f };  // 拡散反射光(Kd)
    Vector4 specular = { 0.0f, 0.0f, 0.0f, 1.0f }; // 鏡面反射光(Ks)
    float shininess = 0.0f;                      // 光沢度(Ns)
    float alpha = 1.0f;                          // 透明度(d)
    Vector2 textureScale = { 1.0f, 1.0f };       // テクスチャスケール(-s option)
    Vector2 textureOffset = { 0.0f, 0.0f };      // テクスチャオフセット(-o option)
};

// ボーンウェイト構造体
struct VertexWeightData {
    float weight;
    uint32_t vectorIndex;
};

struct JointWeightData {
    Matrix4x4 inverseBindPoseMatrix;
    std::vector<VertexWeightData> vertexWeights;
};

struct ModelData {
    std::vector<VertexData>vertices;
    MaterialData material;
    std::map<std::string, JointWeightData> skinClusterData;  // ボーンウェイト情報
    Transform rootTransform;  // ルートノードの変換情報（Blenderで設定されたスケール等）
};