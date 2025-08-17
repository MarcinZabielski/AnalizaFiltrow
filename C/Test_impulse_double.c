#include <stdio.h>
#include <stdlib.h>
#include "./lib/structures.h"
#include "../_filtercoeffs/filtercoeffs.h"

#define N 4096
#define ORDER 5     // Rząd_filtra + 1
#define SECTIONS 2  // liczba sekcji w CASCADE

#define FILENAME "../_Analiza(python)/Impulse/impulse_response_double.csv"

// generowanie odpowiedzi impulsowej filtra dla formatu zmiennopozycyjnego double

//Kompilacja: gcc -o Test_impulse_double Test_impulse_double.c ./lib/structures.c ../_filtercoeffs/filtercoeffs.c

int main() {
    double b[ORDER];
    double a[ORDER];
    double sos[SECTIONS * 6];

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
    double x[N] = {0};
    double y_df1[N] = {0};
    double y_df2[N] = {0};
    double y_tdf2[N] = {0};
    double y_cascade[N] = {0};

    x[0] = 1.0; // impuls

    DF1_d(x, y_df1, b, a, N, ORDER);
    DF2_d(x, y_df2, b, a, N, ORDER);
    TDF2_d(x, y_tdf2, b, a, N, ORDER);
    CASCADE_d(x, y_cascade, sos, N, SECTIONS);

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