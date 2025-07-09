#include "Object3d.hlsli"

struct Material {
	float4 color; //カラー
	float4x4 uvTransform;
	int enableLighting; //Lightingを有効にするフラグ
	float shininess;
	float envCoefficient; //環境マップの反射係数
};

struct DirectionalLight {
	float4 color;     //ライトのカラー	
	float3 direction; //ライトの向き
	float intensity;  //輝度
};

struct PointLight {
	float4 color;    //ライトのカラー	
	float3 position; //ライトの位置
	float intensity; //輝度
	float radius;    //ライトの届く最大距離
	float decay;     //減衰率
};

struct SpotLight {
	float4 color;        //ライトのカラー	
	float3 position;     //ライトの位置
	float intensity;     //輝度
	float3 direction;    //ライトの向き
	float distance;      //ライトの届く最大距離
	float decay;         //減衰率
	float cosAngle;      //スポットライトの角度
	float penumbraAngle; //影のぼかし角度
};

struct Camera {
	float3 worldPosition;
};

//マテリアル
ConstantBuffer<Material> gMaterial : register(b0);
//並行光源
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
//カメラ
ConstantBuffer<Camera> gCamera : register(b2);
//ポイントライト
ConstantBuffer<PointLight> gPointLight : register(b3);
//スポットライト
ConstantBuffer<SpotLight> gSpotLight : register(b4);

//テクスチャ
Texture2D<float4> gTexture : register(t0);
//環境マップ用テクスチャ
TextureCube<float4> gEnvMapTexture : register(t1);
//サンプラー
SamplerState gSampler : register(s0);

struct PixelShaderOutPut {
	float4 color : SV_TARGET0;
};

PixelShaderOutPut main(VertexShaderOutput input) {
	PixelShaderOutPut output;

	float4 transformedUV = mul(float4(input.texcoord,0.0f,1.0f), gMaterial.uvTransform);
	float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
	
	if (textureColor.a < 0.5f) { discard; }

	//Lightingの計算
	if (gMaterial.enableLighting == 1) {
		
		//DirectionalLightの計算
		float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction); 
		float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
		
		//specular反射の計算
		float3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
		float3 halfVector = normalize(-gDirectionalLight.direction + toEye);
		float NdotH = dot(normalize(input.normal), halfVector);
		float specularPow = pow(saturate(NdotH), gMaterial.shininess);
		
		//pointLightの計算
		float3 pointLightDirection = normalize(gPointLight.position - input.worldPosition);
		float cosPoint = pow(dot(input.normal, pointLightDirection) * 0.5f + 0.5f, 2.0f);
		float3 halfVectorPoint = normalize(pointLightDirection + toEye);
		float NdotHPoint = dot(normalize(input.normal), halfVectorPoint);
		float specularPowPoint = pow(saturate(NdotHPoint), gMaterial.shininess);
		//光の減衰計算
		float distancePoint = length(gPointLight.position - input.worldPosition); //ポイントライトへの距離
		float factor = pow(saturate(-distancePoint / gPointLight.radius + 1.0f), gPointLight.decay); //逆二乗則による減衰係数
		
		//spotLightの計算
		float3 spotLightDirectionOfSurface = normalize(gSpotLight.position - input.worldPosition);
		float3 harfVectorSpot = normalize(spotLightDirectionOfSurface + toEye);
		float NdotHSpot = dot(normalize(input.normal), harfVectorSpot);
		float specularPowSpot = pow(saturate(NdotHSpot), gMaterial.shininess);
		//距離による減衰
		float distanceSpot = length(gSpotLight.position - input.worldPosition);
		float attenuationFactor = pow(saturate(-distanceSpot / gSpotLight.distance + 1.0f), gSpotLight.decay);
		//Falloff
		float cosAngle = dot(spotLightDirectionOfSurface, gSpotLight.direction);
		float falloffFactor = saturate((cosAngle - gSpotLight.cosAngle) / (gSpotLight.penumbraAngle - gSpotLight.cosAngle));
		
		//環境マップの計算
		float3 cameraToPosition = normalize(input.worldPosition - gCamera.worldPosition);
		float3 reflectedVector = reflect(cameraToPosition, normalize(input.normal));
		float4 environmentColor = gEnvMapTexture.Sample(gSampler, reflectedVector);
		
		//Diffuse_Reflection
		float3 diffuseDir = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
		float3 diffusePoint = gMaterial.color.rgb * textureColor.rgb * gPointLight.color.rgb * cosPoint* gPointLight.intensity * factor;
		float3 diffuseSpot = gMaterial.color.rgb * textureColor.rgb * gSpotLight.color.rgb * gSpotLight.intensity * attenuationFactor * falloffFactor;
		
		//Specular_Reflection
		float3 specularDir = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float3(1.0f, 1.0f, 1.0f);
		float3 specularPoint = gPointLight.color.rgb * gPointLight.intensity * factor * specularPowPoint * float3(1.0f, 1.0f, 1.0f);
		float3 specularSpot = gSpotLight.color.rgb * gSpotLight.intensity * falloffFactor * attenuationFactor * specularPowSpot * float3(1.0f, 1.0f, 1.0f);
			
		//拡散反射+鏡面反射
		output.color.rgb = diffuseDir + specularDir + diffusePoint + specularPoint + diffuseSpot + specularSpot;
		//環境マップ
		output.color.rgb += environmentColor.rgb * gMaterial.envCoefficient;
		//アルファ値
		output.color.a = gMaterial.color.a * textureColor.a;
		
	}else if (gMaterial.enableLighting == 0) { //Lightingしない場合。前回まで同じ計算
		output.color.rgb = gMaterial.color.rgb * textureColor.rgb;
		output.color.a = gMaterial.color.a * textureColor.a;
		//output.color.rgb = saturate(input.normal);
	}
	
	return output;
}