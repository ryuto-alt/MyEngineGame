#pragma once
#include <string>
#include <vector>
#include "../Math/Vector3.h"

class StageGrid;
class StageBuilder;

struct StageData {
    std::string version = "1.0";
    std::string name;
    int gridWidth;
    int gridHeight;
    
    struct PartData {
        std::string type;
        int x;
        int y;
        int rotation;
    };
    
    std::vector<PartData> parts;
    std::vector<Vector3> spawnPoints;
    std::vector<Vector3> itemLocations;
};

class StageSerializer {
public:
    static bool SaveStage(const std::string& filePath, StageBuilder* builder);
    
    static bool LoadStage(const std::string& filePath, StageBuilder* builder);
    
    static StageData SerializeStage(StageBuilder* builder);
    
    static bool DeserializeStage(const StageData& data, StageBuilder* builder);
    
    // JSONとの相互変換（シンプルな実装）
    static std::string StageDataToJson(const StageData& data);
    static StageData JsonToStageData(const std::string& jsonStr);
    
    // ファイルパス管理
    static std::string GetStageDirectory() { return "Resources/Stages/"; }
    static std::string GetStageFilePath(const std::string& stageName);
    
    // ステージリストの取得
    static std::vector<std::string> GetAvailableStages();

private:
    static bool ValidateStageData(const StageData& data);
};