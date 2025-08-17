#include <stddef.h>
#include <stdlib.h>
#include "fixedpointQ31.h"
#include "structuresQ31.h"

void DF1_q31(q31 *x, q31 *y, q31 *b, q31 *a, int N, int order) {
    for (int n = 0; n < N; ++n) {
        int64_t acc = 0;
        for (int i = 0; i < order; ++i) {
            if (n - i >= 0) 
                acc = q31_add(acc, q31_mul(b[i], x[n - i]));
            if (i > 0 && n - i >= 0) 
                acc = q31_sub(acc, q31_mul(a[i], y[n - i]));
        }
        y[n] = q31_saturate(acc >> Q31_SHIFT);
    }
}

void DF2_q31(q31 *x, q31 *y, q31 *b, q31 *a, int N, int order) {
    int64_t w[64] = {0};  //max order 64

    for (int n = 0; n < N; ++n) {
        w[0] = x[n];
        for (int i = 1; i < order; ++i) {
            w[0] = q31_sub(w[0], q31_mul(a[i], w[i]));
        }

        int64_t acc = 0;
        for (int i = 0; i < order; ++i) {
            acc = q31_add(acc, q31_mul(b[i], w[i]));
        }

        y[n] = (q31)acc;

        for (int i = order - 1; i > 0; --i) {
            w[i] = w[i - 1];
        }
    }
}

void TDF2_q31(q31 *x, q31 *y, q31 *b, q31 *a, int N, int order) {
    int64_t w[64] = {0};  // max order 64

    for (int n = 0; n < N; ++n) {
        int64_t yn = q31_add(w[0], q31_mul(b[0], x[n]));

        for (int i = 0; i < order - 2; ++i) {
            w[i] = q31_add(
                        q31_sub(w[i + 1], q31_mul(a[i + 1], yn)),
                        q31_mul(b[i + 1], x[n])
                    );
        }

        w[order - 2] = q31_sub(q31_mul(b[order - 1], x[n]),
                               q31_mul(a[order - 1], yn));

        y[n] = q31_saturate(yn >> Q31_SHIFT);
    }
}

void CASCADE_q31(q31 *x, q31 *y, q31 *sos, int N, int sections) {
    q31 *temp = malloc(N * sizeof(q31));
    q31 *in = x;
    q31 *out = temp;

    for (int s = 0; s < sections; ++s) {
        q31 b0 = sos[s * 6 + 0];
        q31 b1 = sos[s * 6 + 1];
        q31 b2 = sos[s * 6 + 2];
        q31 a0 = sos[s * 6 + 3];  // zazwyczaj == 1
        q31 a1 = sos[s * 6 + 4];
        q31 a2 = sos[s * 6 + 5];

        int64_t w1 = 0, w2 = 0;

        for (int n = 0; n < N; ++n) {
            int64_t wn = q31_sub(in[n],
                               q31_add(q31_mul(a1, w1), q31_mul(a2, w2)));

            out[n] = q31_add(
                         q31_add(q31_mul(b0, wn), q31_mul(b1, w1)),
                         q31_mul(b2, w2)
                     );

            w2 = w1;
            w1 = wn;
        }

        q31 *tmp = in;
        in = out;
        out = tmp;
    }

    if (in != y) {
        for (int i = 0; i < N; ++i) y[i] = in[i];
    }

    free(temp);
}