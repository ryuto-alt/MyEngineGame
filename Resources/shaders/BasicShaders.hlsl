// 基本的な頂点シェーダーとピクセルシェーダー

// 頂点シェーダー入力
struct VSInput {
    float4 position : POSITION;
    float2 texcoord : TEXCOORD;
    float3 normal : NORMAL;
};

// ピクセルシェーダー入力
struct PSInput {
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
    float3 normal : NORMAL;
};

// 定数バッファ
cbuffer TransformationMatrix : register(b0) {
    float4x4 WVP;        // ワールド・ビュー・プロジェクション合成行列
    float4x4 World;      // ワールド変換行列
};

// 頂点シェーダー
PSInput VSMain(VSInput input) {
    PSInput output;
    
    // 座標変換
    output.position = mul(input.position, WVP);
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(float4(input.normal, 0.0f), World).xyz);
    
    return output;
}

// ピクセルシェーダー
float4 PSMain(PSInput input) : SV_TARGET {
    // 簡易的なライティング (環境光 + 拡散反射光)
    float3 lightDir = normalize(float3(1.0, 1.0, -1.0));  // 光の方向
    float3 normal = normalize(input.normal);
    
    // 拡散反射の計算
    float diffuse = max(0.0, dot(normal, lightDir));
    
    // 環境光の設定
    float ambient = 0.3;
    
    // 最終的な色
    float3 finalColor = float3(1.0, 1.0, 1.0) * (ambient + diffuse);
    
    return float4(finalColor, 1.0);
}