#pragma once
struct Vector4
{
    float x;
    float y;
    float z;
    float w;
    
    Vector4() : x(0), y(0), z(0), w(0) {}
    Vector4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
    
    static Vector4 Lerp(const Vector4& a, const Vector4& b, float t) {
        return Vector4(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t,
            a.z + (b.z - a.z) * t,
            a.w + (b.w - a.w) * t
        );
    }
};