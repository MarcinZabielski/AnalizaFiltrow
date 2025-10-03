#include "fixedpointQ12.h"

// === Licznik przepełnień ===
extern int q12_overflow_count; 
extern int q12_underflow_count; 

// === Konwersje ===
q12 float_to_q12(float x) {
    return (q12)(x * (float)Q12_ONE);
}

float q12_to_float(q12 x) {
    return (float)x / (float)Q12_ONE;
}

q12 double_to_q12(double x) {
    return (q12)(x * (double)Q12_ONE);
}

double q12_to_double(q12 x) {
    return (double)x / (double)Q12_ONE;
}

// === Operacje ===
q12 q12_saturate(int32_t value) {
    if (value > Q12_MAX) {
        q12_overflow_count++; 
        return Q12_MAX;
    }
    if (value < Q12_MIN) {
        q12_underflow_count++; 
        return Q12_MIN;
    }
    return (q12)value;
}

q12 q12_add(q12 a, q12 b) {
    int32_t temp = (int32_t)a + (int32_t)b;
    return temp;
}

q12 q12_sub(q12 a, q12 b) {
    int32_t temp = (int32_t)a - (int32_t)b;
    return temp;
}

q12 q12_mul(q12 a, q12 b) {
    int32_t temp = (int32_t)a * (int32_t)b;
    temp >>= Q12_SHIFT;
    return temp;
}

q12 q12_add_s(q12 a, q12 b) {
    int32_t temp = (int32_t)a + (int32_t)b;
    return q12_saturate(temp);
}

q12 q12_sub_s(q12 a, q12 b) {
    int32_t temp = (int32_t)a - (int32_t)b;
    return q12_saturate(temp);
}

q12 q12_mul_s(q12 a, q12 b) {
    int32_t temp = (int32_t)a * (int32_t)b;
    temp >>= Q12_SHIFT;
    return q12_saturate(temp);
}