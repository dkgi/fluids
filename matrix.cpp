#include "matrix.h"

#include <cmath>

Matrix4f::Matrix4f(const float data[4][4]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            this->data[i][j] = data[i][j];
        }
    }
}

/* static */ Matrix4f Matrix4f::identity() {
    float data[4][4] = {
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };
    return Matrix4f(data);
}

/* static */ Matrix4f Matrix4f::translation(float x, float y, float z) {
    Matrix4f result = Matrix4f::identity();
    result.data[0][3] = x;
    result.data[1][3] = y;
    result.data[2][3] = z;
    return result;
}

/* static */ Matrix4f Matrix4f::rotation(float angle, Axis axis) {
    Matrix4f result = Matrix4f::identity();
    switch(axis) {
        case Axis::X:
            result.data[1][1] = cos(angle);
            result.data[1][2] = -sin(angle);
            result.data[2][1] = sin(angle);
            result.data[2][2] = cos(angle);
            break;
        case Axis::Y:
            result.data[0][0] = cos(angle);
            result.data[0][2] = -sin(angle);
            result.data[2][0] = sin(angle);
            result.data[2][2] = cos(angle);
            break;
        case Axis::Z:
            result.data[0][0] = cos(angle);
            result.data[0][1] = -sin(angle);
            result.data[1][0] = sin(angle);
            result.data[1][1] = cos(angle);
            break;
    }
    return result;
}
    
/* static */ Matrix4f Matrix4f::perspective(
        int width,
        int height,
        float near,
        float far,
        float fov) {
    float ratio = (float)width / height;
    float range = far - near;
    float tanFov = tan(fov / 2.0f);

    Matrix4f result;
    result.data[0][0] = 1.0f / (tanFov * ratio);
    result.data[1][1] = 1.0f / tanFov;
    result.data[2][2] = (-near - far) / range;
    result.data[2][3] = 2.0f * far * near / range;
    result.data[3][2] = 1.0f;
    return result;
}

Matrix4f Matrix4f::multiply(const Matrix4f& other) const {
    Matrix4f result;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float product = 0.0f;
            for (int k = 0; k < 4; k++) {
                product += this->data[i][k] * other.data[k][j];
            }
            result.data[i][j] = product;
        }
    }
    return result;
}

std::ostream& operator<<(std::ostream& out, const Matrix4f& matrix) {
    out << "{";
    for (int i = 0; i < 4; i++) {
        out << "{";
        for (int j = 0; j < 4; j++) {
            out << matrix.data[i][j];
            if (j != 3) {
                out << ", ";
            }
        }
        if (i != 3) {
            out << "}, ";
        };
    }
    return out << "}";
}