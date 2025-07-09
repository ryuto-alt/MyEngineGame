
struct Well {
	float4x4 skeletonSpaceMatrix;
	float4x4 skeletonSpaceInvTransposeMatrix;
};

struct Vertex {
	float4 position;
	float2 texcoord;
	float3 normal;
};

struct VertexInfluence {
	float4 weight;
	int4 jointIndices;
};

struct SkinningInfomation {
	uint numVertices;
};

StructuredBuffer<Well> gPalette : register(t0);
StructuredBuffer<Vertex> gInputVertices : register(t1);

StructuredBuffer<VertexInfluence> gInfluences : register(t2);

RWStructuredBuffer<Vertex> gOutputVertices : register(u0);

ConstantBuffer<SkinningInfomation> gSkinningInfo : register(b0);

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
	
	uint vertexIndex = DTid.x;
	if (vertexIndex < gSkinningInfo.numVertices) {
	
		Vertex input = gInputVertices[vertexIndex];
	VertexInfluence influence = gInfluences[vertexIndex];
		
		//Skinning後の頂点の計算
		Vertex skinned;
		skinned.texcoord = input.texcoord;
		
		//位置の変換
		skinned.position = mul(input.position, gPalette[influence.jointIndices.x].skeletonSpaceMatrix) * influence.weight.x;
		skinned.position += mul(input.position, gPalette[influence.jointIndices.y].skeletonSpaceMatrix) * influence.weight.y;
		skinned.position += mul(input.position, gPalette[influence.jointIndices.z].skeletonSpaceMatrix) * influence.weight.z;
		skinned.position += mul(input.position, gPalette[influence.jointIndices.w].skeletonSpaceMatrix) * influence.weight.w;
		skinned.position.w = 1.0f;
	
	//法線の変換
		skinned.normal = mul(input.normal, (float3x3) gPalette[influence.jointIndices.x].skeletonSpaceInvTransposeMatrix) * influence.weight.x;
		skinned.normal += mul(input.normal, (float3x3) gPalette[influence.jointIndices.y].skeletonSpaceInvTransposeMatrix) * influence.weight.y;
		skinned.normal += mul(input.normal, (float3x3) gPalette[influence.jointIndices.z].skeletonSpaceInvTransposeMatrix) * influence.weight.z;
		skinned.normal += mul(input.normal, (float3x3) gPalette[influence.jointIndices.w].skeletonSpaceInvTransposeMatrix) * influence.weight.w;
	//正規化しておく
		skinned.normal = normalize(skinned.normal);
		
		gOutputVertices[vertexIndex] = skinned;
	}
}