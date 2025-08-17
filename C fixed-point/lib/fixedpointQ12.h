#ifndef FIXEDPOINTQ12_H
#define FIXEDPOINTQ12_H

#include <stdint.h>

#define Q12_SHIFT 12
#define Q12_ONE   (1 << Q12_SHIFT)
#define Q12_MIN   -32768
#define Q12_MAX   32767

typedef int16_t q12;

// === Konwersje ===
q12 float_to_q12(float x);
float q12_to_float(q12 x);
q12 double_to_q12(double x);
double q12_to_double(q12 x);

// === Operacje ===
q12 q12_add(q12 a, q12 b);
q12 q12_sub(q12 a, q12 b);
q12 q12_mul(q12 a, q12 b);
q12 q12_add_s(q12 a, q12 b);
q12 q12_sub_s(q12 a, q12 b);
q12 q12_mul_s(q12 a, q12 b);
q12 q12_saturate(int32_t value);

#endif // FIXEDPOINTQ12_H