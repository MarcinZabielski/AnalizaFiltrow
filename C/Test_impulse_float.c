#include <stdio.h>
#include <stdlib.h>
#include "./lib/structures.h"
#include "../_filtercoeffs/filtercoeffs.h"

#define N 4096
#define ORDER 5     // Rząd_filtra + 1
#define SECTIONS 2  // liczba sekcji w CASCADE

#define FILENAME "../_Analiza(python)/Impulse/impulse_response_float.csv"

// generowanie odpowiedzi impulsowej filtra dla formatu zmiennopozycyjnego float

//Kompilacja: gcc -o Test_impulse_float Test_impulse_float.c ./lib/structures.c ../_filtercoeffs/filtercoeffs.c

int main() {
    float b[ORDER];
    float a[ORDER];
    float sos[SECTIONS * 6];

    for (int i = 0; i < ORDER; ++i) {
        b[i] = ellip_df1_order4_cut2000_f64_ba[0][i];
        a[i] = ellip_df1_order4_cut2000_f64_ba[1][i];
    }

    for (int s = 0; s < SECTIONS; ++s) {
        for (int i = 0; i < 6; ++i) {
            sos[s * 6 + i] = ellip_cascade_order4_cut2000_f64_sos[s][i];
        }
    }

    // Bufory
    float x[N] = {0};
    float y_df1[N] = {0};
    float y_df2[N] = {0};
    float y_tdf2[N] = {0};
    float y_cascade[N] = {0};

    x[0] = 1.0f; // impuls

    DF1_f(x, y_df1, b, a, N, ORDER);
    DF2_f(x, y_df2, b, a, N, ORDER);
    TDF2_f(x, y_tdf2, b, a, N, ORDER);
    CASCADE_f(x, y_cascade, sos, N, SECTIONS);

    FILE *fp = fopen(FILENAME, "w");
    fprintf(fp, "sample,DF1,DF2,TDF2,CASCADE\n");
    for (int i = 0; i < N; i++) {
        fprintf(fp, "%d,%.10f,%.10f,%.10f,%.10f\n", i,
            y_df1[i],
            y_df2[i],
            y_tdf2[i],
            y_cascade[i]
        );
    }
    fclose(fp);

    return 0;
}