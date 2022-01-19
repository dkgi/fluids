#pragma once

#include <vector>

struct Vector {
    float x, y, z;
};

using Data = std::vector<std::vector<std::vector<Vector>>>;

struct Simulation {
    Simulation(int N);

    void update(float time);

    int N;
    Data data;
};