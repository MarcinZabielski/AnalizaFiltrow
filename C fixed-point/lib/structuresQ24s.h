#ifndef STRUCTURESQ24_H
#define STRUCTURESQ24_H

#include "fixedpointQ24.h"

void DF1_q24(q24 *x, q24 *y, q24 *b, q24 *a, int N, int order);
void DF2_q24(q24 *x, q24 *y, q24 *b, q24 *a, int N, int order);
void TDF2_q24(q24 *x, q24 *y, q24 *b, q24 *a, int N, int order);
void CASCADE_q24(q24 *x, q24 *y, q24 *sos, int N, int sections);

#endif
