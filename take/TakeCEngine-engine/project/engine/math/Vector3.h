#pragma once
#include <cmath>
#include <algorithm>
#include <json.hpp>

/// <summary>
/// 3次元ベクトル
/// </summary>
struct Vector3 final {
	float x;
	float y;
	float z;

	Vector3 operator+=(const Vector3& v);
	Vector3 operator-=(const Vector3& v);
	Vector3 operator*=(float s);
	Vector3 operator/=(float s);
	bool operator==(const Vector3& v) const;

	float Dot(const Vector3& other) const;

	Vector3 Cross(const Vector3& other) const;

	float Length() const;

	Vector3 Normalize() const;
};

Vector3 operator+(const Vector3& v1, const Vector3& v2);
Vector3 operator+(const Vector3& v);
Vector3 operator-(const Vector3& v1, const Vector3& v2);
Vector3 operator-(const Vector3& v);
Vector3 operator*(float s, const Vector3& v);
Vector3 operator*(const Vector3& v, float s);
Vector3 operator*(const Vector3& v1, const Vector3& v2);
Vector3 operator/(float s, const Vector3& v);
Vector3 operator/(const Vector3& v, float s);

using json = nlohmann::json;

void to_json(nlohmann::json& j, const Vector3& v);

void from_json(const nlohmann::json& j, Vector3& v);
