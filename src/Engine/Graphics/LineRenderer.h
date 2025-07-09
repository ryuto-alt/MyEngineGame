#pragma once
#include "DirectXCommon.h"
#include "SpriteCommon.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include <vector>
#include <wrl/client.h>

struct LineVertex {
    Vector4 position;
    Vector4 color;
};

class LineRenderer {
public:
    LineRenderer();
    ~LineRenderer();

    void Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon);
    void BeginFrame();
    void DrawLine(const Vector3& start, const Vector3& end, const Vector4& color = {1.0f, 1.0f, 1.0f, 1.0f});
    void EndFrame(const Matrix4x4& viewProjectionMatrix);

private:
    void CreateVertexBuffer();
    void CreateConstantBuffer();

    DirectXCommon* dxCommon_ = nullptr;
    SpriteCommon* spriteCommon_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
    Microsoft::WRL::ComPtr<ID3D12Resource> constantBuffer_;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
    
    std::vector<LineVertex> vertices_;
    LineVertex* mappedData_ = nullptr;
    Matrix4x4* constantData_ = nullptr;
    
    static const size_t kMaxLines = 1000;
};