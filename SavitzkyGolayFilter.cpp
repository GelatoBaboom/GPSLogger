#include "SavitzkyGolayFilter.h"

SavitzkyGolayFilter::SavitzkyGolayFilter() {
    for (int i = 0; i < windowSize; i++) {
        buffer[i] = 0.0f;
    }
    index = 0;
    bufferFilled = false;
}

float SavitzkyGolayFilter::update(float newValue) {
    buffer[index] = newValue;
    index = (index + 1) % windowSize;

    if (!bufferFilled && index == 0) {
        bufferFilled = true;
    }

    if (!bufferFilled) {
        return newValue; // aún no hay suficientes datos
    }

    // Coeficientes precalculados para ventana 5, orden 2: [-3, 12, 17, 12, -3] / 35
    const float coeffs[windowSize] = {-3, 12, 17, 12, -3};
    const float normalizer = 35.0f;

    float result = 0.0f;
    for (int i = 0; i < windowSize; i++) {
        int bufferIndex = (index + i) % windowSize;
        result += coeffs[i] * buffer[bufferIndex];
    }

    return result / normalizer;
}
