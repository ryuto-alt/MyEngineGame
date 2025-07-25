#include "PBRObject3d.hlsli"

// Constant Buffers
ConstantBuffer<PBRMaterial> gPBRMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<SpotLight> gSpotLight : register(b2);
ConstantBuffer<CameraData> gCameraData : register(b3);

// Textures
Texture2D<float32_t4> gBaseColorTexture : register(t0);
TextureCube<float32_t4> gEnvironmentMap : register(t2);

// Samplers
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    // デバッグ：位置に基づいて色を生成
    // これにより、各オブジェクトが異なる位置にあることを視覚的に確認できる
    float32_t3 debugColor;
    debugColor.r = saturate((input.worldPosition.x + 15.0f) / 30.0f); // Xに基づいて赤
    debugColor.g = saturate((input.worldPosition.y + 5.0f) / 10.0f);   // Yに基づいて緑
    debugColor.b = saturate((input.worldPosition.z + 5.0f) / 10.0f);   // Zに基づいて青
    
    // マテリアルカラーとデバッグカラーを混合
    float32_t4 baseColor = gPBRMaterial.baseColorFactor;
    float32_t3 finalColor = lerp(debugColor, baseColor.rgb, 0.7f); // 70%マテリアルカラー、30%デバッグカラー
    
    output.color = float32_t4(finalColor, baseColor.a);
    return output;
}