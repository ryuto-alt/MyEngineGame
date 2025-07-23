// BlueTest用のPixel Shader（確実に動作確認するためのテスト用）

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
    
    // 元のテクスチャを読み込む
    output.color = gTexture.Sample(gSampler, input.texcoord);
    
    // 【テスト用】強制的に青色に変換（シェーダーが動作しているかの確認）
    output.color.r = 0.0f;  // 赤成分を0に
    output.color.g = 0.0f;  // 緑成分を0に
    output.color.b = 1.0f;  // 青成分を最大に
    // アルファはそのまま維持
    
    return output;
}