#include "object3d.hlsli"

struct Material
{
    float32_t4 color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
};

struct DirectionalLight
{
    float32_t4 color;
    float32_t3 direction;
    float intensity;
};

struct SpotLight
{
    float32_t3 position;       // スポットライトの位置
    float range;               // ライトの最大範囲
    float32_t3 direction;      // スポットライトの方向（正規化済み）
    float innerCone;           // 内側コーンのcos値（完全な明るさ）
    float32_t4 color;          // ライトの色
    float outerCone;           // 外側コーンのcos値（減衰開始）
    float intensity;           // ライトの強度
    float32_t2 attenuation;    // 減衰係数 (linear, quadratic)
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSample : register(s0);

ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<SpotLight> gSpotLight : register(b2);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSample, transformedUV.xy);
    
    // アルファ値が闾値未満の場合、そのピクセルを描画しない
    if (textureColor.a < 0.1f)
    {
        discard; // このピクセルの処理を中止
    }
    
    PixelShaderOutput output;
    
    // 正規化された法線を使用（念のため再度正規化）
    float3 normal = normalize(input.normal);
    
    if (gMaterial.enableLighting != 0)
    {
        // 基本の環境光
        float3 ambient = float3(0.05f, 0.05f, 0.05f);  // モードによって変更される
        float3 finalColor = gMaterial.color.rgb * textureColor.rgb * ambient;
        
        // enableLightingがライティングモードを表す
        // 1 = ディレクショナルライトのみ
        // 2 = スポットライトのみ
        // 3 = 両方
        
        // ディレクショナルライト
        if (gMaterial.enableLighting == 1 || gMaterial.enableLighting == 3)
        {
            float3 lightDir = normalize(-gDirectionalLight.direction);
            float NdotL = max(0.0f, dot(normal, lightDir));
            float3 directionalContribution = gDirectionalLight.color.rgb * gDirectionalLight.intensity * NdotL;
            finalColor += gMaterial.color.rgb * textureColor.rgb * directionalContribution;
        }
        
        // スポットライト
        if (gMaterial.enableLighting == 2 || gMaterial.enableLighting == 3)
        {
            // スポットライトモードの場合は環境光を非常に暗くする
            if (gMaterial.enableLighting == 2)
            {
                // ホラーゲーム風の暗さにリセット
                finalColor = gMaterial.color.rgb * textureColor.rgb * float3(0.01f, 0.01f, 0.01f);
            }
            
            float3 lightVector = gSpotLight.position - input.worldPosition;
            float distance = length(lightVector);
            
            // 範囲チェック
            if (distance <= gSpotLight.range)
            {
                // ライトベクトルを正規化
                lightVector = normalize(lightVector);
                
                // スポットライトのコーン内かチェック
                float cosAngle = dot(gSpotLight.direction, -lightVector);
                
                // 外側コーンの範囲内か？
                if (cosAngle > gSpotLight.outerCone)
                {
                    // コーンの減衰を計算
                    float spotAttenuation = 1.0f;
                    if (cosAngle < gSpotLight.innerCone)
                    {
                        spotAttenuation = smoothstep(gSpotLight.outerCone, gSpotLight.innerCone, cosAngle);
                    }
                    
                    // 距離による減衰
                    float distanceAttenuation = 1.0f / (1.0f + gSpotLight.attenuation.x * distance + 
                                                       gSpotLight.attenuation.y * distance * distance);
                    
                    // ランバートの拡散反射
                    float NdotL = max(0.0f, dot(normal, lightVector));
                    
                    // 最終的なライトの強度
                    float3 lightContribution = gSpotLight.color.rgb * gSpotLight.intensity * 
                                             NdotL * spotAttenuation * distanceAttenuation;
                    
                    finalColor += gMaterial.color.rgb * textureColor.rgb * lightContribution;
                }
            }
        }
        
        output.color.rgb = finalColor;
        output.color.a = gMaterial.color.a * textureColor.a;
    }
    else
    {
        output.color = gMaterial.color * textureColor;
    }
    
    return output;
}