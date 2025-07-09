#include "PrimitiveType.h"

void to_json(nlohmann::json& j, const PrimitiveType& type) {

	switch (type) {
	case PRIMITIVE_RING:
		j = "PRIMITIVE_RING";
		break;
	case PRIMITIVE_PLANE:
		j = "PRIMITIVE_PLANE";
		break;
	case PRIMITIVE_SPHERE:
		j = "PRIMITIVE_SPHERE";
		break;
	default:
		j = "UNKNOWN";
		break;
	}
}

void from_json(const nlohmann::json& j, PrimitiveType& type) {

	std::string str = j.get<std::string>();
	if (str == "PRIMITIVE_RING") {
		type = PRIMITIVE_RING;
	} else if (str == "PRIMITIVE_PLANE") {
		type = PRIMITIVE_PLANE;
	} else if (str == "PRIMITIVE_SPHERE") {
		type = PRIMITIVE_SPHERE;
	} else {
		type = PRIMITIVE_COUNT; // 不明なタイプ
	}
}
