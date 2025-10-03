#ifndef STRUCTURESQ31_H
#define STRUCTURESQ31_H

#include "fixedpointQ31.h"

void DF1_q31(q31 *x, q31 *y, q31 *b, q31 *a, int N, int order);
void DF2_q31(q31 *x, q31 *y, q31 *b, q31 *a, int N, int order);
void TDF2_q31(q31 *x, q31 *y, q31 *b, q31 *a, int N, int order);
void CASCADE_q31(q31 *x, q31 *y, q31 *sos, int N, int sections);

#endif
