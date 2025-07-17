// src/Engine/Graphics/Model.cpp
#include "Model.h"
#include "TextureManager.h"
#include <fstream>
#include <sstream>
#include <cassert>
#include <unordered_map>
#include <cmath>
#include <cstdio>

// tinygltf implementation
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION  
#define STB_IMAGE_WRITE_IMPLEMENTATION
// Disable warnings for external library
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100) // unreferenced formal parameter
#pragma warning(disable: 4189) // local variable is initialized but not referenced
#pragma warning(disable: 4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable: 4267) // conversion from 'size_t' to 'type', possible loss of data
#pragma warning(disable: 4996) // deprecated functions
#endif
#include "../../externals/tinygltf/tiny_gltf.h"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

Model::Model() : dxCommon_(nullptr) {}

Model::~Model() {
	// 頂点リソースの解放（Unmapは不要 - 頂点データは永続的にマップされていない）
	if (vertexResource_) {
		vertexResource_.Reset();
	}
}

void Model::Initialize(DirectXCommon* dxCommon) {
	assert(dxCommon);
	dxCommon_ = dxCommon;
}

void Model::LoadFromObj(const std::string& directoryPath, const std::string& filename) {
	// モデルデータの読み込み
	modelData_ = LoadObjFile(directoryPath, filename);

	// モデルデータを最適化（UV球などの表示品質向上のため）
	// ファイル名も渡すように修正
	// OptimizeTriangles(modelData_, filename);  // パフォーマンス向上のため一時的に無効化

	// テクスチャの読み込み
	if (!modelData_.material.textureFilePath.empty()) {
		// テクスチャパスをログに出力
		OutputDebugStringA(("Model: Texture path from MTL: " + modelData_.material.textureFilePath + "\n").c_str());

		// テクスチャが存在するかチェック
		DWORD fileAttributes = GetFileAttributesA(modelData_.material.textureFilePath.c_str());
		if (fileAttributes != INVALID_FILE_ATTRIBUTES) {
			// テクスチャが存在する場合のみ読み込み
			TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath);
			OutputDebugStringA(("Model: Texture loaded - " + modelData_.material.textureFilePath + "\n").c_str());
		}
		else {
			// テクスチャが見つからない場合、別の場所を探す
			OutputDebugStringA(("WARNING: Texture file not found at: " + modelData_.material.textureFilePath + "\n").c_str());

			// ファイル名のみを抽出
			std::string filenameOnly = modelData_.material.textureFilePath;
			size_t lastSlash = filenameOnly.find_last_of("/\\");
			if (lastSlash != std::string::npos) {
				filenameOnly = filenameOnly.substr(lastSlash + 1);
			}

			// 複数の可能性のある場所を探索
			std::vector<std::string> possiblePaths = {
				"Resources/textures/" + filenameOnly,
				directoryPath + "/" + filenameOnly,
				"Resources/" + filenameOnly,
				"Resources/models/" + filenameOnly
			};

			bool found = false;
			for (const auto& path : possiblePaths) {
				OutputDebugStringA(("Model: Trying alternative path: " + path + "\n").c_str());
				if (GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES) {
					// 見つかった場合はパスを更新して読み込み
					modelData_.material.textureFilePath = path;
					TextureManager::GetInstance()->LoadTexture(path);
					OutputDebugStringA(("Model: Texture found and loaded from: " + path + "\n").c_str());
					found = true;
					break;
				}
			}

			if (!found) {
				// どこにも見つからない場合
				OutputDebugStringA("WARNING: Texture file not found in any location. Clearing texture path.\n");
				modelData_.material.textureFilePath = ""; // パスをクリア
			}
		}
	}
	else {
		OutputDebugStringA("Model: No texture specified in MTL file\n");
	}

	// 頂点バッファの作成
	vertexResource_ = dxCommon_->CreateBufferResource(sizeof(VertexData) * modelData_.vertices.size());

	// 頂点バッファビューの設定
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 頂点データの書き込み
	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
	vertexResource_->Unmap(0, nullptr);

	// デバッグ情報
	OutputDebugStringA(("Model: Loaded " + std::to_string(modelData_.vertices.size()) + " vertices from " + filename + "\n").c_str());
}

// 頂点バッファの作成（継承クラス用）
void Model::CreateVertexBuffer() {
	assert(dxCommon_);

	if (modelData_.vertices.empty()) {
		OutputDebugStringA("Model::CreateVertexBuffer - Warning: No vertices to create buffer\n");
		return;
	}

	// 頂点バッファの作成
	vertexResource_ = dxCommon_->CreateBufferResource(sizeof(VertexData) * modelData_.vertices.size());

	// 頂点バッファビューの設定
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 頂点データの書き込み
	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());
	vertexResource_->Unmap(0, nullptr);

	OutputDebugStringA(("Model::CreateVertexBuffer - Created buffer for " +
		std::to_string(modelData_.vertices.size()) + " vertices\n").c_str());
}

// UV球などの表示品質を向上させるためのモデルデータ最適化関数
void Model::OptimizeTriangles(ModelData& modelData, const std::string& filename) {
	// 最適化前の頂点数を保存
	size_t originalVertexCount = modelData.vertices.size();

	// 重複頂点の検出と削除のためのデータ構造
	std::vector<VertexData> optimizedVertices;
	std::vector<uint32_t> indices;
	std::unordered_map<std::string, uint32_t> vertexMap;

	// 各頂点を処理
	for (const auto& vertex : modelData.vertices) {
		// 頂点のハッシュキーを作成（類似頂点の統合を強化）
		// 精度を大幅に下げて頂点統合を促進（小数点以下2桁に制限）
		char buffer[256];
		snprintf(buffer, sizeof(buffer), "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f",
			vertex.position.x, vertex.position.y, vertex.position.z,
			vertex.normal.x, vertex.normal.y, vertex.normal.z,
			vertex.texcoord.x, vertex.texcoord.y);
		std::string key = buffer;

		// この頂点がまだ追加されていなければ追加する
		if (vertexMap.find(key) == vertexMap.end()) {
			vertexMap[key] = static_cast<uint32_t>(optimizedVertices.size());

			// 法線を正規化して品質を向上
			VertexData normalizedVertex = vertex;
			float length = std::sqrt(
				normalizedVertex.normal.x * normalizedVertex.normal.x +
				normalizedVertex.normal.y * normalizedVertex.normal.y +
				normalizedVertex.normal.z * normalizedVertex.normal.z
			);

			if (length > 0.0001f) {
				normalizedVertex.normal.x /= length;
				normalizedVertex.normal.y /= length;
				normalizedVertex.normal.z /= length;
			}

			optimizedVertices.push_back(normalizedVertex);
		}

		// インデックスを追加
		indices.push_back(vertexMap[key]);
	}

	// 法線平均化処理を追加（共有頂点の法線を平均化して滑らかにする）
	std::vector<Vector3> smoothedNormals(optimizedVertices.size(), { 0.0f, 0.0f, 0.0f });
	std::vector<int> normalCount(optimizedVertices.size(), 0);

	// 各三角形の法線を集計
	for (size_t i = 0; i < indices.size(); i += 3) {
		if (i + 2 < indices.size()) {
			// 三角形の頂点インデックス
			uint32_t idx0 = indices[i];
			uint32_t idx1 = indices[i + 1];
			uint32_t idx2 = indices[i + 2];

			// 三角形の辺ベクトル
			Vector3 edge1 = {
				optimizedVertices[idx1].position.x - optimizedVertices[idx0].position.x,
				optimizedVertices[idx1].position.y - optimizedVertices[idx0].position.y,
				optimizedVertices[idx1].position.z - optimizedVertices[idx0].position.z
			};

			Vector3 edge2 = {
				optimizedVertices[idx2].position.x - optimizedVertices[idx0].position.x,
				optimizedVertices[idx2].position.y - optimizedVertices[idx0].position.y,
				optimizedVertices[idx2].position.z - optimizedVertices[idx0].position.z
			};

			// 外積で面法線を計算
			Vector3 faceNormal = {
				edge1.y * edge2.z - edge1.z * edge2.y,
				edge1.z * edge2.x - edge1.x * edge2.z,
				edge1.x * edge2.y - edge1.y * edge2.x
			};

			// 法線の長さを計算
			float length = std::sqrt(
				faceNormal.x * faceNormal.x +
				faceNormal.y * faceNormal.y +
				faceNormal.z * faceNormal.z
			);

			// 法線を正規化
			if (length > 0.0001f) {
				faceNormal.x /= length;
				faceNormal.y /= length;
				faceNormal.z /= length;

				// 各頂点に面法線を加算
				smoothedNormals[idx0].x += faceNormal.x;
				smoothedNormals[idx0].y += faceNormal.y;
				smoothedNormals[idx0].z += faceNormal.z;
				normalCount[idx0]++;

				smoothedNormals[idx1].x += faceNormal.x;
				smoothedNormals[idx1].y += faceNormal.y;
				smoothedNormals[idx1].z += faceNormal.z;
				normalCount[idx1]++;

				smoothedNormals[idx2].x += faceNormal.x;
				smoothedNormals[idx2].y += faceNormal.y;
				smoothedNormals[idx2].z += faceNormal.z;
				normalCount[idx2]++;
			}
		}
	}

	// 法線を平均化
	for (size_t i = 0; i < optimizedVertices.size(); i++) {
		if (normalCount[i] > 0) {
			smoothedNormals[i].x /= normalCount[i];
			smoothedNormals[i].y /= normalCount[i];
			smoothedNormals[i].z /= normalCount[i];

			// 長さを正規化
			float length = std::sqrt(
				smoothedNormals[i].x * smoothedNormals[i].x +
				smoothedNormals[i].y * smoothedNormals[i].y +
				smoothedNormals[i].z * smoothedNormals[i].z
			);

			if (length > 0.0001f) {
				smoothedNormals[i].x /= length;
				smoothedNormals[i].y /= length;
				smoothedNormals[i].z /= length;
			}

			// 平滑化された法線を適用
			optimizedVertices[i].normal = smoothedNormals[i];
		}
	}

	// UV球の場合は特別な処理（UV座標から理論的な法線を計算）
	bool isSphere = (filename.find("sphere") != std::string::npos) ||
		(filename.find("ball") != std::string::npos) ||
		(filename.find("globe") != std::string::npos);

	if (isSphere) {
		for (size_t i = 0; i < optimizedVertices.size(); i++) {
			// UV座標から球面上の位置を計算
			float u = optimizedVertices[i].texcoord.x;
			float v = optimizedVertices[i].texcoord.y;

			// 球面座標
			float phi = u * 2.0f * 3.14159265f;  // 0～2π
			float theta = v * 3.14159265f;       // 0～π

			// 球面座標から理論的な法線を計算
			Vector3 theoreticalNormal;
			theoreticalNormal.x = std::sin(theta) * std::cos(phi);
			theoreticalNormal.y = std::cos(theta);
			theoreticalNormal.z = std::sin(theta) * std::sin(phi);

			// 99%理論的な法線を使用して完全な球面を強制
			Vector3 blendedNormal;
			blendedNormal.x = 0.99f * theoreticalNormal.x + 0.01f * optimizedVertices[i].normal.x;
			blendedNormal.y = 0.99f * theoreticalNormal.y + 0.01f * optimizedVertices[i].normal.y;
			blendedNormal.z = 0.99f * theoreticalNormal.z + 0.01f * optimizedVertices[i].normal.z;

			// 長さを正規化
			float length = std::sqrt(
				blendedNormal.x * blendedNormal.x +
				blendedNormal.y * blendedNormal.y +
				blendedNormal.z * blendedNormal.z
			);

			if (length > 0.0001f) {
				blendedNormal.x /= length;
				blendedNormal.y /= length;
				blendedNormal.z /= length;
			}

			// 混合された法線を適用
			optimizedVertices[i].normal = blendedNormal;
		}
	}

	// インデックスリストから頂点配列を再構築
	std::vector<VertexData> rebuiltVertices;
	for (size_t i = 0; i < indices.size(); i += 3) {
		// 三角形の頂点を追加（順序を維持）
		if (i + 2 < indices.size()) {
			rebuiltVertices.push_back(optimizedVertices[indices[i + 2]]);
			rebuiltVertices.push_back(optimizedVertices[indices[i + 1]]);
			rebuiltVertices.push_back(optimizedVertices[indices[i]]);
		}
	}

	// 最適化された頂点配列で元の配列を置き換え
	if (!rebuiltVertices.empty()) {
		modelData.vertices = rebuiltVertices;
	}
}

ModelData Model::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	ModelData modelData; // 構築するModelData
	std::vector<Vector4> positions; // 位置
	std::vector<Vector3> normals; // 法線
	std::vector<Vector2> texcoords; // テクスチャ座標
	std::string line; // ファイルから読んだ1行を格納するもの

	// ファイル読み込み
	std::ifstream file(directoryPath + "/" + filename); // fileを開く
	assert(file.is_open()); // 開けなかったら止める

	OutputDebugStringA(("Model: Loading OBJ file: " + directoryPath + "/" + filename + "\n").c_str());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier; // 先頭の識別子を読む

		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			position.x *= -1;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1 - texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1;
			normals.push_back(normal);
		}
		else if (identifier == "f") {
			VertexData triangle[3];
			// 面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				// 頂点の要素へのIndexは「位置・UV・法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/'); // 区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}
				// 要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];

				triangle[faceVertex] = { position, texcoord, normal };
			}
			// 頂点を逆順で登録することで、周り順を逆にする
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib") {
			// materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;

			// MTLファイル名をログに出力
			OutputDebugStringA(("Model: Found MTL reference: " + materialFilename + "\n").c_str());

			// 基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}
	return modelData;
}

MaterialData Model::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	MaterialData materialData; // 構築するMaterialData
	std::string line; // ファイルから読んだ1行を格納するもの

	// ファイルのフルパス
	std::string mtlPath = directoryPath + "/" + filename;
	std::ifstream file(mtlPath); // ファイルを開く

	// ファイルが開けなかった場合は警告を出力して、デフォルト値を返す
	if (!file.is_open()) {
		OutputDebugStringA(("WARNING: Failed to open MTL file - " + mtlPath + "\n").c_str());
		return materialData;
	}

	OutputDebugStringA(("Model: Successfully opened MTL file - " + mtlPath + "\n").c_str());

	while (std::getline(file, line)) {
		std::string identifier;
		std::stringstream s(line);
		s >> identifier;

		// identifierの応じた処理
		if (identifier == "map_Kd") {
			std::string token;
			std::string textureFilename;

			// オプション引数（-s, -o, -t など）を解析してファイル名を見つける
			while (s >> token) {
				if (token[0] == '-') {
					// オプション引数の場合
					if (token == "-s") {
						// スケールオプション：3つの値を読み取る
						float scaleU, scaleV, scaleW;
						s >> scaleU >> scaleV >> scaleW;
						materialData.textureScale.x = scaleU;
						materialData.textureScale.y = scaleV;
						OutputDebugStringA(("MTL Parser: Texture scale: " + std::to_string(scaleU) + ", " + std::to_string(scaleV) + "\n").c_str());
					}
					else if (token == "-o") {
						// オフセットオプション：3つの値を読み取る
						float offsetU, offsetV, offsetW;
						s >> offsetU >> offsetV >> offsetW;
						materialData.textureOffset.x = offsetU;
						materialData.textureOffset.y = offsetV;
						OutputDebugStringA(("MTL Parser: Texture offset: " + std::to_string(offsetU) + ", " + std::to_string(offsetV) + "\n").c_str());
					}
					else if (token == "-t") {
						// タービュレンスオプション：3つの値をスキップ
						std::string dummy1, dummy2, dummy3;
						s >> dummy1 >> dummy2 >> dummy3;
					}
					else if (token == "-mm") {
						// -mmオプションは2つの値をスキップ
						std::string dummy1, dummy2;
						s >> dummy1 >> dummy2;
					}
					else {
						// その他のオプションは1つの値をスキップ
						std::string dummy;
						s >> dummy;
					}
				}
				else {
					// オプションではない場合、これがテクスチャファイル名
					textureFilename = token;
					break;
				}
			}

			// テクスチャファイル名をログに出力
			OutputDebugStringA(("MTL Parser: Found texture reference: " + textureFilename + "\n").c_str());

			// 連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;

			// フルパスをログに出力
			OutputDebugStringA(("MTL Parser: Full texture path constructed: " + materialData.textureFilePath + "\n").c_str());
		}
		else if (identifier == "Ka") {
			// ambient color
			s >> materialData.ambient.x >> materialData.ambient.y >> materialData.ambient.z;
			materialData.ambient.w = 1.0f;
		}
		else if (identifier == "Kd") {
			// diffuse color
			s >> materialData.diffuse.x >> materialData.diffuse.y >> materialData.diffuse.z;
			materialData.diffuse.w = 1.0f;
		}
		else if (identifier == "Ks") {
			// specular color
			s >> materialData.specular.x >> materialData.specular.y >> materialData.specular.z;
			materialData.specular.w = 1.0f;
		}
		else if (identifier == "Ns") {
			// shininess
			s >> materialData.shininess;
		}
		else if (identifier == "d" || identifier == "Tr") {
			// transparency (d) or transparency inverted (Tr)
			if (identifier == "d") {
				s >> materialData.alpha;
			}
			else { // Tr (transparency inverted)
				float tr;
				s >> tr;
				materialData.alpha = 1.0f - tr;
			}
		}
	}

	return materialData;
}

void Model::LoadFromGLB(const std::string& filePath) {
	// GLBモデルデータの読み込み
	modelData_ = LoadGLBFile(filePath);

	OutputDebugStringA(("Model::LoadFromGLB - Loaded " + std::to_string(modelData_.vertices.size()) + " vertices\n").c_str());

	// テクスチャの読み込み（埋め込みテクスチャが保存された後）
	if (!modelData_.material.textureFilePath.empty()) {
		OutputDebugStringA(("Model::LoadFromGLB - Loading texture: " + modelData_.material.textureFilePath + "\n").c_str());

		// テクスチャが存在するかチェック
		DWORD fileAttributes = GetFileAttributesA(modelData_.material.textureFilePath.c_str());
		if (fileAttributes != INVALID_FILE_ATTRIBUTES) {
			// テクスチャが存在する場合のみ読み込み
			TextureManager::GetInstance()->LoadTexture(modelData_.material.textureFilePath);
			OutputDebugStringA(("Model::LoadFromGLB - Texture loaded: " + modelData_.material.textureFilePath + "\n").c_str());
		}
		else {
			OutputDebugStringA(("WARNING: Texture file not found: " + modelData_.material.textureFilePath + "\n").c_str());
		}
	}

	// 頂点バッファの作成
	CreateVertexBuffer();

	// 最終的なマテリアル値の確認
	char debugMsg[512];
	sprintf_s(debugMsg, "GLB Final Material - Diffuse: R=%.2f, G=%.2f, B=%.2f, A=%.2f, Texture: %s\n",
		modelData_.material.diffuse.x, modelData_.material.diffuse.y,
		modelData_.material.diffuse.z, modelData_.material.diffuse.w,
		modelData_.material.textureFilePath.empty() ? "None" : modelData_.material.textureFilePath.c_str());
	OutputDebugStringA(debugMsg);
}

ModelData Model::LoadGLBFile(const std::string& filePath) {
	ModelData result = {};

	// マテリアルのデフォルト値を設定（MaterialDataのデフォルトコンストラクタの値を使用）
	// デバッグ用にマテリアルの初期値を出力
	char initDebugMsg[256];
	sprintf_s(initDebugMsg, "GLB Initial Material - Diffuse: R=%.2f, G=%.2f, B=%.2f, A=%.2f\n",
		result.material.diffuse.x, result.material.diffuse.y,
		result.material.diffuse.z, result.material.diffuse.w);
	OutputDebugStringA(initDebugMsg);

	tinygltf::Model gltfModel;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	// GLBファイルを読み込む
	OutputDebugStringA(("Loading GLB file: " + filePath + "\n").c_str());

	// ファイルの存在確認
	DWORD fileAttributes = GetFileAttributesA(filePath.c_str());
	if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
		OutputDebugStringA(("ERROR: GLB file not found: " + filePath + "\n").c_str());
		return result;
	}

	bool success = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, filePath);

	if (!warn.empty()) {
		OutputDebugStringA(("GLTF Warning: " + warn + "\n").c_str());
	}

	if (!err.empty()) {
		OutputDebugStringA(("GLTF Error: " + err + "\n").c_str());
	}

	if (!success) {
		OutputDebugStringA("Failed to load GLB file\n");
		return result;
	}

	OutputDebugStringA(("GLB file loaded successfully. Scenes: " + std::to_string(gltfModel.scenes.size()) +
		", Meshes: " + std::to_string(gltfModel.meshes.size()) +
		", Materials: " + std::to_string(gltfModel.materials.size()) +
		", Textures: " + std::to_string(gltfModel.textures.size()) + "\n").c_str());

	// デフォルトシーンを取得
	if (gltfModel.defaultScene < 0 || gltfModel.defaultScene >= static_cast<int>(gltfModel.scenes.size())) {
		OutputDebugStringA("No default scene found in GLB file, using scene 0\n");
		// デフォルトシーンがない場合は最初のシーンを使用
		if (!gltfModel.scenes.empty()) {
			gltfModel.defaultScene = 0;
		}
		else {
			OutputDebugStringA("ERROR: No scenes found in GLB file\n");
			return result;
		}
	}

	const tinygltf::Scene& scene = gltfModel.scenes[gltfModel.defaultScene];

	// ルートノードのスケール情報を保存するためのフラグ
	bool rootTransformSaved = false;

	// ノード変換行列を計算する関数
	std::function<void(const tinygltf::Model&, int, const Matrix4x4&)> processNode;
	processNode = [&](const tinygltf::Model& model, int nodeIndex, const Matrix4x4& parentTransform) {
		if (nodeIndex < 0 || nodeIndex >= static_cast<int>(model.nodes.size())) {
			return;
		}

		const tinygltf::Node& node = model.nodes[nodeIndex];

		// ノードの変換行列を計算
		Matrix4x4 nodeTransform = MakeIdentity4x4();

		if (node.matrix.size() == 16) {
			// 行列が直接指定されている場合
			for (int i = 0; i < 16; i++) {
				nodeTransform.m[i / 4][i % 4] = static_cast<float>(node.matrix[i]);
			}
		}
		else {
			// TRS（Translation, Rotation, Scale）から行列を計算
			Vector3 translation = { 0.0f, 0.0f, 0.0f };
			if (node.translation.size() == 3) {
				translation.x = static_cast<float>(node.translation[0]);
				translation.y = static_cast<float>(node.translation[1]);
				translation.z = static_cast<float>(node.translation[2]);
			}

			// クォータニオンから回転行列を作成（とりあえずアイデンティティ）
			Matrix4x4 rotationMatrix = MakeIdentity4x4();
			if (node.rotation.size() == 4) {
				// クォータニオンの処理は省略し、アイデンティティを使用
			}

			Vector3 scale = { 1.0f, 1.0f, 1.0f };
			if (node.scale.size() == 3) {
				scale.x = static_cast<float>(node.scale[0]);
				scale.y = static_cast<float>(node.scale[1]);
				scale.z = static_cast<float>(node.scale[2]);
			}

			// 最初のメッシュを持つノードのスケール情報をルートTransformとして保存
			if (!rootTransformSaved && node.mesh >= 0 && node.mesh < static_cast<int>(model.meshes.size())) {
				result.rootTransform.scale = scale;
				result.rootTransform.translate = translation;
				// 回転はとりあえずデフォルト値のまま
				result.rootTransform.rotate = { 0.0f, 0.0f, 0.0f };
				rootTransformSaved = true;

				// デバッグ情報を出力
				char debugMsg[256];
				sprintf_s(debugMsg, "GLB Root Transform saved - Scale: X=%.3f, Y=%.3f, Z=%.3f, Translation: X=%.3f, Y=%.3f, Z=%.3f\n",
					scale.x, scale.y, scale.z, translation.x, translation.y, translation.z);
				OutputDebugStringA(debugMsg);
			}

			// TRS行列を作成
			Matrix4x4 translationMatrix = MakeTranslateMatrix(translation);
			Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
			nodeTransform = Multiply(scaleMatrix, Multiply(rotationMatrix, translationMatrix));
		}

		Matrix4x4 worldTransform = Multiply(nodeTransform, parentTransform);

		// メッシュがある場合は処理
		if (node.mesh >= 0 && node.mesh < static_cast<int>(model.meshes.size())) {
			const tinygltf::Mesh& mesh = model.meshes[node.mesh];
			OutputDebugStringA(("GLB Processing mesh: " + mesh.name + " with " + std::to_string(mesh.primitives.size()) + " primitives\n").c_str());

			int primitiveIndex = 0;
			for (const auto& primitive : mesh.primitives) {
				OutputDebugStringA(("  Processing primitive[" + std::to_string(primitiveIndex++) + "] with material index: " + std::to_string(primitive.material) + "\n").c_str());
				// 位置属性を取得
				auto positionIt = primitive.attributes.find("POSITION");
				if (positionIt == primitive.attributes.end()) {
					continue;
				}

				const tinygltf::Accessor& positionAccessor = model.accessors[positionIt->second];
				const tinygltf::BufferView& positionBufferView = model.bufferViews[positionAccessor.bufferView];
				const tinygltf::Buffer& positionBuffer = model.buffers[positionBufferView.buffer];

				// 法線属性を取得
				const tinygltf::Accessor* normalAccessor = nullptr;
				const tinygltf::BufferView* normalBufferView = nullptr;
				const tinygltf::Buffer* normalBuffer = nullptr;
				auto normalIt = primitive.attributes.find("NORMAL");
				if (normalIt != primitive.attributes.end()) {
					normalAccessor = &model.accessors[normalIt->second];
					normalBufferView = &model.bufferViews[normalAccessor->bufferView];
					normalBuffer = &model.buffers[normalBufferView->buffer];
				}

				// テクスチャ座標属性を取得
				const tinygltf::Accessor* texcoordAccessor = nullptr;
				const tinygltf::BufferView* texcoordBufferView = nullptr;
				const tinygltf::Buffer* texcoordBuffer = nullptr;
				auto texcoordIt = primitive.attributes.find("TEXCOORD_0");
				if (texcoordIt != primitive.attributes.end()) {
					texcoordAccessor = &model.accessors[texcoordIt->second];
					texcoordBufferView = &model.bufferViews[texcoordAccessor->bufferView];
					texcoordBuffer = &model.buffers[texcoordBufferView->buffer];
				}

				// インデックスを取得
				std::vector<uint32_t> indices;
				if (primitive.indices >= 0) {
					const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
					const tinygltf::BufferView& indexBufferView = model.bufferViews[indexAccessor.bufferView];
					const tinygltf::Buffer& indexBuffer = model.buffers[indexBufferView.buffer];

					const uint8_t* indexData = indexBuffer.data.data() + indexBufferView.byteOffset + indexAccessor.byteOffset;

					for (size_t i = 0; i < indexAccessor.count; ++i) {
						uint32_t index = 0;
						if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
							index = *reinterpret_cast<const uint16_t*>(indexData + i * sizeof(uint16_t));
						}
						else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
							index = *reinterpret_cast<const uint32_t*>(indexData + i * sizeof(uint32_t));
						}
						indices.push_back(index);
					}
				}
				else {
					// インデックスなしの場合、順番に頂点を使用
					for (size_t i = 0; i < positionAccessor.count; ++i) {
						indices.push_back(static_cast<uint32_t>(i));
					}
				}

				// 頂点データを構築
				const uint8_t* positionData = positionBuffer.data.data() + positionBufferView.byteOffset + positionAccessor.byteOffset;
				const uint8_t* normalData = normalBuffer ? normalBuffer->data.data() + normalBufferView->byteOffset + normalAccessor->byteOffset : nullptr;
				const uint8_t* texcoordData = texcoordBuffer ? texcoordBuffer->data.data() + texcoordBufferView->byteOffset + texcoordAccessor->byteOffset : nullptr;

				// 各プリミティブごとに別々のModelDataを作成（マルチマテリアル対応）
				ModelData primitiveModelData;
				
				// 三角形ごとに頂点を処理（逆順で追加）
				for (size_t i = 0; i < indices.size(); i += 3) {
					if (i + 2 < indices.size()) {
						for (int j = 2; j >= 0; --j) { // 逆順でループ
							uint32_t vertexIndex = indices[i + j];

							VertexData vertex = {};

							// 位置
							if (vertexIndex < positionAccessor.count) {
								const float* pos = reinterpret_cast<const float*>(positionData + vertexIndex * 3 * sizeof(float));
								vertex.position = { -pos[0], pos[1], pos[2], 1.0f }; // X軸反転
							}

							// 法線
							if (normalData && vertexIndex < normalAccessor->count) {
								const float* norm = reinterpret_cast<const float*>(normalData + vertexIndex * 3 * sizeof(float));
								vertex.normal = { -norm[0], norm[1], norm[2] }; // X軸反転
							}
							else {
								vertex.normal = { 0.0f, 1.0f, 0.0f }; // デフォルト法線
							}

							// テクスチャ座標
							if (texcoordData && vertexIndex < texcoordAccessor->count) {
								const float* tex = reinterpret_cast<const float*>(texcoordData + vertexIndex * 2 * sizeof(float));
								vertex.texcoord = { tex[0], 1.0f - tex[1] }; // Y軸反転
							}
							else {
								vertex.texcoord = { 0.0f, 0.0f }; // デフォルトUV
							}

							primitiveModelData.vertices.push_back(vertex);
						}
					}
				}

				// マテリアル処理（現在のプリミティブのマテリアルを適用）
				if (primitive.material >= 0 && primitive.material < static_cast<int>(model.materials.size())) {
					// 各プリミティブのマテリアルを処理する
					const tinygltf::Material& material = model.materials[primitive.material];

					// ベースカラーテクスチャ
					if (material.pbrMetallicRoughness.baseColorTexture.index >= 0) {
						int textureIndex = material.pbrMetallicRoughness.baseColorTexture.index;

						auto extensionIt = material.pbrMetallicRoughness.baseColorTexture.extensions.find("KHR_texture_transform");
						if (extensionIt != material.pbrMetallicRoughness.baseColorTexture.extensions.end()) {
							const tinygltf::Value& transform = extensionIt->second;

							// Scaleの読み取り
							if (transform.Has("scale") && transform.Get("scale").IsArray()) {
								const tinygltf::Value::Array& scaleArray = transform.Get("scale").Get<tinygltf::Value::Array>();
								if (scaleArray.size() >= 2) {
									primitiveModelData.material.textureScale.x = static_cast<float>(scaleArray[0].Get<double>());
									primitiveModelData.material.textureScale.y = static_cast<float>(scaleArray[1].Get<double>());
								}
							}

							// Offsetの読み取り
							if (transform.Has("offset") && transform.Get("offset").IsArray()) {
								const tinygltf::Value::Array& offsetArray = transform.Get("offset").Get<tinygltf::Value::Array>();
								if (offsetArray.size() >= 2) {
									primitiveModelData.material.textureOffset.x = static_cast<float>(offsetArray[0].Get<double>());
									primitiveModelData.material.textureOffset.y = static_cast<float>(offsetArray[1].Get<double>());
								}
							}

							// デバッグ: テクスチャ変換情報を出力
							char debugMsg[256];
							sprintf_s(debugMsg, "GLB KHR_texture_transform - Scale: X=%.3f, Y=%.3f, Offset: X=%.3f, Y=%.3f\n",
								primitiveModelData.material.textureScale.x, primitiveModelData.material.textureScale.y,
								primitiveModelData.material.textureOffset.x, primitiveModelData.material.textureOffset.y);
							OutputDebugStringA(debugMsg);
						}
						if (textureIndex < static_cast<int>(model.textures.size())) {
							const tinygltf::Texture& texture = model.textures[textureIndex];
							if (texture.source >= 0 && texture.source < static_cast<int>(model.images.size())) {
								const tinygltf::Image& image = model.images[texture.source];

								// 埋め込みテクスチャを保存
								if (!image.image.empty()) {
									// 画像フォーマットに基づいて拡張子を決定
									std::string extension = ".png"; // デフォルト
									if (image.mimeType == "image/jpeg" || image.mimeType == "image/jpg") {
										extension = ".jpg";
									}
									else if (image.mimeType == "image/png") {
										extension = ".png";
									}

									std::string textureFilename = "Resources/textures/glb_embedded_" + std::to_string(texture.source) + "_" + std::to_string(std::hash<std::string>{}(filePath)) + extension;

									// ファイルが既に存在するか確認
									DWORD fileAttributes = GetFileAttributesA(textureFilename.c_str());
									if (fileAttributes != INVALID_FILE_ATTRIBUTES) {
										// 既に存在する場合は再利用
										primitiveModelData.material.textureFilePath = textureFilename;
										OutputDebugStringA(("GLB: Reusing existing texture: " + textureFilename + "\n").c_str());
									}
									else {
										// ディレクトリを作成
										CreateDirectoryA("Resources", NULL);
										CreateDirectoryA("Resources/textures", NULL);

										// tinygltfはimage.imageにデコード済みのピクセルデータを格納している
										// image.uriまたはimage.bufferViewがある場合は元のエンコード済みデータを使用
										if (image.uri.empty() && image.bufferView < 0) {
											// デコード済みデータしかない場合はPNGとして保存
											extension = ".png";
											textureFilename = "Resources/textures/glb_embedded_" + std::to_string(texture.source) + "_" + std::to_string(std::hash<std::string>{}(filePath)) + extension;

											// stb_image_writeを使用してPNGとして保存
											int width = image.width;
											int height = image.height;
											int comp = image.component;
											if (stbi_write_png(textureFilename.c_str(), width, height, comp, image.image.data(), width * comp)) {
												primitiveModelData.material.textureFilePath = textureFilename;
												OutputDebugStringA(("GLB: Saved embedded texture as PNG: " + textureFilename + "\n").c_str());
											}
											else {
												OutputDebugStringA(("GLB: Failed to save texture as PNG: " + textureFilename + "\n").c_str());
											}
										}
										else if (image.bufferView >= 0) {
											// BufferViewから元のエンコード済みデータを取得
											const tinygltf::BufferView& bufferView = model.bufferViews[image.bufferView];
											const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

											std::ofstream texFile(textureFilename, std::ios::binary);
											if (texFile.is_open()) {
												texFile.write(reinterpret_cast<const char*>(buffer.data.data() + bufferView.byteOffset), bufferView.byteLength);
												texFile.close();
												primitiveModelData.material.textureFilePath = textureFilename;
												OutputDebugStringA(("GLB: Saved embedded texture from buffer: " + textureFilename + "\n").c_str());
											}
										}
									}
								}
							}
						}
					}

					// PBRマテリアルデータの設定
					primitiveModelData.material.isPBR = true;  // PBRレンダリングを有効化
					
					// ベースカラー係数の設定
					if (material.pbrMetallicRoughness.baseColorFactor.size() >= 3) {
						primitiveModelData.material.baseColorFactor.x = static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[0]);
						primitiveModelData.material.baseColorFactor.y = static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[1]);
						primitiveModelData.material.baseColorFactor.z = static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[2]);
						primitiveModelData.material.baseColorFactor.w = material.pbrMetallicRoughness.baseColorFactor.size() >= 4 ?
							static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[3]) : 1.0f;
						
						// 従来のdiffuseにも反映（後方互換性）
						primitiveModelData.material.diffuse = primitiveModelData.material.baseColorFactor;
					}
					else {
						primitiveModelData.material.baseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
						primitiveModelData.material.diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
						OutputDebugStringA("GLB: No baseColorFactor specified, using white default\n");
					}
					
					// PBRプロパティの設定
					primitiveModelData.material.metallicFactor = static_cast<float>(material.pbrMetallicRoughness.metallicFactor);
					primitiveModelData.material.roughnessFactor = static_cast<float>(material.pbrMetallicRoughness.roughnessFactor);
					primitiveModelData.material.emissiveFactor.x = material.emissiveFactor.size() > 0 ? static_cast<float>(material.emissiveFactor[0]) : 0.0f;
					primitiveModelData.material.emissiveFactor.y = material.emissiveFactor.size() > 1 ? static_cast<float>(material.emissiveFactor[1]) : 0.0f;
					primitiveModelData.material.emissiveFactor.z = material.emissiveFactor.size() > 2 ? static_cast<float>(material.emissiveFactor[2]) : 0.0f;
					
					// アルファモードとカットオフ
					if (material.alphaMode == "MASK") {
						primitiveModelData.material.alphaMode = "MASK";
						primitiveModelData.material.alphaCutoff = static_cast<float>(material.alphaCutoff);
					} else if (material.alphaMode == "BLEND") {
						primitiveModelData.material.alphaMode = "BLEND";
					} else {
						primitiveModelData.material.alphaMode = "OPAQUE";
					}
					
					primitiveModelData.material.doubleSided = material.doubleSided;

					// デバッグ: PBRマテリアルデータを出力
					char debugMsg[512];
					sprintf_s(debugMsg, "GLB PBR Material[%d] - %s:\n"
						"  baseColorFactor: R=%.3f, G=%.3f, B=%.3f, A=%.3f\n"
						"  metallicFactor: %.3f, roughnessFactor: %.3f\n"
						"  emissiveFactor: R=%.3f, G=%.3f, B=%.3f\n"
						"  alphaMode: %s, doubleSided: %s\n",
						primitive.material,
						material.name.empty() ? "Unnamed" : material.name.c_str(),
						primitiveModelData.material.baseColorFactor.x, primitiveModelData.material.baseColorFactor.y,
						primitiveModelData.material.baseColorFactor.z, primitiveModelData.material.baseColorFactor.w,
						primitiveModelData.material.metallicFactor, primitiveModelData.material.roughnessFactor,
						primitiveModelData.material.emissiveFactor.x, primitiveModelData.material.emissiveFactor.y, primitiveModelData.material.emissiveFactor.z,
						primitiveModelData.material.alphaMode.c_str(), primitiveModelData.material.doubleSided ? "true" : "false");
					OutputDebugStringA(debugMsg);
				}
				else {
					// マテリアルが指定されていない場合のデフォルト値
					// PBRマテリアルのデフォルト値を設定
					primitiveModelData.material.isPBR = true;
					primitiveModelData.material.baseColorFactor = { 0.8f, 0.8f, 0.8f, 1.0f };
					primitiveModelData.material.diffuse = primitiveModelData.material.baseColorFactor;
					primitiveModelData.material.metallicFactor = 0.0f;
					primitiveModelData.material.roughnessFactor = 0.9f;
					primitiveModelData.material.emissiveFactor = { 0.0f, 0.0f, 0.0f };
					primitiveModelData.material.alphaMode = "OPAQUE";
					primitiveModelData.material.doubleSided = false;
					OutputDebugStringA("GLB: No material specified for this primitive, using default PBR material\n");
				}
				
				// 各プリミティブのModelDataを結果に追加
				primitiveModelData.rootTransform = result.rootTransform;
				if (!primitiveModelData.vertices.empty()) {
					// 最初のプリミティブは既存のresultに統合
					if (result.vertices.empty()) {
						result = primitiveModelData;
					} else {
						// 2番目以降のプリミティブは頂点データを統合（一時的な解決策）
						result.vertices.insert(result.vertices.end(), 
							primitiveModelData.vertices.begin(), 
							primitiveModelData.vertices.end());
					}
				}
			}
		}

		// 子ノードを再帰的に処理
		for (int childIndex : node.children) {
			processNode(model, childIndex, worldTransform);
		}
		};

	// シーンのルートノードから開始
	for (int nodeIndex : scene.nodes) {
		processNode(gltfModel, nodeIndex, MakeIdentity4x4());
	}

	OutputDebugStringA(("GLB processing complete. Final vertex count: " + std::to_string(result.vertices.size()) + "\n").c_str());

	// 最終的なマテリアル値の確認
	char finalDebugMsg[512];
	sprintf_s(finalDebugMsg, "GLB Return Material - Diffuse: R=%.2f, G=%.2f, B=%.2f, A=%.2f, Texture: %s\n",
		result.material.diffuse.x, result.material.diffuse.y,
		result.material.diffuse.z, result.material.diffuse.w,
		result.material.textureFilePath.empty() ? "None" : result.material.textureFilePath.c_str());
	OutputDebugStringA(finalDebugMsg);

	return result;
}

std::vector<ModelData> Model::LoadMultiMaterialGLB(const std::string& filePath) {
	std::vector<ModelData> resultList;
	
	tinygltf::Model gltfModel;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	// GLBファイルを読み込む
	OutputDebugStringA(("Loading GLB file for multi-material: " + filePath + "\n").c_str());

	// ファイルの存在確認
	DWORD fileAttributes = GetFileAttributesA(filePath.c_str());
	if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
		OutputDebugStringA(("ERROR: GLB file not found: " + filePath + "\n").c_str());
		return resultList;
	}

	bool success = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, filePath);

	if (!warn.empty()) {
		OutputDebugStringA(("GLTF Warning: " + warn + "\n").c_str());
	}

	if (!err.empty()) {
		OutputDebugStringA(("GLTF Error: " + err + "\n").c_str());
	}

	if (!success) {
		OutputDebugStringA("Failed to load GLB file\n");
		return resultList;
	}

	// デフォルトシーンを取得
	if (gltfModel.defaultScene < 0 || gltfModel.defaultScene >= static_cast<int>(gltfModel.scenes.size())) {
		OutputDebugStringA("No default scene found in GLB file, using scene 0\n");
		if (!gltfModel.scenes.empty()) {
			gltfModel.defaultScene = 0;
		}
		else {
			OutputDebugStringA("ERROR: No scenes found in GLB file\n");
			return resultList;
		}
	}

	const tinygltf::Scene& scene = gltfModel.scenes[gltfModel.defaultScene];

	// ノード変換行列を計算する関数
	std::function<void(const tinygltf::Model&, int, const Matrix4x4&)> processNode;
	processNode = [&](const tinygltf::Model& model, int nodeIndex, const Matrix4x4& parentTransform) {
		if (nodeIndex < 0 || nodeIndex >= static_cast<int>(model.nodes.size())) {
			return;
		}

		const tinygltf::Node& node = model.nodes[nodeIndex];

		// ノードの変換行列を計算
		Matrix4x4 nodeTransform = MakeIdentity4x4();
		
		// 簡易的なTRS処理（詳細は省略）
		Vector3 translation = { 0.0f, 0.0f, 0.0f };
		Vector3 scale = { 1.0f, 1.0f, 1.0f };
		
		if (node.translation.size() == 3) {
			translation.x = static_cast<float>(node.translation[0]);
			translation.y = static_cast<float>(node.translation[1]);
			translation.z = static_cast<float>(node.translation[2]);
		}
		
		if (node.scale.size() == 3) {
			scale.x = static_cast<float>(node.scale[0]);
			scale.y = static_cast<float>(node.scale[1]);
			scale.z = static_cast<float>(node.scale[2]);
		}

		Matrix4x4 worldTransform = Multiply(nodeTransform, parentTransform);

		// メッシュがある場合は各プリミティブを別々のModelDataとして処理
		if (node.mesh >= 0 && node.mesh < static_cast<int>(model.meshes.size())) {
			const tinygltf::Mesh& mesh = model.meshes[node.mesh];
			
			for (const auto& primitive : mesh.primitives) {
				ModelData primitiveModelData;
				primitiveModelData.rootTransform.scale = scale;
				primitiveModelData.rootTransform.translate = translation;
				primitiveModelData.rootTransform.rotate = { 0.0f, 0.0f, 0.0f };
				
				// 位置属性を取得
				auto positionIt = primitive.attributes.find("POSITION");
				if (positionIt == primitive.attributes.end()) {
					continue;
				}

				const tinygltf::Accessor& positionAccessor = model.accessors[positionIt->second];
				const tinygltf::BufferView& positionBufferView = model.bufferViews[positionAccessor.bufferView];
				const tinygltf::Buffer& positionBuffer = model.buffers[positionBufferView.buffer];

				// 法線属性を取得
				const tinygltf::Accessor* normalAccessor = nullptr;
				const tinygltf::BufferView* normalBufferView = nullptr;
				const tinygltf::Buffer* normalBuffer = nullptr;
				auto normalIt = primitive.attributes.find("NORMAL");
				if (normalIt != primitive.attributes.end()) {
					normalAccessor = &model.accessors[normalIt->second];
					normalBufferView = &model.bufferViews[normalAccessor->bufferView];
					normalBuffer = &model.buffers[normalBufferView->buffer];
				}

				// テクスチャ座標属性を取得
				const tinygltf::Accessor* texcoordAccessor = nullptr;
				const tinygltf::BufferView* texcoordBufferView = nullptr;
				const tinygltf::Buffer* texcoordBuffer = nullptr;
				auto texcoordIt = primitive.attributes.find("TEXCOORD_0");
				if (texcoordIt != primitive.attributes.end()) {
					texcoordAccessor = &model.accessors[texcoordIt->second];
					texcoordBufferView = &model.bufferViews[texcoordAccessor->bufferView];
					texcoordBuffer = &model.buffers[texcoordBufferView->buffer];
				}

				// インデックスを取得
				std::vector<uint32_t> indices;
				if (primitive.indices >= 0) {
					const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
					const tinygltf::BufferView& indexBufferView = model.bufferViews[indexAccessor.bufferView];
					const tinygltf::Buffer& indexBuffer = model.buffers[indexBufferView.buffer];

					const uint8_t* indexData = indexBuffer.data.data() + indexBufferView.byteOffset + indexAccessor.byteOffset;

					for (size_t i = 0; i < indexAccessor.count; ++i) {
						uint32_t index = 0;
						if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
							index = *reinterpret_cast<const uint16_t*>(indexData + i * sizeof(uint16_t));
						}
						else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
							index = *reinterpret_cast<const uint32_t*>(indexData + i * sizeof(uint32_t));
						}
						indices.push_back(index);
					}
				}
				else {
					// インデックスなしの場合、順番に頂点を使用
					for (size_t i = 0; i < positionAccessor.count; ++i) {
						indices.push_back(static_cast<uint32_t>(i));
					}
				}

				// 頂点データを構築
				const uint8_t* positionData = positionBuffer.data.data() + positionBufferView.byteOffset + positionAccessor.byteOffset;
				const uint8_t* normalData = normalBuffer ? normalBuffer->data.data() + normalBufferView->byteOffset + normalAccessor->byteOffset : nullptr;
				const uint8_t* texcoordData = texcoordBuffer ? texcoordBuffer->data.data() + texcoordBufferView->byteOffset + texcoordAccessor->byteOffset : nullptr;

				// 三角形ごとに頂点を処理（逆順で追加）
				for (size_t i = 0; i < indices.size(); i += 3) {
					if (i + 2 < indices.size()) {
						for (int j = 2; j >= 0; --j) {
							uint32_t vertexIndex = indices[i + j];

							VertexData vertex = {};

							// 位置
							if (vertexIndex < positionAccessor.count) {
								const float* pos = reinterpret_cast<const float*>(positionData + vertexIndex * 3 * sizeof(float));
								vertex.position = { -pos[0], pos[1], pos[2], 1.0f };
							}

							// 法線
							if (normalData && vertexIndex < normalAccessor->count) {
								const float* norm = reinterpret_cast<const float*>(normalData + vertexIndex * 3 * sizeof(float));
								vertex.normal = { -norm[0], norm[1], norm[2] };
							}
							else {
								vertex.normal = { 0.0f, 1.0f, 0.0f };
							}

							// テクスチャ座標
							if (texcoordData && vertexIndex < texcoordAccessor->count) {
								const float* tex = reinterpret_cast<const float*>(texcoordData + vertexIndex * 2 * sizeof(float));
								vertex.texcoord = { tex[0], 1.0f - tex[1] };
							}
							else {
								vertex.texcoord = { 0.0f, 0.0f };
							}

							primitiveModelData.vertices.push_back(vertex);
						}
					}
				}

				// マテリアル処理
				if (primitive.material >= 0 && primitive.material < static_cast<int>(model.materials.size())) {
					const tinygltf::Material& material = model.materials[primitive.material];

					// ベースカラーテクスチャの処理
					if (material.pbrMetallicRoughness.baseColorTexture.index >= 0) {
						int textureIndex = material.pbrMetallicRoughness.baseColorTexture.index;
						if (textureIndex < static_cast<int>(model.textures.size())) {
							const tinygltf::Texture& texture = model.textures[textureIndex];
							if (texture.source >= 0 && texture.source < static_cast<int>(model.images.size())) {
								const tinygltf::Image& image = model.images[texture.source];
								
								// 埋め込みテクスチャを保存
								if (!image.image.empty()) {
									std::string extension = ".png";
									if (image.mimeType == "image/jpeg" || image.mimeType == "image/jpg") {
										extension = ".jpg";
									}
									
									std::string textureFilename = "Resources/textures/glb_embedded_" + std::to_string(texture.source) + "_" + std::to_string(std::hash<std::string>{}(filePath)) + extension;
									
									// ファイルが既に存在するか確認
									DWORD fileAttributes = GetFileAttributesA(textureFilename.c_str());
									if (fileAttributes != INVALID_FILE_ATTRIBUTES) {
										primitiveModelData.material.textureFilePath = textureFilename;
										OutputDebugStringA(("GLB Multi-Material: Reusing existing texture: " + textureFilename + "\n").c_str());
									}
									else {
										// ディレクトリを作成
										CreateDirectoryA("Resources", NULL);
										CreateDirectoryA("Resources/textures", NULL);
										
										if (image.bufferView >= 0) {
											const tinygltf::BufferView& bufferView = model.bufferViews[image.bufferView];
											const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
											
											std::ofstream texFile(textureFilename, std::ios::binary);
											if (texFile.is_open()) {
												texFile.write(reinterpret_cast<const char*>(buffer.data.data() + bufferView.byteOffset), bufferView.byteLength);
												texFile.close();
												primitiveModelData.material.textureFilePath = textureFilename;
												OutputDebugStringA(("GLB Multi-Material: Saved embedded texture: " + textureFilename + "\n").c_str());
											}
										}
										else {
											// デコード済みデータをPNGとして保存
											extension = ".png";
											textureFilename = "Resources/textures/glb_embedded_" + std::to_string(texture.source) + "_" + std::to_string(std::hash<std::string>{}(filePath)) + extension;
											
											int width = image.width;
											int height = image.height;
											int comp = image.component;
											if (stbi_write_png(textureFilename.c_str(), width, height, comp, image.image.data(), width * comp)) {
												primitiveModelData.material.textureFilePath = textureFilename;
												OutputDebugStringA(("GLB Multi-Material: Saved embedded texture as PNG: " + textureFilename + "\n").c_str());
											}
										}
									}
								}
							}
						}
					}
					
					// PBRマテリアルデータの設定
					primitiveModelData.material.isPBR = true;
					
					// ベースカラー係数の設定
					if (material.pbrMetallicRoughness.baseColorFactor.size() >= 3) {
						primitiveModelData.material.baseColorFactor.x = static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[0]);
						primitiveModelData.material.baseColorFactor.y = static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[1]);
						primitiveModelData.material.baseColorFactor.z = static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[2]);
						primitiveModelData.material.baseColorFactor.w = material.pbrMetallicRoughness.baseColorFactor.size() >= 4 ?
							static_cast<float>(material.pbrMetallicRoughness.baseColorFactor[3]) : 1.0f;
						
						primitiveModelData.material.diffuse = primitiveModelData.material.baseColorFactor;
					}
					else {
						primitiveModelData.material.baseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
						primitiveModelData.material.diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
					}
					
					// PBRプロパティの設定
					primitiveModelData.material.metallicFactor = static_cast<float>(material.pbrMetallicRoughness.metallicFactor);
					primitiveModelData.material.roughnessFactor = static_cast<float>(material.pbrMetallicRoughness.roughnessFactor);
					primitiveModelData.material.emissiveFactor.x = material.emissiveFactor.size() > 0 ? static_cast<float>(material.emissiveFactor[0]) : 0.0f;
					primitiveModelData.material.emissiveFactor.y = material.emissiveFactor.size() > 1 ? static_cast<float>(material.emissiveFactor[1]) : 0.0f;
					primitiveModelData.material.emissiveFactor.z = material.emissiveFactor.size() > 2 ? static_cast<float>(material.emissiveFactor[2]) : 0.0f;
					
					// アルファモード
					if (material.alphaMode == "MASK") {
						primitiveModelData.material.alphaMode = "MASK";
						primitiveModelData.material.alphaCutoff = static_cast<float>(material.alphaCutoff);
					} else if (material.alphaMode == "BLEND") {
						primitiveModelData.material.alphaMode = "BLEND";
					} else {
						primitiveModelData.material.alphaMode = "OPAQUE";
					}
					
					primitiveModelData.material.doubleSided = material.doubleSided;

					// デバッグ: 各プリミティブのマテリアルを出力
					char debugMsg[512];
					sprintf_s(debugMsg, "GLB Multi-Material[%d] - BaseColor: R=%.3f, G=%.3f, B=%.3f, A=%.3f, Texture: %s\n",
						primitive.material,
						primitiveModelData.material.baseColorFactor.x, primitiveModelData.material.baseColorFactor.y,
						primitiveModelData.material.baseColorFactor.z, primitiveModelData.material.baseColorFactor.w,
						primitiveModelData.material.textureFilePath.empty() ? "None" : primitiveModelData.material.textureFilePath.c_str());
					OutputDebugStringA(debugMsg);
				}
				else {
					// デフォルトマテリアル
					primitiveModelData.material.isPBR = true;
					primitiveModelData.material.baseColorFactor = { 0.8f, 0.8f, 0.8f, 1.0f };
					primitiveModelData.material.diffuse = primitiveModelData.material.baseColorFactor;
					primitiveModelData.material.metallicFactor = 0.0f;
					primitiveModelData.material.roughnessFactor = 0.9f;
					primitiveModelData.material.emissiveFactor = { 0.0f, 0.0f, 0.0f };
					primitiveModelData.material.alphaMode = "OPAQUE";
					primitiveModelData.material.doubleSided = false;
				}

				// 頂点が存在する場合のみ追加
				if (!primitiveModelData.vertices.empty()) {
					resultList.push_back(primitiveModelData);
				}
			}
		}

		// 子ノードを再帰的に処理
		for (int childIndex : node.children) {
			processNode(model, childIndex, worldTransform);
		}
	};

	// シーンのルートノードから開始
	for (int nodeIndex : scene.nodes) {
		processNode(gltfModel, nodeIndex, MakeIdentity4x4());
	}

	// マルチマテリアル処理結果の詳細なデバッグ情報
	OutputDebugStringA(("GLB multi-material processing complete. Created " + std::to_string(resultList.size()) + " ModelData objects\n").c_str());
	
	for (size_t i = 0; i < resultList.size(); ++i) {
		char debugMsg[512];
		sprintf_s(debugMsg, "ModelData[%zu]: Vertices=%zu, BaseColor=(%.3f,%.3f,%.3f,%.3f), Metallic=%.3f, Roughness=%.3f, isPBR=%s\n",
			i, resultList[i].vertices.size(),
			resultList[i].material.baseColorFactor.x, resultList[i].material.baseColorFactor.y,
			resultList[i].material.baseColorFactor.z, resultList[i].material.baseColorFactor.w,
			resultList[i].material.metallicFactor, resultList[i].material.roughnessFactor,
			resultList[i].material.isPBR ? "true" : "false");
		OutputDebugStringA(debugMsg);
	}

	return resultList;
}