#include "WireFrame/WireFrame.hlsli"

struct PixelShaderOutPut {
	float4 color : SV_TARGET0;
};
PixelShaderOutPut main(VertexShaderOutput input) {
	PixelShaderOutPut output;
	output.color = input.color;
	return output;
}