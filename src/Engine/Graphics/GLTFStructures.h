#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Mymath.h"

// glTF 2.0 メタデータ構造
namespace GLTF {
    // 基本的なインデックス型
    using Index = int32_t;
    static const Index INVALID_INDEX = -1;

    // テクスチャ情報
    struct TextureInfo {
        Index index = INVALID_INDEX;     // テクスチャインデックス
        int32_t texCoord = 0;            // テクスチャ座標セット
        float scale = 1.0f;              // スケール（Normal Map用）
        float strength = 1.0f;           // 強度（Occlusion Map用）
    };

    // PBRメタリックラフネス
    struct PbrMetallicRoughness {
        Vector4 baseColorFactor = {1.0f, 1.0f, 1.0f, 1.0f};  // ベースカラー係数
        std::optional<TextureInfo> baseColorTexture;          // ベースカラーテクスチャ
        float metallicFactor = 1.0f;                          // メタリック係数
        float roughnessFactor = 1.0f;                         // ラフネス係数
        std::optional<TextureInfo> metallicRoughnessTexture;  // メタリック・ラフネステクスチャ
    };

    // マテリアル
    struct Material {
        std::string name;
        PbrMetallicRoughness pbrMetallicRoughness;
        std::optional<TextureInfo> normalTexture;         // 法線マップ
        std::optional<TextureInfo> occlusionTexture;      // オクルージョンマップ
        std::optional<TextureInfo> emissiveTexture;       // エミッシブマップ
        Vector3 emissiveFactor = {0.0f, 0.0f, 0.0f};     // エミッシブ係数
        std::string alphaMode = "OPAQUE";                 // アルファモード
        float alphaCutoff = 0.5f;                         // アルファカットオフ
        bool doubleSided = false;                         // 両面レンダリング
    };

    // 画像
    struct Image {
        std::string name;
        std::string uri;                    // URI（外部ファイルの場合）
        std::string mimeType;               // MIMEタイプ
        Index bufferView = INVALID_INDEX;   // バッファビューインデックス
    };

    // サンプラー
    struct Sampler {
        std::string name;
        int32_t magFilter = 9729;    // GL_LINEAR
        int32_t minFilter = 9987;    // GL_LINEAR_MIPMAP_LINEAR
        int32_t wrapS = 10497;       // GL_REPEAT
        int32_t wrapT = 10497;       // GL_REPEAT
    };

    // テクスチャ
    struct Texture {
        std::string name;
        Index sampler = INVALID_INDEX;  // サンプラーインデックス
        Index source = INVALID_INDEX;   // 画像インデックス
    };

    // プリミティブ属性
    struct Attributes {
        Index position = INVALID_INDEX;
        Index normal = INVALID_INDEX;
        Index tangent = INVALID_INDEX;
        Index texcoord_0 = INVALID_INDEX;
        Index texcoord_1 = INVALID_INDEX;
        Index color_0 = INVALID_INDEX;
        Index joints_0 = INVALID_INDEX;
        Index weights_0 = INVALID_INDEX;
    };

    // プリミティブ
    struct Primitive {
        Attributes attributes;
        Index indices = INVALID_INDEX;      // インデックスアクセサ
        Index material = INVALID_INDEX;     // マテリアルインデックス
        int32_t mode = 4;                   // GL_TRIANGLES
    };

    // メッシュ
    struct Mesh {
        std::string name;
        std::vector<Primitive> primitives;
    };

    // ノード
    struct Node {
        std::string name;
        std::vector<Index> children;        // 子ノードインデックス
        Matrix4x4 matrix = MakeIdentity4x4(); // 変換行列（matrixまたはTRS）
        Vector3 translation = {0.0f, 0.0f, 0.0f};  // 平行移動
        Vector4 rotation = {0.0f, 0.0f, 0.0f, 1.0f}; // 回転（クォータニオン）
        Vector3 scale = {1.0f, 1.0f, 1.0f};         // スケール
        Index mesh = INVALID_INDEX;         // メッシュインデックス
        Index skin = INVALID_INDEX;         // スキンインデックス
    };

    // シーン
    struct Scene {
        std::string name;
        std::vector<Index> nodes;           // ルートノードインデックス
    };

    // バッファ
    struct Buffer {
        std::string name;
        std::string uri;                    // URI（外部ファイルの場合）
        size_t byteLength = 0;              // バッファサイズ
    };

    // バッファビュー
    struct BufferView {
        std::string name;
        Index buffer = INVALID_INDEX;       // バッファインデックス
        size_t byteOffset = 0;              // バイトオフセット
        size_t byteLength = 0;              // バイト長
        size_t byteStride = 0;              // バイトストライド
        int32_t target = 0;                 // ターゲット（頂点・インデックス）
    };

    // アクセサ
    struct Accessor {
        std::string name;
        Index bufferView = INVALID_INDEX;   // バッファビューインデックス
        size_t byteOffset = 0;              // バイトオフセット
        int32_t componentType = 0;          // コンポーネントタイプ
        bool normalized = false;            // 正規化フラグ
        size_t count = 0;                   // 要素数
        std::string type;                   // タイプ（SCALAR, VEC2, VEC3, VEC4, MAT2, MAT3, MAT4）
        std::vector<double> max;            // 最大値
        std::vector<double> min;            // 最小値
    };

    // アセット情報
    struct Asset {
        std::string copyright;
        std::string generator;
        std::string version = "2.0";        // glTFバージョン
        std::string minVersion;
    };

    // メインのglTFドキュメント
    struct Document {
        Asset asset;
        std::optional<Index> scene;         // デフォルトシーンインデックス
        std::vector<Scene> scenes;
        std::vector<Node> nodes;
        std::vector<Mesh> meshes;
        std::vector<Material> materials;
        std::vector<Texture> textures;
        std::vector<Image> images;
        std::vector<Sampler> samplers;
        std::vector<Buffer> buffers;
        std::vector<BufferView> bufferViews;
        std::vector<Accessor> accessors;
        
        // 拡張機能（必要に応じて追加）
        std::unordered_map<std::string, std::string> extensions;
        std::vector<std::string> extensionsUsed;
        std::vector<std::string> extensionsRequired;
    };

    // コンポーネントタイプ定数
    namespace ComponentType {
        static const int32_t BYTE = 5120;
        static const int32_t UNSIGNED_BYTE = 5121;
        static const int32_t SHORT = 5122;
        static const int32_t UNSIGNED_SHORT = 5123;
        static const int32_t UNSIGNED_INT = 5125;
        static const int32_t FLOAT = 5126;
    }

    // プリミティブモード定数
    namespace PrimitiveMode {
        static const int32_t POINTS = 0;
        static const int32_t LINES = 1;
        static const int32_t LINE_LOOP = 2;
        static const int32_t LINE_STRIP = 3;
        static const int32_t TRIANGLES = 4;
        static const int32_t TRIANGLE_STRIP = 5;
        static const int32_t TRIANGLE_FAN = 6;
    }

    // バッファビューターゲット定数
    namespace BufferTarget {
        static const int32_t ARRAY_BUFFER = 34962;
        static const int32_t ELEMENT_ARRAY_BUFFER = 34963;
    }
}