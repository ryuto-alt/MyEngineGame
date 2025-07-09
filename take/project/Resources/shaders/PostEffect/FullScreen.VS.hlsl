#include "PostEffect/FullScreen.hlsli"

static const uint kNumVertex = 3;

static const float4 kPositions[kNumVertex] = {
	float4(-1.0f, 1.0f, 0.0f, 1.0f), //LeftTop
	float4( 3.0f, 1.0f, 0.0f, 1.0f), //RightTop
	float4(-1.0f,-3.0f, 0.0f, 1.0f)  //LeftBottom
};
static const float2 kTexCoords[kNumVertex] = {
	float2(0.0f, 0.0f), //LeftTop
	float2(2.0f, 0.0f), //RightTop
	float2(0.0f, 2.0f)  //LeftBottom
};

VertexShaderOutput main(uint vertexId : SV_VertexID) {
	VertexShaderOutput output;
	output.position = kPositions[vertexId];
	output.texcoord = kTexCoords[vertexId];
	return output;
}