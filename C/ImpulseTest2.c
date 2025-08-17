#include <stdio.h>
#include <stdlib.h>
#include "./lib/structures.h"
#include "../_filtercoeffs/filtercoeffs.h"

#define N 512

int main() {
    float x_f32[N] = {0.0f};
    double x_f64[N] = {0.0};

    x_f32[0] = 1.0f;  // impuls
    x_f64[0] = 1.0;   // impuls

    // Bufory wyjściowe
    float y_df1_f32[N] = {0}, y_df2_f32[N] = {0}, y_tdf2_f32[N] = {0}, y_cascade_f32[N] = {0};
    double y_df1_f64[N] = {0}, y_df2_f64[N] = {0}, y_tdf2_f64[N] = {0}, y_cascade_f64[N] = {0};

    // F32 - filtracja
    DF1_f(x_f32, y_df1_f32, ellip_df1_order6_cut1000_f32_ba[0], ellip_df1_order6_cut1000_f32_ba[1], N, 7);
    DF2_f(x_f32, y_df2_f32, ellip_df1_order6_cut1000_f32_ba[0], ellip_df1_order6_cut1000_f32_ba[1], N, 7);
    TDF2_f(x_f32, y_tdf2_f32, ellip_df1_order6_cut1000_f32_ba[0], ellip_df1_order6_cut1000_f32_ba[1], N, 7);
    CASCADE_f(x_f32, y_cascade_f32, (float *)ellip_cascade_order6_cut1000_f32_sos, N, 3);

    // F64 - filtracja
    DF1_d(x_f64, y_df1_f64, ellip_df1_order6_cut1000_f64_ba[0], ellip_df1_order6_cut1000_f64_ba[1], N, 7);
    DF2_d(x_f64, y_df2_f64, ellip_df1_order6_cut1000_f64_ba[0], ellip_df1_order6_cut1000_f64_ba[1], N, 7);
    TDF2_d(x_f64, y_tdf2_f64, ellip_df1_order6_cut1000_f64_ba[0], ellip_df1_order6_cut1000_f64_ba[1], N, 7);
    CASCADE_d(x_f64, y_cascade_f64, (double *)ellip_cascade_order6_cut1000_f64_sos, N, 3);

    // Zapis do pliku CSV
    FILE *f = fopen("output.csv", "w");
    if (!f) {
        perror("Nie można otworzyć pliku");
        return 1;
    }

    fprintf(f, "index,df1_f32,df2_f32,tdf2_f32,cascade_f32,df1_f64,df2_f64,tdf2_f64,cascade_f64\n");

    for (int i = 0; i < N; ++i) {
        fprintf(f, "%d,%.8e,%.8e,%.8e,%.8e,%.16e,%.16e,%.16e,%.16e\n",
                i,
                y_df1_f32[i], y_df2_f32[i], y_tdf2_f32[i], y_cascade_f32[i],
                y_df1_f64[i], y_df2_f64[i], y_tdf2_f64[i], y_cascade_f64[i]);
    }

    fclose(f);
    printf("Zapisano dane do output.csv\n");
    return 0;
}
