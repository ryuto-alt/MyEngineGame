#pragma once
#include "3d/Object3d.h"
#include <string>
#include <vector>
#include <memory>

struct LevelData {


	struct ObjectData {
		std::string type; // オブジェクトの種類
		std::string file_name; // モデルファイル名
		Vector3 translation; // 位置
		Vector3 rotation; // 回転
		Vector3 scale; // スケール
	};

	std::vector<ObjectData> objects; // レベル内のオブジェクトのリスト
};