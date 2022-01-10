#pragma once

struct Simulation {
    Simulation(int N);
    virtual ~Simulation();

    void update(float time);

    int N;
    float *data = nullptr;
};