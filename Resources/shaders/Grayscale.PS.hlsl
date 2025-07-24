// Grayscale用のPixel Shader（最もシンプルな実装）

struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
    float32_t2 texcoord : TEXCOORD0;
};

// RenderTextureをサンプリングするためのテクスチャとサンプラー
Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    // テクスチャをサンプリング
    output.color = gTexture.Sample(gSampler, input.texcoord);
    
    // 人間の視覚特性を考慮したグレースケール変換（ITU-R BT.709準拠）
    // 緑色に最も敏感で、青色に最も敏感でない人間の目の特性を反映
    float32_t gray = 0.2126f * output.color.r + 0.7152f * output.color.g + 0.0722f * output.color.b;
    output.color.r = gray;
    output.color.g = gray;
    output.color.b = gray;
    // アルファはそのまま維持
    
    return output;
}