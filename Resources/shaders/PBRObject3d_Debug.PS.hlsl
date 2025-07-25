// PBRオブジェクト3D デバッグ用ピクセルシェーダー
// マテリアルデータが正しく渡されているか確認するためのシェーダー

struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
    float32_t3 worldPosition : POSITION0;
    float32_t3 tangent : TANGENT0;
    float32_t3 bitangent : BITANGENT0;
};

// PBRマテリアル構造体
struct PBRMaterial
{
    // Base Color
    float32_t4 baseColorFactor;
    
    // Metallic & Roughness
    float32_t metallicFactor;
    float32_t roughnessFactor;
    float32_t normalScale;
    float32_t occlusionStrength;
    
    // Emissive
    float32_t3 emissiveFactor;
    float32_t alphaCutoff;
    
    // Flags
    int32_t hasBaseColorTexture;
    int32_t hasMetallicRoughnessTexture;
    int32_t hasNormalTexture;
    int32_t hasOcclusionTexture;
    int32_t hasEmissiveTexture;
    int32_t enableLighting;
    int32_t alphaMode; // 0=OPAQUE, 1=MASK, 2=BLEND
    int32_t doubleSided;
    
    // UV変換
    float32_t4x4 uvTransform;
    
    // パディング
    float32_t padding[2];
};

// Constant Buffers
ConstantBuffer<PBRMaterial> gPBRMaterial : register(b0);

// Textures
Texture2D<float32_t4> gBaseColorTexture : register(t0);

// Samplers
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    // デバッグモード: マテリアルのbaseColorFactorを直接出力
    // これにより、CPUから渡されたマテリアルデータが正しくGPUに届いているか確認できます
    
    // baseColorFactorの値を直接使用（ガンマ補正なし）
    float32_t4 debugColor = gPBRMaterial.baseColorFactor;
    
    // デバッグ用の可視化オプション（コメントアウトして切り替え）
    
    // オプション1: baseColorFactorをそのまま表示
    output.color = debugColor;
    
    // オプション2: 各チャンネルを個別に確認（赤チャンネル）
    // output.color = float32_t4(debugColor.r, 0.0f, 0.0f, 1.0f);
    
    // オプション3: 各チャンネルを個別に確認（緑チャンネル）
    // output.color = float32_t4(0.0f, debugColor.g, 0.0f, 1.0f);
    
    // オプション4: 各チャンネルを個別に確認（青チャンネル）
    // output.color = float32_t4(0.0f, 0.0f, debugColor.b, 1.0f);
    
    // オプション5: metallicFactorを赤で表示
    // output.color = float32_t4(gPBRMaterial.metallicFactor, 0.0f, 0.0f, 1.0f);
    
    // オプション6: roughnessFactorを緑で表示
    // output.color = float32_t4(0.0f, gPBRMaterial.roughnessFactor, 0.0f, 1.0f);
    
    // オプション7: テクスチャの有無を確認
    // if (gPBRMaterial.hasBaseColorTexture > 0)
    // {
    //     // テクスチャがある場合は黄色
    //     output.color = float32_t4(1.0f, 1.0f, 0.0f, 1.0f);
    // }
    // else
    // {
    //     // テクスチャがない場合はマゼンタ
    //     output.color = float32_t4(1.0f, 0.0f, 1.0f, 1.0f);
    // }
    
    // オプション8: UV座標を色として表示
    // output.color = float32_t4(input.texcoord.x, input.texcoord.y, 0.0f, 1.0f);
    
    // オプション9: 法線を色として表示
    // float32_t3 normalColor = normalize(input.normal) * 0.5f + 0.5f;
    // output.color = float32_t4(normalColor, 1.0f);
    
    // オプション10: 固定色で表示（シェーダーが動作しているか確認）
    // output.color = float32_t4(1.0f, 0.0f, 1.0f, 1.0f); // マゼンタ
    
    return output;
}