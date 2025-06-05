#include "AnimatedObject3d.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "SRVManager.h"
#include <cassert>
#include <algorithm>

AnimatedObject3d::AnimatedObject3d() {
}

AnimatedObject3d::~AnimatedObject3d() {
}

void AnimatedObject3d::Initialize() {
    CreateConstantBuffers();
}

void AnimatedObject3d::Initialize(DirectXCommon* dxCommon, SpriteCommon* spriteCommon) {
    Object3d::Initialize(dxCommon, spriteCommon);
    CreateConstantBuffers();
}

void AnimatedObject3d::CreateConstantBuffers() {
    if (!dxCommon_) return;
    
    auto device = dxCommon_->GetDevice();
    
    D3D12_HEAP_PROPERTIES heapProp{};
    heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
    
    D3D12_RESOURCE_DESC resourceDesc{};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = (sizeof(BoneConstBufferData) + 0xff) & ~0xff;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    
    HRESULT hr = device->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&boneConstBuffer)
    );
    assert(SUCCEEDED(hr));
}

void AnimatedObject3d::Update(float deltaTime) {
    if (fbxModel) {
        fbxModel->Update(deltaTime);
        UpdateConstantBuffers();
    }
}

void AnimatedObject3d::Update() {
    // 親クラスのUpdateを呼び出して変換行列を更新
    Object3d::Update();
    
    // アニメーションの更新（デルタタイムは60FPSと仮定）
    if (fbxModel) {
        fbxModel->Update(1.0f / 60.0f);
        UpdateConstantBuffers();
    }
}

void AnimatedObject3d::UpdateConstantBuffers() {
    if (!fbxModel) return;
    
    BoneConstBufferData* boneData = nullptr;
    boneConstBuffer->Map(0, nullptr, reinterpret_cast<void**>(&boneData));
    
    const auto& boneMatrices = fbxModel->GetBoneMatrices();
    size_t boneCount = (std::min)(boneMatrices.size(), static_cast<size_t>(128));
    
    // デバッグ: ボーン行列の状態を確認
    static int updateCount = 0;
    if (updateCount++ % 60 == 0) {  // 1秒ごとに出力
        OutputDebugStringA(("AnimatedObject3d::UpdateConstantBuffers - Bone count: " + 
                          std::to_string(boneCount) + "\n").c_str());
        
        // 最初の脚のボーン行列を確認
        if (boneCount > 1) {
            const Matrix4x4& legBone = boneMatrices[1];
            OutputDebugStringA(("AnimatedObject3d::UpdateConstantBuffers - Leg0 bone matrix [3][0]: " + 
                              std::to_string(legBone.m[3][0]) + " [3][1]: " + 
                              std::to_string(legBone.m[3][1]) + " [3][2]: " + 
                              std::to_string(legBone.m[3][2]) + "\n").c_str());
        }
    }
    
    for (size_t i = 0; i < boneCount; i++) {
        boneData->bones[i] = boneMatrices[i];
    }
    
    for (size_t i = boneCount; i < 128; i++) {
        boneData->bones[i] = Matrix4x4::MakeIdentity();
    }
    
    boneConstBuffer->Unmap(0, nullptr);
}

void AnimatedObject3d::Draw() {
    if (!fbxModel) {
        Object3d::Draw();
        return;
    }
    
    if (!dxCommon_ || !spriteCommon_) return;
    
    OutputDebugStringA("AnimatedObject3d::Draw - Starting draw\n");
    auto commandList = dxCommon_->GetCommandList();
    
    // 親クラスのUpdateを呼び出して変換行列を更新
    Object3d::Update();
    
    // マテリアル定数バッファの更新（デフォルト値）
    Material* materialData = nullptr;
    materialConstBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
    materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    materialData->enableLighting = 1;
    materialData->uvTransform = Matrix4x4::MakeIdentity();
    materialConstBuffer_->Unmap(0, nullptr);
    
    // ライト定数バッファの更新
    DirectionalLight* lightData = nullptr;
    directionalLightConstBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&lightData));
    lightData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    lightData->direction = Vector3(0.0f, -1.0f, 0.0f);  // 下向きのライト
    lightData->intensity = 1.0f;
    directionalLightConstBuffer_->Unmap(0, nullptr);
    
    // 共通描画設定（RootSignatureとPipelineStateの設定）はAnimatedRenderingPipelineで行う
    
    commandList->SetGraphicsRootConstantBufferView(0, materialConstBuffer_->GetGPUVirtualAddress());
    commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixConstBuffer_->GetGPUVirtualAddress());
    commandList->SetGraphicsRootConstantBufferView(3, directionalLightConstBuffer_->GetGPUVirtualAddress());
    commandList->SetGraphicsRootConstantBufferView(4, boneConstBuffer->GetGPUVirtualAddress());
    
    // プリミティブトポロジーを設定
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    const auto& meshes = fbxModel->GetMeshes();
    const auto& materials = fbxModel->GetMaterials();
    
    OutputDebugStringA(("AnimatedObject3d::Draw - Drawing " + std::to_string(meshes.size()) + " meshes\n").c_str());
    
    for (const auto& mesh : meshes) {
        if (mesh.materialIndex < materials.size()) {
            const auto& material = materials[mesh.materialIndex];
            
            // テクスチャの設定（既にロード済みのテクスチャを使用）
            if (!material.diffuseTexture.empty() && TextureManager::GetInstance()->IsTextureExists(material.diffuseTexture)) {
                D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = TextureManager::GetInstance()->GetSrvHandleGPU(material.diffuseTexture);
                commandList->SetGraphicsRootDescriptorTable(2, srvHandle);
            } else {
                // デフォルトテクスチャを使用（既にロード済みと仮定）
                D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = TextureManager::GetInstance()->GetSrvHandleGPU("Resources/white.png");
                commandList->SetGraphicsRootDescriptorTable(2, srvHandle);
            }
        } else {
            // デフォルトテクスチャを使用（既にロード済みと仮定）
            D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = TextureManager::GetInstance()->GetSrvHandleGPU("Resources/white.png");
            commandList->SetGraphicsRootDescriptorTable(2, srvHandle);
        }
        
        OutputDebugStringA(("AnimatedObject3d::Draw - Mesh vertices: " + std::to_string(mesh.vertices.size()) + ", indices: " + std::to_string(mesh.indices.size()) + "\n").c_str());
        
        commandList->IASetVertexBuffers(0, 1, &mesh.vertexBufferView);
        commandList->IASetIndexBuffer(&mesh.indexBufferView);
        commandList->DrawIndexedInstanced(static_cast<UINT>(mesh.indices.size()), 1, 0, 0, 0);
    }
}

void AnimatedObject3d::PlayAnimation(const std::string& animationName, bool loop) {
    if (fbxModel) {
        fbxModel->SetAnimation(animationName);
        fbxModel->PlayAnimation(loop);
    }
}

void AnimatedObject3d::StopAnimation() {
    if (fbxModel) {
        fbxModel->StopAnimation();
    }
}

bool AnimatedObject3d::IsAnimationPlaying() const {
    return fbxModel ? fbxModel->IsAnimationPlaying() : false;
}