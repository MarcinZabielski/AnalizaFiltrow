#include <stdint.h>
#include <limits.h>
#include "fixedpointQ24.h"

// === Licznik przepełnienia ===
extern int q24_overflow_count;
extern int q24_underflow_count; 

// === Konwersje ===
q24 float_to_q24(float x) {
    return (q24)(x * (float)Q24_ONE);
}

float q24_to_float(q24 x) {
    return (float)x / (float)Q24_ONE;
}

q24 double_to_q24(double x) {
    return (q24)(x * (double)Q24_ONE);
}

double q24_to_double(q24 x) {
    return (double)x / (double)Q24_ONE;
}

// === Operacje ===

q24 q24_saturate(int64_t value) {
    if (value > Q24_MAX) {
        q24_overflow_count++; 
        return Q24_MAX;
    }
    if (value < Q24_MIN) {
        q24_underflow_count++; 
        return Q24_MIN;
    }
    return (q24)value;
}

q24 q24_add(q24 a, q24 b) {
    int64_t temp = (int64_t)a + b;
    return temp;
}

q24 q24_sub(q24 a, q24 b) {
    int64_t temp = (int64_t)a - b;
    return temp;
}

q24 q24_mul(q24 a, q24 b) {
    int64_t temp = (int64_t)a * b;
    temp >>= Q24_SHIFT;
    return temp;
}

q24 q24_add_s(q24 a, q24 b) {
    int64_t temp = (int64_t)a + b;
    return q24_saturate(temp);
}

q24 q24_sub_s(q24 a, q24 b) {
    int64_t temp = (int64_t)a - b;
    return q24_saturate(temp);
}

q24 q24_mul_s(q24 a, q24 b) {
    int64_t temp = (int64_t)a * b;
    temp >>= Q24_SHIFT;
    return q24_saturate(temp);
}