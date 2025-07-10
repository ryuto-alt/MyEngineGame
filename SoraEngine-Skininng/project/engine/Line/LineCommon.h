#pragma once
#include "DirectXCommon.h"
#include "GraphicsPipeline.h"
#include "SrvManager.h"

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "RenderingData.h"

#include <assert.h>
#include <cmath>
#include <stdio.h>
#include <string>
//#include <Windows.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <Camera.h>
#include <vector>

struct VertexDataLine
{
	Vector3 position;

};

struct LineInstanceData {
	Vector3 start;
	Vector3 end;
	Vector4 color;
};

struct CameraBufferforGpu {
	Matrix4x4 view;
	Matrix4x4 projection;

};

class LineCommon
{
public:
	static LineCommon* GetInstance();
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(DirectXCommon* dxCommon, SrvManager* srvManager);
	//終了
	void Finalize();
	//共通描画設定
	void CommonDraw();

	//更新
	void Update();
	//描画
	void Draw();

	void DrawLine(const Vector3& start, const Vector3& end, const Vector4& color);
	//DXCommon
	DirectXCommon* GetDxCommon()const { return dxCommon_; }
	//SrvMnager
	SrvManager* GetSrvmanager()const { return srvManager_; }


private:
	LineCommon() = default;
	~LineCommon() = default;
	LineCommon(const LineCommon&) = delete;
	LineCommon& operator=(const LineCommon&) = delete;
private:
	//インスタンス
	static LineCommon* instance_;
	DirectXCommon* dxCommon_;
	SrvManager* srvManager_;
	std::unique_ptr<GraphicsPipeline> graphicsPipeline_;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;

	uint32_t instanceSrvIndex_ = UINT32_MAX;
	Microsoft::WRL::ComPtr<ID3D12Resource> instanceResource_;

	LineInstanceData instance = {
		.start = {0.0f, 0.0f, 0.0f},
		.end = {0.0f, 1.0f, 1.0f},
		.color = {1.0f, 0.0f, 0.0f, 1.0f}
	};

	std::vector<VertexDataLine>linevertices = {

		{{0.0f,0.0f,0.0f}},
		{{1.0f,0.0f,0.0f}},

	};
	std::vector<LineInstanceData> instances_; // ← 複数ライン用

	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource;//カメラのデータを送るためのリソース
	CameraBufferforGpu* camerabuffer = nullptr;//カメラのデータをGPUに送るための構造体


};

