#include "PBRObject3d.hlsli"

// Constant Buffers
ConstantBuffer<PBRMaterial> gPBRMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

// Textures
Texture2D<float32_t4> gBaseColorTexture : register(t0);
Texture2D<float32_t4> gMetallicRoughnessTexture : register(t1);
Texture2D<float32_t4> gNormalTexture : register(t2);
Texture2D<float32_t4> gOcclusionTexture : register(t3);
Texture2D<float32_t4> gEmissiveTexture : register(t4);

// Samplers
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    // UV座標の変換
    float32_t4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gPBRMaterial.uvTransform);
    float32_t2 uv = transformedUV.xy;
    
    // Base Color の取得
    float32_t4 baseColor = gPBRMaterial.baseColorFactor;
    if (gPBRMaterial.hasBaseColorTexture)
    {
        float32_t4 baseColorTexture = gBaseColorTexture.Sample(gSampler, uv);
        baseColor *= float32_t4(GammaToLinear(baseColorTexture.rgb), baseColorTexture.a);
    }
    
    // Alpha Test
    if (gPBRMaterial.alphaMode == 1) // MASK
    {
        if (baseColor.a < gPBRMaterial.alphaCutoff)
        {
            discard;
        }
    }
    
    // Metallic & Roughness の取得
    float32_t metallic = gPBRMaterial.metallicFactor;
    float32_t roughness = gPBRMaterial.roughnessFactor;
    if (gPBRMaterial.hasMetallicRoughnessTexture)
    {
        float32_t4 metallicRoughnessTexture = gMetallicRoughnessTexture.Sample(gSampler, uv);
        // glTF仕様: G=Roughness, B=Metallic
        roughness *= metallicRoughnessTexture.g;
        metallic *= metallicRoughnessTexture.b;
    }
    
    // 法線の取得
    float32_t3 normal = normalize(input.normal);
    if (gPBRMaterial.hasNormalTexture)
    {
        float32_t3 normalMap = gNormalTexture.Sample(gSampler, uv).rgb;
        normal = ApplyNormalMap(normalMap, input.normal, input.tangent, input.bitangent, gPBRMaterial.normalScale);
    }
    
    // Occlusion の取得
    float32_t occlusion = 1.0;
    if (gPBRMaterial.hasOcclusionTexture)
    {
        occlusion = gOcclusionTexture.Sample(gSampler, uv).r;
        occlusion = lerp(1.0, occlusion, gPBRMaterial.occlusionStrength);
    }
    
    // Emissive の取得
    float32_t3 emissive = gPBRMaterial.emissiveFactor;
    if (gPBRMaterial.hasEmissiveTexture)
    {
        float32_t3 emissiveTexture = gEmissiveTexture.Sample(gSampler, uv).rgb;
        emissive *= GammaToLinear(emissiveTexture);
    }
    
    // ライティングが無効の場合はベースカラーをそのまま出力
    if (!gPBRMaterial.enableLighting)
    {
        output.color = float32_t4(LinearToGamma(baseColor.rgb + emissive), baseColor.a);
        return output;
    }
    
    // PBR計算用の値準備
    float32_t3 albedo = baseColor.rgb;
    float32_t3 V = normalize(-input.worldPosition); // カメラ方向（簡易実装）
    float32_t3 L = normalize(-gDirectionalLight.direction);
    float32_t3 H = normalize(V + L);
    float32_t3 R = reflect(-V, normal);
    
    // F0の計算（金属の場合はalbedo、非金属の場合は0.04）
    float32_t3 F0 = lerp(float32_t3(0.04, 0.04, 0.04), albedo, metallic);
    
    // Cook-Torrance BRDF計算
    float32_t NDF = DistributionGGX(normal, H, roughness);
    float32_t G = GeometrySmith(normal, V, L, roughness);
    float32_t3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
    
    float32_t3 numerator = NDF * G * F;
    float32_t denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, L), 0.0) + 0.0001;
    float32_t3 specular = numerator / denominator;
    
    // エネルギー保存則
    float32_t3 kS = F;
    float32_t3 kD = float32_t3(1.0, 1.0, 1.0) - kS;
    kD *= 1.0 - metallic; // 金属は拡散反射しない
    
    // Lambert拡散
    float32_t3 diffuse = kD * albedo / 3.14159265359;
    
    // 出力反射率の計算
    float32_t NdotL = max(dot(normal, L), 0.0);
    float32_t3 radiance = gDirectionalLight.color.rgb * gDirectionalLight.intensity;
    float32_t3 color = (diffuse + specular) * radiance * NdotL;
    
    // 環境光の簡易実装
    float32_t3 ambient = float32_t3(0.03, 0.03, 0.03) * albedo * occlusion;
    color += ambient;
    
    // エミッシブ追加
    color += emissive;
    
    // HDRトーンマッピング（Reinhard）
    color = color / (color + float32_t3(1.0, 1.0, 1.0));
    
    // ガンマ補正
    color = LinearToGamma(color);
    
    output.color = float32_t4(color, baseColor.a);
    return output;
}