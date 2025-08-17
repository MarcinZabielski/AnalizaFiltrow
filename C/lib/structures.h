#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <stddef.h>

void DF1_f(float *x, float *y, float *b, float *a, int N, int order);
void DF2_f(float *x, float *y, float *b, float *a, int N, int order);
void TDF2_f(float *x, float *y, float *b, float *a, int N, int order);
void CASCADE_f(float *x, float *y, float *sos, int N, int sections);

void DF1_d(double *x, double *y, double *b, double *a, int N, int order);
void DF2_d(double *x, double *y, double *b, double *a, int N, int order);
void TDF2_d(double *x, double *y, double *b, double *a, int N, int order);
void CASCADE_d(double *x, double *y, double *sos, int N, int sections);


#endif // STRUCTURES_H