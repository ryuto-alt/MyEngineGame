#pragma once
#include "Matrix4x4.h"

// トランスフォーメーション行列（ワールド・ビュー・プロジェクションなど）
struct TransformationMatrix {
    Matrix4x4 WVP;     // ワールド・ビュー・プロジェクション合成行列
    Matrix4x4 World;   // ワールド変換行列
};
