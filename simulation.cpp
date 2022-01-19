#include <cmath>
#include <stdlib.h>

#include "simulation.h"

Simulation::Simulation(int N) : N(N) {
    data.resize(N);
    for (int i = 0; i < N; i++) {
        data[i].resize(N);
        for (int j = 0; j < N; j++) {
            data[i][j] = std::vector(N, Vector{0.2f, 1.0f, 0.3f});
        }
    }
}

void Simulation::update(float time) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < N; k++) {
                data[i][j][k].y = .5f * cos(time);
            }
        }
    }
}