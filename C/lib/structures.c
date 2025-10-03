#include "structures.h"
#include <stddef.h>
#include <stdlib.h>

void DF1_f(float *x, float *y, float *b, float *a, int N, int order) {
    for (int n = 0; n < N; ++n) {
        y[n] = 0.0f;
        for (int i = 0; i < order; ++i) {
            if (n - i >= 0) y[n] += b[i] * x[n - i];
            if (i > 0 && n - i >= 0) y[n] -= a[i] * y[n - i];
        }
    }
}

void DF2_f(float *x, float *y, float *b, float *a, int N, int order) {
    float w[64] = {0}; // zakładamy max order 64
    for (int n = 0; n < N; ++n) {
        w[0] = x[n];
        for (int i = 1; i < order; ++i)
            w[0] -= a[i] * w[i];
        y[n] = 0.0f;
        for (int i = 0; i < order; ++i)
            y[n] += b[i] * w[i];
        for (int i = order - 1; i > 0; --i)
            w[i] = w[i - 1];
    }
}

void TDF2_f(float *x, float *y, float *b, float *a, int N, int order) {
    float w[64] = {0.0f};  // zakładamy max rząd 64

    for (int n = 0; n < N; ++n) {
        float yn = w[0] + b[0] * x[n];

        for (int i = 0; i < order - 2; ++i) {
            w[i] = w[i + 1] + b[i + 1] * x[n] - a[i + 1] * yn;
        }

        w[order - 2] = b[order - 1] * x[n] - a[order - 1] * yn;

        y[n] = yn;
    }
}

void CASCADE_f(float *x, float *y, float *sos, int N, int sections) {
    float *temp = malloc(N * sizeof(float));
    float *in = x;
    float *out = temp;

    for (int s = 0; s < sections; ++s) {
        float b0 = sos[s * 6 + 0];
        float b1 = sos[s * 6 + 1];
        float b2 = sos[s * 6 + 2];
        float a0 = sos[s * 6 + 3];
        float a1 = sos[s * 6 + 4];
        float a2 = sos[s * 6 + 5];
        float w1 = 0.0f, w2 = 0.0f;

        for (int n = 0; n < N; ++n) {
            float wn = in[n] - a1 * w1 - a2 * w2;
            out[n] = b0 * wn + b1 * w1 + b2 * w2;
            w2 = w1;
            w1 = wn;
        }
        float *tmp = in;
        in = out;
        out = tmp;
    }

    if (in != y) for (int i = 0; i < N; ++i) y[i] = in[i];
    free(temp);
}

void DF1_d(double *x, double *y, double *b, double *a, int N, int order) {
    for (int n = 0; n < N; ++n) {
        y[n] = 0.0;
        for (int i = 0; i < order; ++i) {
            if (n - i >= 0) y[n] += b[i] * x[n - i];
            if (i > 0 && n - i >= 0) y[n] -= a[i] * y[n - i];
        }
    }
}

void DF2_d(double *x, double *y, double *b, double *a, int N, int order) {
    double w[64] = {0}; // zakładamy max order 64
    for (int n = 0; n < N; ++n) {
        w[0] = x[n];
        for (int i = 1; i < order; ++i)
            w[0] -= a[i] * w[i];
        y[n] = 0.0;
        for (int i = 0; i < order; ++i)
            y[n] += b[i] * w[i];
        for (int i = order - 1; i > 0; --i)
            w[i] = w[i - 1];
    }
}

void TDF2_d(double *x, double *y, double *b, double *a, int N, int order) {
    double w[64] = {0.0};  // zakładamy max rząd 64

    for (int n = 0; n < N; ++n) {
        double yn = w[0] + b[0] * x[n];

        for (int i = 0; i < order - 2; ++i) {
            w[i] = w[i + 1] + b[i + 1] * x[n] - a[i + 1] * yn;
        }

        w[order - 2] = b[order - 1] * x[n] - a[order - 1] * yn;

        y[n] = yn;
    }
}

void CASCADE_d(double *x, double *y, double *sos, int N, int sections) {
    double *temp = malloc(N * sizeof(double));
    double *in = x;
    double *out = temp;

    for (int s = 0; s < sections; ++s) {
        double b0 = sos[s * 6 + 0];
        double b1 = sos[s * 6 + 1];
        double b2 = sos[s * 6 + 2];
        double a0 = sos[s * 6 + 3];
        double a1 = sos[s * 6 + 4];
        double a2 = sos[s * 6 + 5];
        double w1 = 0.0, w2 = 0.0;

        for (int n = 0; n < N; ++n) {
            double wn = in[n] - a1 * w1 - a2 * w2;
            out[n] = b0 * wn + b1 * w1 + b2 * w2;
            w2 = w1;
            w1 = wn;
        }
        double *tmp = in;
        in = out;
        out = tmp;
    }

    if (in != y) for (int i = 0; i < N; ++i) y[i] = in[i];
    free(temp);
}
