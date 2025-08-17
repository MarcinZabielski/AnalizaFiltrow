#include <stdio.h>
#include <stdlib.h>
#include "./lib/fixedpointQ24.h"
#include "./lib/structuresQ24s.h"
#include "../_filtercoeffs/filtercoeffs.h"

#define N 4096
#define ORDER 5     // Rząd_filtra + 1
#define SECTIONS 2  // liczba sekcji w CASCADE

#define FILENAME "../_Analiza(python)/Impulse/impulse_response_q24.csv"

// generowanie odpowiedzi impulsowej filtra dla formatu stałopozycyjnego Q8.24

//Kompilacja: gcc -o Test_impulse_fixedQ24 Test_impulse_fixedQ24.c ./lib/fixedpointQ24.c ./lib/structuresQ24s.c ../_filtercoeffs/filtercoeffs.c

// === Globalne liczniki (nieuzywane tu) ===
int q24_overflow_count = 0;
int q24_underflow_count = 0;

// Konwersja BA: coeffs[2][N] -> b_q[N], a_q[N]
void convert_ba_to_q24(double ba[2][ORDER], int order, q24 *b_q, q24 *a_q) {
    for (int i = 0; i < order; ++i) {
        b_q[i] = double_to_q24((double)ba[0][i]);
        a_q[i] = double_to_q24((double)ba[1][i]);
        printf("b_d[%d] = %f, b_q[%d] = %d\n", i, ba[0][i], i, b_q[i]);
        printf("a_d[%d] = %f, a_q[%d] = %d\n", i, ba[1][i], i, a_q[i]);
    }
}

// Konwersja SOS: sos[M][6] -> q24 sos_flat[M * 6]
void convert_sos_to_q24(double sos[][6], int sections, q24 *sos_q24) {
    for (int s = 0; s < sections; ++s) {
        for (int i = 0; i < 6; ++i) {
            sos_q24[s * 6 + i] = double_to_q24((double)sos[s][i]);
        }
    }
}

int main() {
    q24 b[ORDER];
    q24 a[ORDER];
    q24 sos_q[SECTIONS * 6];

    // konwersja
    convert_ba_to_q24(ellip_df1_order4_cut2000_f64_ba, ORDER, b, a);
    convert_sos_to_q24(ellip_cascade_order4_cut2000_f64_sos, SECTIONS, sos_q);

    // Bufory
    q24 x[N] = {0};
    q24 y_df1[N] = {0};
    q24 y_df2[N] = {0};
    q24 y_tdf2[N] = {0};
    q24 y_cascade[N] = {0};

    x[0] = double_to_q24(1.0); // impuls

    DF1_q24(x, y_df1, b, a, N, ORDER);
    DF2_q24(x, y_df2, b, a, N, ORDER);
    TDF2_q24(x, y_tdf2, b, a, N, ORDER);
    CASCADE_q24(x, y_cascade, sos_q, N, SECTIONS);

    FILE *fp = fopen(FILENAME, "w");
    fprintf(fp, "sample,DF1,DF2,TDF2,CASCADE\n");
    for (int i = 0; i < N; i++) {
        fprintf(fp, "%d,%.10f,%.10f,%.10f,%.10f\n", i,
            q24_to_double(y_df1[i]),
            q24_to_double(y_df2[i]),
            q24_to_double(y_tdf2[i]),
            q24_to_double(y_cascade[i])
        );
    }
    fclose(fp);

    return 0;
}