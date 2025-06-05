#pragma once
#include <cmath>

struct Vector3
{
    float x;
    float y;
    float z;
    
    Vector3() : x(0), y(0), z(0) {}
    Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    
    float Length() const {
        return std::sqrt(x * x + y * y + z * z);
    }
    
    Vector3 Normalize() const {
        float length = Length();
        if (length != 0) {
            return Vector3(x / length, y / length, z / length);
        }
        return *this;
    }
    
    static Vector3 Lerp(const Vector3& a, const Vector3& b, float t) {
        return Vector3(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t
        );
    }
    
    // 演算子オーバーロード
    Vector3 operator+(const Vector3& other) const {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }
    
    Vector3 operator-(const Vector3& other) const {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }
    
    Vector3 operator*(float scalar) const {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }
    
    Vector3 operator/(float scalar) const {
        return Vector3(x / scalar, y / scalar, z / scalar);
    }
    
    // エイリアスメソッド
    Vector3 Normalized() const {
        return Normalize();
    }
};