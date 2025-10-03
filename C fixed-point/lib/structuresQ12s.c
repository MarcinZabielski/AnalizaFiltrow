#include <stddef.h>
#include <stdlib.h>
#include "fixedpointQ12.h"
#include "structuresQ12.h"

void DF1_q12(q12 *x, q12 *y, q12 *b, q12 *a, int N, int order) {
    for (int n = 0; n < N; ++n) {
        int32_t acc = 0;
        for (int i = 0; i < order; ++i) {
            if (n - i >= 0) 
                acc = q12_add_s(acc, q12_mul_s(b[i], x[n - i]));
            if (i > 0 && n - i >= 0) 
                acc = q12_sub_s(acc, q12_mul_s(a[i], y[n - i]));
        }
        y[n] = acc;
    }
}

void DF2_q12(q12 *x, q12 *y, q12 *b, q12 *a, int N, int order) {
    int32_t w[64] = {0};  //max order 64

    for (int n = 0; n < N; ++n) {
        w[0] = x[n];
        for (int i = 1; i < order; ++i) {
            w[0] = q12_sub_s(w[0], q12_mul_s(a[i], w[i]));
        }

        int32_t acc = 0;
        for (int i = 0; i < order; ++i) {
            acc = q12_add_s(acc, q12_mul_s(b[i], w[i]));
        }

        y[n] = q12_saturate(acc);

        for (int i = order - 1; i > 0; --i) {
            w[i] = w[i - 1];
        }
    }
}

void TDF2_q12(q12 *x, q12 *y, q12 *b, q12 *a, int N, int order) {
    int32_t w[64] = {0};  // max order 64

    for (int n = 0; n < N; ++n) {
        int32_t acc = q12_add_s(w[0], q12_mul_s(b[0], x[n]));

        for (int i = 0; i < order - 2; ++i) {
            w[i] = q12_add_s(
                        q12_sub_s(w[i + 1], q12_mul_s(a[i + 1], acc)),
                        q12_mul_s(b[i + 1], x[n])
                    );
        }

        w[order - 2] = q12_sub_s(q12_mul_s(b[order - 1], x[n]),
                               q12_mul_s(a[order - 1], acc));

        y[n] = q12_saturate(acc);
    }
}

void CASCADE_q12(q12 *x, q12 *y, q12 *sos, int N, int sections) {
    q12 *temp = malloc(N * sizeof(q12));
    q12 *in = x;
    q12 *out = temp;

    for (int s = 0; s < sections; ++s) {
        q12 b0 = sos[s * 6 + 0];
        q12 b1 = sos[s * 6 + 1];
        q12 b2 = sos[s * 6 + 2];
        q12 a0 = sos[s * 6 + 3];
        q12 a1 = sos[s * 6 + 4];
        q12 a2 = sos[s * 6 + 5];

        int32_t w1 = 0, w2 = 0;

        for (int n = 0; n < N; ++n) {
            int32_t wn = q12_sub_s(in[n],
                               q12_add_s(q12_mul_s(a1, w1), q12_mul_s(a2, w2)));

            out[n] = q12_add_s(
                         q12_add_s(q12_mul_s(b0, wn), q12_mul_s(b1, w1)),
                         q12_mul_s(b2, w2)
                     );

            w2 = w1;
            w1 = wn;
        }

        q12 *tmp = in;
        in = out;
        out = tmp;
    }

    if (in != y) {
        for (int i = 0; i < N; ++i) y[i] = in[i];
    }

    free(temp);
}