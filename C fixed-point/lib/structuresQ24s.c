#include <stddef.h>
#include <stdlib.h>
#include "fixedpointQ24.h"
#include "structuresQ24s.h"

void DF1_q24(q24 *x, q24 *y, q24 *b, q24 *a, int N, int order) {
    for (int n = 0; n < N; ++n) {
        int64_t acc = 0;
        for (int i = 0; i < order; ++i) {
            if (n - i >= 0)
                acc = q24_add_s(acc, q24_mul_s(b[i], x[n - i]));
            if (i > 0 && n - i >= 0)
                acc = q24_sub_s(acc, q24_mul_s(a[i], y[n - i]));
        }
        y[n] = q24_saturate(acc);
    }
}

void DF2_q24(q24 *x, q24 *y, q24 *b, q24 *a, int N, int order) {
    int64_t w[64] = {0};  //max order 64

    for (int n = 0; n < N; ++n) {
        w[0] = x[n];
        for (int i = 1; i < order; ++i) {
            w[0] = q24_sub_s(w[0], q24_mul_s(a[i], w[i]));
        }

        int64_t acc = 0;
        for (int i = 0; i < order; ++i) {
            acc = q24_add_s(acc, q24_mul_s(b[i], w[i]));
        }

        y[n] = q24_saturate(acc);

        for (int i = order - 1; i > 0; --i) {
            w[i] = w[i - 1];
        }
    }
}

void TDF2_q24(q24 *x, q24 *y, q24 *b, q24 *a, int N, int order) {
    int64_t w[64] = {0};  // max order 64

    for (int n = 0; n < N; ++n) {
        int64_t acc = q24_add_s(w[0], q24_mul_s(b[0], x[n]));

        for (int i = 0; i < order - 2; ++i) {
            w[i] = q24_add_s(
                        q24_sub_s(w[i + 1], q24_mul_s(a[i + 1], acc)),
                        q24_mul_s(b[i + 1], x[n])
                    );
        }

        w[order - 2] = q24_sub_s(q24_mul_s(b[order - 1], x[n]),q24_mul_s(a[order - 1], acc));

        y[n] = q24_saturate(acc);
    }
}

void CASCADE_q24(q24 *x, q24 *y, q24 *sos, int N, int sections) {
    q24 *temp = malloc(N * sizeof(q24));
    q24 *in = x;
    q24 *out = temp;

    for (int s = 0; s < sections; ++s) {
        q24 b0 = sos[s * 6 + 0];
        q24 b1 = sos[s * 6 + 1];
        q24 b2 = sos[s * 6 + 2];
        q24 a0 = sos[s * 6 + 3];
        q24 a1 = sos[s * 6 + 4];
        q24 a2 = sos[s * 6 + 5];

        int64_t w1 = 0, w2 = 0;

        for (int n = 0; n < N; ++n) {
            int64_t wn = q24_sub_s(in[n],
                               q24_add_s(q24_mul(a1, w1), q24_mul_s(a2, w2)));

            out[n] = q24_add(
                         q24_add_s(q24_mul(b0, wn), q24_mul_s(b1, w1)),
                         q24_mul_s(b2, w2)
                     );

            w2 = w1;
            w1 = wn;
        }

        q24 *tmp = in;
        in = out;
        out = tmp;
    }

    if (in != y) {
        for (int i = 0; i < N; ++i) y[i] = in[i];
    }

    free(temp);
}