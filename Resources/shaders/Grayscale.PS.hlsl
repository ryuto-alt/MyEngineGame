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
    
    // 最もシンプルなグレースケール変換（平均値使用）
    float32_t gray = (output.color.r + output.color.g + output.color.b) / 3.0f;
    output.color.r = gray;
    output.color.g = gray;
    output.color.b = gray;
    // アルファはそのまま維持
    
    return output;
}