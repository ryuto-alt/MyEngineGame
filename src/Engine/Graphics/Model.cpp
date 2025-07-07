#include "Model.h"
#include "TextureManager.h"
#include <cassert>
#include <fstream>
#include <sstream>

Model::Model() : dxCommon_(nullptr) {}

Model::~Model() {
    if (vertexResource_) {
        vertexResource_.Reset();
    }
}

void Model::Initialize(DirectXCommon* dxCommon) {
    assert(dxCommon);
    dxCommon_ = dxCommon;
}

void Model::LoadFromObj(const std::string& directoryPath, const std::string& filename) {
    OutputDebugStringA(("Model: Loading " + directoryPath + "/" + filename + " with Assimp\n").c_str());
    LoadWithAssimp(directoryPath, filename);
    CreateVertexBuffer();
}

void Model::CreateVertexBuffer() {
    assert(dxCommon_);
    
    if (modelData_.vertices.empty()) {
        OutputDebugStringA("Model::CreateVertexBuffer - Warning: No vertices to create buffer\n");
        return;
    }
    
    vertexResource_ = dxCommon_->CreateBufferResource(sizeof(VertexData) * modelData_.vertices.size());
    
    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * modelData_.vertices.size());
    vertexBufferView_.StrideInBytes = sizeof(VertexData);
    
    VertexData* vertexData = nullptr;
    vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
    std::memcpy(vertexData, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
    vertexResource_->Unmap(0, nullptr);
    
    OutputDebugStringA(("Model: Created vertex buffer with " + 
                       std::to_string(modelData_.vertices.size()) + " vertices\n").c_str());
}

void Model::LoadWithAssimp(const std::string& directoryPath, const std::string& filename) {
    std::string fullPath = directoryPath + "/" + filename;
    
    OutputDebugStringA(("Model: Loading file: " + fullPath + "\n").c_str());
    
    const aiScene* scene = assimpImporter_.ReadFile(fullPath,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenSmoothNormals |
        aiProcess_ValidateDataStructure
    );
    
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        OutputDebugStringA(("Model: Error loading file: " + std::string(assimpImporter_.GetErrorString()) + "\n").c_str());
        return;
    }
    
    OutputDebugStringA(("Model: Successfully loaded file with " + 
                       std::to_string(scene->mNumMeshes) + " meshes, " +
                       std::to_string(scene->mNumMaterials) + " materials\n").c_str());
    
    for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
        const aiMaterial* material = scene->mMaterials[i];
        if (material) {
            aiString materialName;
            if (material->Get(AI_MATKEY_NAME, materialName) == AI_SUCCESS) {
                OutputDebugStringA(("Model: Material " + std::to_string(i) + " name: " + materialName.C_Str() + "\n").c_str());
            }
            
            OutputDebugStringA(("Model: Material " + std::to_string(i) + " has " + 
                               std::to_string(material->GetTextureCount(aiTextureType_DIFFUSE)) + " diffuse textures\n").c_str());
            
            for (unsigned int j = 0; j < material->GetTextureCount(aiTextureType_DIFFUSE); j++) {
                aiString texturePath;
                if (material->GetTexture(aiTextureType_DIFFUSE, j, &texturePath) == AI_SUCCESS) {
                    OutputDebugStringA(("Model: Diffuse texture " + std::to_string(j) + ": " + texturePath.C_Str() + "\n").c_str());
                }
            }
        }
    }
    
    ProcessAssimpScene(scene, directoryPath, filename);
}

void Model::ProcessAssimpScene(const aiScene* scene, const std::string& directoryPath, const std::string& objFileName) {
    if (!scene) {
        OutputDebugStringA("Model: Invalid scene data\n");
        return;
    }
    
    if (scene->mNumMeshes > 0 && scene->mMeshes[0]) {
        ProcessAssimpMesh(scene->mMeshes[0], scene);
    } else {
        OutputDebugStringA("Model: No valid meshes found\n");
    }
    
    if (scene->mNumMaterials > 0 && scene->mMaterials[0]) {
        ProcessAssimpMaterial(scene->mMaterials[0], directoryPath, objFileName);
    } else {
        ProcessAssimpMaterial(nullptr, directoryPath, objFileName);
    }
    
    if (scene->mRootNode) {
        modelData_.rootNode.name = scene->mRootNode->mName.C_Str();
        if (modelData_.rootNode.name.empty()) {
            modelData_.rootNode.name = "RootNode";
        }
        
        aiMatrix4x4 assimpMatrix = scene->mRootNode->mTransformation;
        
        Matrix4x4 transformMatrix;
        transformMatrix.m[0][0] = assimpMatrix.a1; transformMatrix.m[0][1] = assimpMatrix.a2; transformMatrix.m[0][2] = assimpMatrix.a3; transformMatrix.m[0][3] = assimpMatrix.a4;
        transformMatrix.m[1][0] = assimpMatrix.b1; transformMatrix.m[1][1] = assimpMatrix.b2; transformMatrix.m[1][2] = assimpMatrix.b3; transformMatrix.m[1][3] = assimpMatrix.b4;
        transformMatrix.m[2][0] = assimpMatrix.c1; transformMatrix.m[2][1] = assimpMatrix.c2; transformMatrix.m[2][2] = assimpMatrix.c3; transformMatrix.m[2][3] = assimpMatrix.c4;
        transformMatrix.m[3][0] = assimpMatrix.d1; transformMatrix.m[3][1] = assimpMatrix.d2; transformMatrix.m[3][2] = assimpMatrix.d3; transformMatrix.m[3][3] = assimpMatrix.d4;
        
        Matrix4x4 coordinateConversion = MakeIdentity4x4();
        coordinateConversion.m[2][2] = -1.0f;
        
        modelData_.rootNode.localMatrix = Multiply(coordinateConversion, transformMatrix);
        
        OutputDebugStringA(("Model: Root node: " + modelData_.rootNode.name + "\n").c_str());
    } else {
        modelData_.rootNode.name = "DefaultRoot";
        modelData_.rootNode.localMatrix = MakeIdentity4x4();
        OutputDebugStringA("Model: Using default root node\n");
    }
}

void Model::ProcessAssimpMesh(const aiMesh* mesh, const aiScene* scene) {
    if (!mesh || mesh->mNumVertices == 0) {
        OutputDebugStringA("Model: Invalid mesh data\n");
        return;
    }
    
    modelData_.vertices.clear();
    modelData_.vertices.reserve(mesh->mNumFaces * 3);
    
    OutputDebugStringA(("Model: Processing mesh with " + std::to_string(mesh->mNumVertices) + " vertices and " + std::to_string(mesh->mNumFaces) + " faces\n").c_str());
    
    for (unsigned int faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++) {
        const aiFace& face = mesh->mFaces[faceIndex];
        
        if (face.mNumIndices != 3) {
            continue;
        }
        
        unsigned int indices[3] = { face.mIndices[0], face.mIndices[2], face.mIndices[1] };
        
        for (int i = 0; i < 3; i++) {
            unsigned int vertexIndex = indices[i];
            
            if (vertexIndex >= mesh->mNumVertices) {
                OutputDebugStringA(("Model: Invalid vertex index: " + std::to_string(vertexIndex) + "\n").c_str());
                continue;
            }
            
            VertexData vertex{};
            
            vertex.position = {
                mesh->mVertices[vertexIndex].x,
                mesh->mVertices[vertexIndex].y,
                -mesh->mVertices[vertexIndex].z,
                1.0f
            };
            
            if (mesh->HasNormals()) {
                vertex.normal = {
                    mesh->mNormals[vertexIndex].x,
                    mesh->mNormals[vertexIndex].y,
                    -mesh->mNormals[vertexIndex].z
                };
            } else {
                vertex.normal = {0.0f, 1.0f, 0.0f};
            }
            
            if (mesh->mTextureCoords[0] && vertexIndex < mesh->mNumVertices) {
                vertex.texcoord = {
                    mesh->mTextureCoords[0][vertexIndex].x,
                    mesh->mTextureCoords[0][vertexIndex].y
                };
            } else {
                vertex.texcoord = {0.0f, 0.0f};
            }
            
            modelData_.vertices.push_back(vertex);
        }
    }
    
    OutputDebugStringA(("Model: Created " + std::to_string(modelData_.vertices.size()) + " vertices\n").c_str());
}

void Model::ProcessAssimpMaterial(const aiMaterial* material, const std::string& directoryPath, const std::string& objFileName) {
    modelData_.material.diffuse = {1.0f, 1.0f, 1.0f, 1.0f};
    modelData_.material.ambient = {0.2f, 0.2f, 0.2f, 1.0f};
    modelData_.material.specular = {0.5f, 0.5f, 0.5f, 1.0f};
    modelData_.material.alpha = 1.0f;
    modelData_.material.textureFilePath = "";
    
    if (!material) {
        OutputDebugStringA("Model: No material data, using defaults\n");
        modelData_.material.textureFilePath = "Resources/white1x1.png";
        return;
    }
    
    OutputDebugStringA(("Model: Processing material with " + std::to_string(material->GetTextureCount(aiTextureType_DIFFUSE)) + " diffuse textures\n").c_str());
    
    aiTextureType textureTypes[] = {aiTextureType_DIFFUSE, aiTextureType_BASE_COLOR, aiTextureType_UNKNOWN};
    std::string textureFileName;
    
    for (const auto& textureType : textureTypes) {
        if (material->GetTextureCount(textureType) > 0) {
            aiString texturePath;
            if (material->GetTexture(textureType, 0, &texturePath) == AI_SUCCESS) {
                textureFileName = texturePath.C_Str();
                if (!textureFileName.empty()) {
                    OutputDebugStringA(("Model: Found texture in material: " + textureFileName + "\n").c_str());
                    break;
                }
            }
        }
    }
    
    if (!textureFileName.empty()) {
        std::string originalFileName = textureFileName;
        
        size_t lastSlash = textureFileName.find_last_of("/\\");
        std::string filenameOnly = (lastSlash != std::string::npos) ? textureFileName.substr(lastSlash + 1) : textureFileName;
        
        std::vector<std::string> possiblePaths = {
            directoryPath + "/" + textureFileName,
            directoryPath + "/" + filenameOnly,
            "Resources/" + textureFileName,
            "Resources/" + filenameOnly,
            "Resources/textures/" + filenameOnly,
            "Resources/models/" + filenameOnly,
            "Resources/Models/" + filenameOnly,
            "Resources/CG2/Resources/" + filenameOnly,
            "GitHub/CG2/Resources/" + filenameOnly,
            "CG2/Resources/" + filenameOnly
        };
        
        bool found = false;
        for (const auto& path : possiblePaths) {
            OutputDebugStringA(("Model: Checking texture path: " + path + "\n").c_str());
            if (GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES) {
                modelData_.material.textureFilePath = path;
                OutputDebugStringA(("Model: Texture found at: " + path + "\n").c_str());
                found = true;
                break;
            }
        }
        
        if (!found) {
            OutputDebugStringA(("Model: Texture not found - Original: " + originalFileName + ", Filename: " + filenameOnly + "\n").c_str());
            OutputDebugStringA("Model: Checked paths:\n");
            for (const auto& path : possiblePaths) {
                OutputDebugStringA(("  - " + path + "\n").c_str());
            }
            OutputDebugStringA("Model: Using default texture\n");
            modelData_.material.textureFilePath = "Resources/uvChecker.png";
        }
    } else {
        OutputDebugStringA("Model: No texture found in material via assimp, searching manually\n");
        std::string mtlTexture = ParseMTLFile(directoryPath, objFileName);
        
        if (!mtlTexture.empty()) {
            size_t lastSlash = mtlTexture.find_last_of("/\\");
            std::string filenameOnly = (lastSlash != std::string::npos) ? mtlTexture.substr(lastSlash + 1) : mtlTexture;
            
            std::vector<std::string> possiblePaths = {
                directoryPath + "/" + mtlTexture,
                directoryPath + "/" + filenameOnly,
                "Resources/" + filenameOnly,
                "Resources/Models/" + filenameOnly
            };
            
            bool found = false;
            for (const auto& path : possiblePaths) {
                if (GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES) {
                    modelData_.material.textureFilePath = path;
                    OutputDebugStringA(("Model: MTL texture found at: " + path + "\n").c_str());
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                OutputDebugStringA("Model: MTL texture not found, searching directory for PNG files\n");
                std::string autoTexture = FindTextureInDirectory(directoryPath);
                if (!autoTexture.empty()) {
                    modelData_.material.textureFilePath = autoTexture;
                    OutputDebugStringA(("Model: Auto-detected texture: " + autoTexture + "\n").c_str());
                } else {
                    OutputDebugStringA("Model: No PNG files found, using default\n");
                    modelData_.material.textureFilePath = "Resources/uvChecker.png";
                }
            }
        } else {
            OutputDebugStringA("Model: No MTL texture info, searching directory for PNG files\n");
            std::string autoTexture = FindTextureInDirectory(directoryPath);
            if (!autoTexture.empty()) {
                modelData_.material.textureFilePath = autoTexture;
                OutputDebugStringA(("Model: Auto-detected texture: " + autoTexture + "\n").c_str());
            } else {
                OutputDebugStringA("Model: No PNG files found, using default\n");
                modelData_.material.textureFilePath = "Resources/uvChecker.png";
            }
        }
    }
    
    aiColor3D color;
    if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
        modelData_.material.diffuse = {color.r, color.g, color.b, 1.0f};
        OutputDebugStringA(("Model: Diffuse color: " + std::to_string(color.r) + ", " + std::to_string(color.g) + ", " + std::to_string(color.b) + "\n").c_str());
    }
    
    if (material->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS) {
        modelData_.material.ambient = {color.r, color.g, color.b, 1.0f};
    }
    
    if (material->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
        modelData_.material.specular = {color.r, color.g, color.b, 1.0f};
    }
    
    float opacity;
    if (material->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS && opacity > 0.0f) {
        modelData_.material.alpha = opacity;
    }
    
    if (!modelData_.material.textureFilePath.empty()) {
        TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath);
    }
}

std::string Model::FindTextureInDirectory(const std::string& directoryPath) {
    WIN32_FIND_DATAA findData;
    std::string searchPath = directoryPath + "/*.png";
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
    
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                std::string foundTexture = directoryPath + "/" + findData.cFileName;
                OutputDebugStringA(("Model: Found PNG in directory: " + foundTexture + "\n").c_str());
                FindClose(hFind);
                return foundTexture;
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
    
    return "";
}

std::string Model::ParseMTLFile(const std::string& directoryPath, const std::string& objFileName) {
    std::string mtlFileName = objFileName;
    size_t dotPos = mtlFileName.find_last_of('.');
    if (dotPos != std::string::npos) {
        mtlFileName = mtlFileName.substr(0, dotPos) + ".mtl";
    } else {
        mtlFileName += ".mtl";
    }
    
    std::string mtlPath = directoryPath + "/" + mtlFileName;
    std::ifstream mtlFile(mtlPath);
    
    if (!mtlFile.is_open()) {
        OutputDebugStringA(("Model: Cannot open MTL file: " + mtlPath + "\n").c_str());
        return "";
    }
    
    std::string line;
    while (std::getline(mtlFile, line)) {
        if (line.find("map_Kd") == 0) {
            size_t spacePos = line.find(' ');
            if (spacePos != std::string::npos) {
                std::string texturePath = line.substr(spacePos + 1);
                while (!texturePath.empty() && (texturePath.back() == '\r' || texturePath.back() == '\n' || texturePath.back() == ' ')) {
                    texturePath.pop_back();
                }
                OutputDebugStringA(("Model: Found map_Kd in MTL: " + texturePath + "\n").c_str());
                return texturePath;
            }
        }
    }
    
    return "";
}