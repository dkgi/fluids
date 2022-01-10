#include <cmath>
#include <stdlib.h>

#include "simulation.h"

Simulation::Simulation(int N) : N(N) {
    data = (float*)malloc(N * N * N * sizeof(float));
    for (int i = 0; i < N * N * N; i++) {
        data[i] = 1.0f;
    }
}

Simulation::~Simulation() {
    if (data != nullptr) {
        free(data);
    }
}

void Simulation::update(float time) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < N; k++) {
                int index = (i * N * N) + (j * N) + k;
                data[index] = .5f * cos(time);
            }
        }
    }
}