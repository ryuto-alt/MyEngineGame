#include "LineRenderer.h"
#include "Logger.h"

LineRenderer::LineRenderer() {
}

LineRenderer::~LineRenderer() {
    if (mappedData_) {
        vertexBuffer_->Unmap(0, nullptr);
    }
    if (constantData_) {
        constantBuffer_->Unmap(0, nullptr);
    }
}

void LineRenderer::Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon) {
    dxCommon_ = dxCommon;
    spriteCommon_ = spriteCommon;
    CreateVertexBuffer();
    CreateConstantBuffer();
}

void LineRenderer::CreateVertexBuffer() {
    // 頂点バッファの作成
    size_t bufferSize = sizeof(LineVertex) * kMaxLines * 2; // 各ラインに2つの頂点
    
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    
    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = bufferSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    
    HRESULT hr = dxCommon_->GetDevice()->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertexBuffer_)
    );
    
    if (FAILED(hr)) {
        Logger::Log("Failed to create line vertex buffer");
        return;
    }
    
    // マップ
    hr = vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedData_));
    if (FAILED(hr)) {
        Logger::Log("Failed to map line vertex buffer");
        return;
    }
    
    // 頂点バッファビューを作成
    vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<UINT>(bufferSize);
    vertexBufferView_.StrideInBytes = sizeof(LineVertex);
}

void LineRenderer::CreateConstantBuffer() {
    // 定数バッファの作成
    size_t bufferSize = sizeof(Matrix4x4);
    // 256バイト境界にアライン
    bufferSize = (bufferSize + 255) & ~255;
    
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    
    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = bufferSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    
    HRESULT hr = dxCommon_->GetDevice()->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&constantBuffer_)
    );
    
    if (FAILED(hr)) {
        Logger::Log("Failed to create line constant buffer");
        return;
    }
    
    // マップ
    hr = constantBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&constantData_));
    if (FAILED(hr)) {
        Logger::Log("Failed to map line constant buffer");
        return;
    }
}


void LineRenderer::BeginFrame() {
    vertices_.clear();
}

void LineRenderer::DrawLine(const Vector3& start, const Vector3& end, const Vector4& color) {
    if (vertices_.size() + 2 > kMaxLines * 2) {
        return; // バッファが満杯
    }
    
    LineVertex startVertex = {};
    startVertex.position = {start.x, start.y, start.z, 1.0f};
    startVertex.color = color;
    
    LineVertex endVertex = {};
    endVertex.position = {end.x, end.y, end.z, 1.0f};
    endVertex.color = color;
    
    vertices_.push_back(startVertex);
    vertices_.push_back(endVertex);
}

void LineRenderer::EndFrame(const Matrix4x4& viewProjectionMatrix) {
    if (vertices_.empty()) {
        return;
    }
    
    // 頂点データをGPUに転送
    memcpy(mappedData_, vertices_.data(), vertices_.size() * sizeof(LineVertex));
    
    // 定数データをGPUに転送
    *constantData_ = viewProjectionMatrix;
    
    // 描画コマンド
    ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
    
    // 既存のシステムを使用
    spriteCommon_->CommonDraw();
    
    // ライン描画用の設定
    commandList->SetGraphicsRootConstantBufferView(1, constantBuffer_->GetGPUVirtualAddress());
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
    
    commandList->DrawInstanced(static_cast<UINT>(vertices_.size()), 1, 0, 0);
}