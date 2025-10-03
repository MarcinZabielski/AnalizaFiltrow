#ifndef FIXEDPOINTQ24_H
#define FIXEDPOINTQ24_H

#include <stdint.h>

#define Q24_SHIFT 24
#define Q24_ONE   (1 << Q24_SHIFT)
#define Q24_MIN   -2147483648
#define Q24_MAX   2147483647

typedef int32_t q24;

//extern int q411_overflow_count;

// === Konwersje ===
q24 float_to_q24(float x);
float q24_to_float(q24 x);
q24 double_to_q24(double x);
double q24_to_double(q24 x);

// === Operacje ===
q24 q24_add(q24 a, q24 b);
q24 q24_sub(q24 a, q24 b);
q24 q24_mul(q24 a, q24 b);
q24 q24_div(q24 a, q24 b);
q24 q24_add_s(q24 a, q24 b);
q24 q24_sub_s(q24 a, q24 b);
q24 q24_mul_s(q24 a, q24 b);
q24 q24_saturate(int64_t value);

#endif
