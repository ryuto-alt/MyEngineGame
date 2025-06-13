#include "StageSerializer.h"
#include "StageBuilder.h"
#include "StageGrid.h"
#include "StagePart.h"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <sstream>

bool StageSerializer::SaveStage(const std::string& filePath, StageBuilder* builder) {
    if (!builder || !builder->GetGrid()) {
        return false;
    }

    try {
        StageData data = SerializeStage(builder);
        std::string json = StageDataToJson(data);
        
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << filePath << std::endl;
            return false;
        }
        
        file << json;
        file.close();
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving stage: " << e.what() << std::endl;
        return false;
    }
}

bool StageSerializer::LoadStage(const std::string& filePath, StageBuilder* builder) {
    if (!builder) {
        return false;
    }

    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for reading: " << filePath << std::endl;
            return false;
        }
        
        std::string jsonStr((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();
        
        StageData data = JsonToStageData(jsonStr);
        
        if (!ValidateStageData(data)) {
            std::cerr << "Invalid stage data in file: " << filePath << std::endl;
            return false;
        }
        
        return DeserializeStage(data, builder);
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading stage: " << e.what() << std::endl;
        return false;
    }
}

StageData StageSerializer::SerializeStage(StageBuilder* builder) {
    StageData data;
    
    if (!builder || !builder->GetGrid()) {
        return data;
    }
    
    auto grid = builder->GetGrid();
    data.name = builder->GetStageName();
    data.gridWidth = grid->GetWidth();
    data.gridHeight = grid->GetHeight();
    
    // すべてのパーツをシリアライズ
    for (int y = 0; y < grid->GetHeight(); ++y) {
        for (int x = 0; x < grid->GetWidth(); ++x) {
            auto part = grid->GetPart(x, y);
            if (part) {
                StageData::PartData partData;
                partData.type = StagePart::GetTypeName(part->GetType());
                partData.x = x;
                partData.y = y;
                partData.rotation = part->GetRotation();
                data.parts.push_back(partData);
            }
        }
    }
    
    // TODO: スポーンポイントとアイテム位置の保存
    
    return data;
}

bool StageSerializer::DeserializeStage(const StageData& data, StageBuilder* builder) {
    if (!builder) {
        return false;
    }
    
    // 新しいステージを作成
    builder->CreateNewStage(data.gridWidth, data.gridHeight);
    builder->SetStageName(data.name);
    
    // パーツを配置
    for (const auto& partData : data.parts) {
        // パーツタイプ名からenumに変換
        StagePartType type = StagePartType::FLOOR_BASIC;  // デフォルト
        for (int i = 0; i < static_cast<int>(StagePartType::MAX_TYPE); ++i) {
            StagePartType t = static_cast<StagePartType>(i);
            if (StagePart::GetTypeName(t) == partData.type) {
                type = t;
                break;
            }
        }
        
        // パーツを配置
        builder->PlacePart(partData.x, partData.y, type, partData.rotation);
    }
    
    // TODO: スポーンポイントとアイテム位置の復元
    
    return true;
}

std::string StageSerializer::StageDataToJson(const StageData& data) {
    std::stringstream json;
    
    json << "{\n";
    json << "  \"version\": \"" << data.version << "\",\n";
    json << "  \"name\": \"" << data.name << "\",\n";
    json << "  \"gridSize\": [" << data.gridWidth << ", " << data.gridHeight << "],\n";
    
    // パーツ配列
    json << "  \"parts\": [\n";
    for (size_t i = 0; i < data.parts.size(); ++i) {
        const auto& part = data.parts[i];
        json << "    {\n";
        json << "      \"type\": \"" << part.type << "\",\n";
        json << "      \"position\": [" << part.x << ", " << part.y << "],\n";
        json << "      \"rotation\": " << part.rotation << "\n";
        json << "    }";
        if (i < data.parts.size() - 1) json << ",";
        json << "\n";
    }
    json << "  ],\n";
    
    // スポーンポイント
    json << "  \"spawnPoints\": [";
    for (size_t i = 0; i < data.spawnPoints.size(); ++i) {
        const auto& spawn = data.spawnPoints[i];
        json << "[" << spawn.x << ", " << spawn.y << ", " << spawn.z << "]";
        if (i < data.spawnPoints.size() - 1) json << ", ";
    }
    json << "],\n";
    
    // アイテム位置
    json << "  \"itemLocations\": [";
    for (size_t i = 0; i < data.itemLocations.size(); ++i) {
        const auto& item = data.itemLocations[i];
        json << "[" << item.x << ", " << item.y << ", " << item.z << "]";
        if (i < data.itemLocations.size() - 1) json << ", ";
    }
    json << "]\n";
    
    json << "}";
    
    return json.str();
}

StageData StageSerializer::JsonToStageData(const std::string& jsonStr) {
    StageData data;
    
    // 簡易的なJSONパーサー（実際のプロジェクトでは適切なJSONライブラリを使用してください）
    std::stringstream ss(jsonStr);
    std::string line;
    bool inParts = false;
    
    while (std::getline(ss, line)) {
        // バージョン
        if (line.find("\"version\":") != std::string::npos) {
            size_t start = line.find("\"", line.find(":")) + 1;
            size_t end = line.find("\"", start);
            data.version = line.substr(start, end - start);
        }
        // 名前
        else if (line.find("\"name\":") != std::string::npos) {
            size_t start = line.find("\"", line.find(":")) + 1;
            size_t end = line.find("\"", start);
            data.name = line.substr(start, end - start);
        }
        // グリッドサイズ
        else if (line.find("\"gridSize\":") != std::string::npos) {
            size_t start = line.find("[") + 1;
            size_t comma = line.find(",", start);
            size_t end = line.find("]");
            data.gridWidth = std::stoi(line.substr(start, comma - start));
            data.gridHeight = std::stoi(line.substr(comma + 1, end - comma - 1));
        }
        // パーツ配列の開始
        else if (line.find("\"parts\":") != std::string::npos) {
            inParts = true;
        }
        // パーツデータ
        else if (inParts && line.find("\"type\":") != std::string::npos) {
            StageData::PartData part;
            
            // タイプ
            size_t start = line.find("\"", line.find(":")) + 1;
            size_t end = line.find("\"", start);
            part.type = line.substr(start, end - start);
            
            // 位置
            std::getline(ss, line);
            if (line.find("\"position\":") != std::string::npos) {
                start = line.find("[") + 1;
                size_t comma = line.find(",", start);
                end = line.find("]");
                part.x = std::stoi(line.substr(start, comma - start));
                part.y = std::stoi(line.substr(comma + 1, end - comma - 1));
            }
            
            // 回転
            std::getline(ss, line);
            if (line.find("\"rotation\":") != std::string::npos) {
                start = line.find(":") + 1;
                end = line.find_first_not_of(" ", start);
                part.rotation = std::stoi(line.substr(end));
            }
            
            data.parts.push_back(part);
        }
    }
    
    return data;
}

std::string StageSerializer::GetStageFilePath(const std::string& stageName) {
    return GetStageDirectory() + stageName + ".json";
}

std::vector<std::string> StageSerializer::GetAvailableStages() {
    std::vector<std::string> stages;
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(GetStageDirectory())) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                stages.push_back(entry.path().stem().string());
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error reading stage directory: " << e.what() << std::endl;
    }
    
    return stages;
}

bool StageSerializer::ValidateStageData(const StageData& data) {
    // バージョンチェック
    if (data.version != "1.0") {
        std::cerr << "Unsupported stage version: " << data.version << std::endl;
        return false;
    }
    
    // グリッドサイズの妥当性チェック
    if (data.gridWidth <= 0 || data.gridHeight <= 0 || 
        data.gridWidth > 200 || data.gridHeight > 200) {
        std::cerr << "Invalid grid size: " << data.gridWidth << "x" << data.gridHeight << std::endl;
        return false;
    }
    
    // パーツ位置の妥当性チェック
    for (const auto& part : data.parts) {
        if (part.x < 0 || part.x >= data.gridWidth ||
            part.y < 0 || part.y >= data.gridHeight) {
            std::cerr << "Part position out of bounds: " << part.x << ", " << part.y << std::endl;
            return false;
        }
        
        if (part.rotation % 90 != 0 || part.rotation < 0 || part.rotation >= 360) {
            std::cerr << "Invalid part rotation: " << part.rotation << std::endl;
            return false;
        }
    }
    
    return true;
}