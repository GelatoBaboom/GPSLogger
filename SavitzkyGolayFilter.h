#ifndef SAVITZKYGOLAYFILTER_H
#define SAVITZKYGOLAYFILTER_H

class SavitzkyGolayFilter {
public:
    SavitzkyGolayFilter();

    // Llamás a esta función con cada nuevo valor de altitud
    // Te devuelve el valor suavizado (si hay suficientes datos)
    // o el mismo valor crudo si todavía estás "llenando" el buffer
    float update(float newValue);

private:
    static const int windowSize = 5;
    float buffer[windowSize];
    int index;
    bool bufferFilled;
};

#endif
