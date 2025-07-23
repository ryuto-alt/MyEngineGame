// CopyImage用のPixel Shader
// 資料に基づく実装：RenderTextureの内容をそのままコピー

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
    
    // RenderTextureの内容をそのままサンプリング
    output.color = gTexture.Sample(gSampler, input.texcoord);
    
    return output;
}