#pragma once

#include <iostream>

enum class Axis { X, Y, Z };

struct Matrix4f {
    Matrix4f() {}
    Matrix4f(const float data[4][4]);

    static Matrix4f identity();
    static Matrix4f translation(float x, float y, float z);
    static Matrix4f rotation(float angle, Axis axis);
    static Matrix4f perspective(int width, int height, float near, float far, float fov);

    Matrix4f multiply(const Matrix4f& other) const;

    float data[4][4] = {};
};

std::ostream& operator<<(std::ostream& out, const Matrix4f& matrix);