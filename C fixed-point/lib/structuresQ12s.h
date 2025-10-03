#ifndef STRUCTURESQ12_H
#define STRUCTURESQ12_H

#include "fixedpointQ12.h"

void DF1_q12(q12 *x, q12 *y, q12 *b, q12 *a, int N, int order);
void DF2_q12(q12 *x, q12 *y, q12 *b, q12 *a, int N, int order);
void TDF2_q12(q12 *x, q12 *y, q12 *b, q12 *a, int N, int order);
void CASCADE_q12(q12 *x, q12 *y, q12 *sos, int N, int sections);

#endif
