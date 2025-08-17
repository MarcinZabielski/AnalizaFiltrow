#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "./lib/structures.h"
#include "../_filtercoeffs/filtercoeffs.h"

#define N 4096 //dlugosc impulse i rand

// Analiza precyzji filtrowania w języku C (reprezentacja zmiennopozycyjna)
//Kompilacja: gcc -o PrecisionAnalysis PrecisionAnalysis.c ./lib/structures.c ../_filtercoeffs/filtercoeffs.c

// === MAE ===
double compute_mae(const double *ref, const float *test, int len) {
    double sum = 0.0;
    for (int i = 0; i < len; i++) {
        sum += fabs(ref[i] - test[i]);
    }
    return sum / len;
}

// === Analiza precyzji ===
void precision_analysis(FILE *fp, const char *filter_name, const char *structure,
                        void (*func_f)(float*, float*, float*, float*, int, int),
                        void (*func_d)(double*, double*, double*, double*, int, int),
                        float *b_f, float *a_f, double *b_d, double *a_d, int order) {

    int cutoff = -1;
    char filter_type[32];
    sscanf(filter_name, "%[^_]", filter_type);

    char *cut_ptr = strstr(filter_name, "_cut");
    if (cut_ptr != NULL)
        sscanf(cut_ptr, "_cut%d", &cutoff);

    float *x_f = calloc(N, sizeof(float));
    float *y_f = calloc(N, sizeof(float));
    double *x_d = calloc(N, sizeof(double));
    double *y_d = calloc(N, sizeof(double));

    // === impulse ===
    x_f[0] = 1.0f;
    x_d[0] = 1.0;

    func_f(x_f, y_f, b_f, a_f, N, order);
    func_d(x_d, y_d, b_d, a_d, N, order);

    double mae_impulse = compute_mae(y_d, y_f, N);
    fprintf(fp, "%s,float,%s,%d,%d,impulse,%.8e\n", filter_type, structure, cutoff, order - 1, mae_impulse);

    // === rand ===
    for (int i = 0; i < N; i++) {
        float r = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        x_f[i] = r;
        x_d[i] = r;
    }

    memset(y_f, 0, sizeof(float) * N);
    memset(y_d, 0, sizeof(double) * N);

    func_f(x_f, y_f, b_f, a_f, N, order);
    func_d(x_d, y_d, b_d, a_d, N, order);

    double mae_rand = compute_mae(y_d, y_f, 128);
    fprintf(fp, "%s,float,%s,%d,%d,rand,%.8e\n", filter_type, structure, cutoff, order - 1, mae_rand);

    free(x_f); free(y_f); free(x_d); free(y_d);
}

void precision_analysis_cascade(FILE *fp, const char *filter_name,
                                 void (*func_f)(float*, float*, float*, int, int),
                                 void (*func_d)(double*, double*, double*, int, int),
                                 float *sos_f, double *sos_d, int sections) {

    int cutoff = -1;
    char filter_type[32];
    sscanf(filter_name, "%[^_]", filter_type);

    char *cut_ptr = strstr(filter_name, "_cut");
    if (cut_ptr != NULL)
        sscanf(cut_ptr, "_cut%d", &cutoff);

    float *x_f = calloc(N, sizeof(float));
    float *y_f = calloc(N, sizeof(float));
    double *x_d = calloc(N, sizeof(double));
    double *y_d = calloc(N, sizeof(double));

    // === impulse ===
    x_f[0] = 1.0f;
    x_d[0] = 1.0;

    func_f(x_f, y_f, sos_f, N, sections);
    func_d(x_d, y_d, sos_d, N, sections);

    double mae_impulse = compute_mae(y_d, y_f, N);
    fprintf(fp, "%s,float,CASCADE,%d,%d,impulse,%.8e\n", filter_type, cutoff, 2 * sections, mae_impulse);

    // === rand ===
    for (int i = 0; i < N; i++) {
        float r = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        x_f[i] = r;
        x_d[i] = r;
    }

    memset(y_f, 0, sizeof(float) * N);
    memset(y_d, 0, sizeof(double) * N);

    func_f(x_f, y_f, sos_f, N, sections);
    func_d(x_d, y_d, sos_d, N, sections);

    double mae_rand = compute_mae(y_d, y_f, 128);
    fprintf(fp, "%s,float,CASCADE,%d,%d,rand,%.8e\n", filter_type, cutoff, 2 * sections, mae_rand);

    free(x_f); free(y_f); free(x_d); free(y_d);
}


int main() {
    FILE *fp_precision = fopen("c_precision_results.csv", "w");
    if (!fp_precision) {
        perror("Can't open CSV file");
        return 1;
    }
    fprintf(fp_precision, "filter_name,type,structure,cutoff,order,signal,MAE\n");

    // Tu wkleić zawartość pliku generated_calls_precision_floating.txt
    
    precision_analysis(fp_precision, "butter_df1_order2_cut1000_ba", "DF1", DF1_f, DF1_d, butter_df1_order2_cut1000_f32_ba[0], butter_df1_order2_cut1000_f32_ba[1], butter_df1_order2_cut1000_f64_ba[0], butter_df1_order2_cut1000_f64_ba[1], 3);
    precision_analysis(fp_precision, "butter_df2_order2_cut1000_ba", "DF2", DF2_f, DF2_d, butter_df2_order2_cut1000_f32_ba[0], butter_df2_order2_cut1000_f32_ba[1], butter_df2_order2_cut1000_f64_ba[0], butter_df2_order2_cut1000_f64_ba[1], 3);
    precision_analysis(fp_precision, "butter_tdf2_order2_cut1000_ba", "TDF2", TDF2_f, TDF2_d, butter_tdf2_order2_cut1000_f32_ba[0], butter_tdf2_order2_cut1000_f32_ba[1], butter_tdf2_order2_cut1000_f64_ba[0], butter_tdf2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_cascade(fp_precision, "butter_cascade_order2_cut1000_sos", CASCADE_f, CASCADE_d, (float*)butter_cascade_order2_cut1000_f32_sos, (double*)butter_cascade_order2_cut1000_f64_sos, 1);
    precision_analysis(fp_precision, "butter_df1_order2_cut2000_ba", "DF1", DF1_f, DF1_d, butter_df1_order2_cut2000_f32_ba[0], butter_df1_order2_cut2000_f32_ba[1], butter_df1_order2_cut2000_f64_ba[0], butter_df1_order2_cut2000_f64_ba[1], 3);
    precision_analysis(fp_precision, "butter_df2_order2_cut2000_ba", "DF2", DF2_f, DF2_d, butter_df2_order2_cut2000_f32_ba[0], butter_df2_order2_cut2000_f32_ba[1], butter_df2_order2_cut2000_f64_ba[0], butter_df2_order2_cut2000_f64_ba[1], 3);
    precision_analysis(fp_precision, "butter_tdf2_order2_cut2000_ba", "TDF2", TDF2_f, TDF2_d, butter_tdf2_order2_cut2000_f32_ba[0], butter_tdf2_order2_cut2000_f32_ba[1], butter_tdf2_order2_cut2000_f64_ba[0], butter_tdf2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_cascade(fp_precision, "butter_cascade_order2_cut2000_sos", CASCADE_f, CASCADE_d, (float*)butter_cascade_order2_cut2000_f32_sos, (double*)butter_cascade_order2_cut2000_f64_sos, 1);
    precision_analysis(fp_precision, "butter_df1_order2_cut5000_ba", "DF1", DF1_f, DF1_d, butter_df1_order2_cut5000_f32_ba[0], butter_df1_order2_cut5000_f32_ba[1], butter_df1_order2_cut5000_f64_ba[0], butter_df1_order2_cut5000_f64_ba[1], 3);
    precision_analysis(fp_precision, "butter_df2_order2_cut5000_ba", "DF2", DF2_f, DF2_d, butter_df2_order2_cut5000_f32_ba[0], butter_df2_order2_cut5000_f32_ba[1], butter_df2_order2_cut5000_f64_ba[0], butter_df2_order2_cut5000_f64_ba[1], 3);
    precision_analysis(fp_precision, "butter_tdf2_order2_cut5000_ba", "TDF2", TDF2_f, TDF2_d, butter_tdf2_order2_cut5000_f32_ba[0], butter_tdf2_order2_cut5000_f32_ba[1], butter_tdf2_order2_cut5000_f64_ba[0], butter_tdf2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_cascade(fp_precision, "butter_cascade_order2_cut5000_sos", CASCADE_f, CASCADE_d, (float*)butter_cascade_order2_cut5000_f32_sos, (double*)butter_cascade_order2_cut5000_f64_sos, 1);
    precision_analysis(fp_precision, "butter_df1_order4_cut1000_ba", "DF1", DF1_f, DF1_d, butter_df1_order4_cut1000_f32_ba[0], butter_df1_order4_cut1000_f32_ba[1], butter_df1_order4_cut1000_f64_ba[0], butter_df1_order4_cut1000_f64_ba[1], 5);
    precision_analysis(fp_precision, "butter_df2_order4_cut1000_ba", "DF2", DF2_f, DF2_d, butter_df2_order4_cut1000_f32_ba[0], butter_df2_order4_cut1000_f32_ba[1], butter_df2_order4_cut1000_f64_ba[0], butter_df2_order4_cut1000_f64_ba[1], 5);
    precision_analysis(fp_precision, "butter_tdf2_order4_cut1000_ba", "TDF2", TDF2_f, TDF2_d, butter_tdf2_order4_cut1000_f32_ba[0], butter_tdf2_order4_cut1000_f32_ba[1], butter_tdf2_order4_cut1000_f64_ba[0], butter_tdf2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_cascade(fp_precision, "butter_cascade_order4_cut1000_sos", CASCADE_f, CASCADE_d, (float*)butter_cascade_order4_cut1000_f32_sos, (double*)butter_cascade_order4_cut1000_f64_sos, 2);
    precision_analysis(fp_precision, "butter_df1_order4_cut2000_ba", "DF1", DF1_f, DF1_d, butter_df1_order4_cut2000_f32_ba[0], butter_df1_order4_cut2000_f32_ba[1], butter_df1_order4_cut2000_f64_ba[0], butter_df1_order4_cut2000_f64_ba[1], 5);
    precision_analysis(fp_precision, "butter_df2_order4_cut2000_ba", "DF2", DF2_f, DF2_d, butter_df2_order4_cut2000_f32_ba[0], butter_df2_order4_cut2000_f32_ba[1], butter_df2_order4_cut2000_f64_ba[0], butter_df2_order4_cut2000_f64_ba[1], 5);
    precision_analysis(fp_precision, "butter_tdf2_order4_cut2000_ba", "TDF2", TDF2_f, TDF2_d, butter_tdf2_order4_cut2000_f32_ba[0], butter_tdf2_order4_cut2000_f32_ba[1], butter_tdf2_order4_cut2000_f64_ba[0], butter_tdf2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_cascade(fp_precision, "butter_cascade_order4_cut2000_sos", CASCADE_f, CASCADE_d, (float*)butter_cascade_order4_cut2000_f32_sos, (double*)butter_cascade_order4_cut2000_f64_sos, 2);
    precision_analysis(fp_precision, "butter_df1_order4_cut5000_ba", "DF1", DF1_f, DF1_d, butter_df1_order4_cut5000_f32_ba[0], butter_df1_order4_cut5000_f32_ba[1], butter_df1_order4_cut5000_f64_ba[0], butter_df1_order4_cut5000_f64_ba[1], 5);
    precision_analysis(fp_precision, "butter_df2_order4_cut5000_ba", "DF2", DF2_f, DF2_d, butter_df2_order4_cut5000_f32_ba[0], butter_df2_order4_cut5000_f32_ba[1], butter_df2_order4_cut5000_f64_ba[0], butter_df2_order4_cut5000_f64_ba[1], 5);
    precision_analysis(fp_precision, "butter_tdf2_order4_cut5000_ba", "TDF2", TDF2_f, TDF2_d, butter_tdf2_order4_cut5000_f32_ba[0], butter_tdf2_order4_cut5000_f32_ba[1], butter_tdf2_order4_cut5000_f64_ba[0], butter_tdf2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_cascade(fp_precision, "butter_cascade_order4_cut5000_sos", CASCADE_f, CASCADE_d, (float*)butter_cascade_order4_cut5000_f32_sos, (double*)butter_cascade_order4_cut5000_f64_sos, 2);
    precision_analysis(fp_precision, "butter_df1_order6_cut1000_ba", "DF1", DF1_f, DF1_d, butter_df1_order6_cut1000_f32_ba[0], butter_df1_order6_cut1000_f32_ba[1], butter_df1_order6_cut1000_f64_ba[0], butter_df1_order6_cut1000_f64_ba[1], 7);
    precision_analysis(fp_precision, "butter_df2_order6_cut1000_ba", "DF2", DF2_f, DF2_d, butter_df2_order6_cut1000_f32_ba[0], butter_df2_order6_cut1000_f32_ba[1], butter_df2_order6_cut1000_f64_ba[0], butter_df2_order6_cut1000_f64_ba[1], 7);
    precision_analysis(fp_precision, "butter_tdf2_order6_cut1000_ba", "TDF2", TDF2_f, TDF2_d, butter_tdf2_order6_cut1000_f32_ba[0], butter_tdf2_order6_cut1000_f32_ba[1], butter_tdf2_order6_cut1000_f64_ba[0], butter_tdf2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_cascade(fp_precision, "butter_cascade_order6_cut1000_sos", CASCADE_f, CASCADE_d, (float*)butter_cascade_order6_cut1000_f32_sos, (double*)butter_cascade_order6_cut1000_f64_sos, 3);
    precision_analysis(fp_precision, "butter_df1_order6_cut2000_ba", "DF1", DF1_f, DF1_d, butter_df1_order6_cut2000_f32_ba[0], butter_df1_order6_cut2000_f32_ba[1], butter_df1_order6_cut2000_f64_ba[0], butter_df1_order6_cut2000_f64_ba[1], 7);
    precision_analysis(fp_precision, "butter_df2_order6_cut2000_ba", "DF2", DF2_f, DF2_d, butter_df2_order6_cut2000_f32_ba[0], butter_df2_order6_cut2000_f32_ba[1], butter_df2_order6_cut2000_f64_ba[0], butter_df2_order6_cut2000_f64_ba[1], 7);
    precision_analysis(fp_precision, "butter_tdf2_order6_cut2000_ba", "TDF2", TDF2_f, TDF2_d, butter_tdf2_order6_cut2000_f32_ba[0], butter_tdf2_order6_cut2000_f32_ba[1], butter_tdf2_order6_cut2000_f64_ba[0], butter_tdf2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_cascade(fp_precision, "butter_cascade_order6_cut2000_sos", CASCADE_f, CASCADE_d, (float*)butter_cascade_order6_cut2000_f32_sos, (double*)butter_cascade_order6_cut2000_f64_sos, 3);
    precision_analysis(fp_precision, "butter_df1_order6_cut5000_ba", "DF1", DF1_f, DF1_d, butter_df1_order6_cut5000_f32_ba[0], butter_df1_order6_cut5000_f32_ba[1], butter_df1_order6_cut5000_f64_ba[0], butter_df1_order6_cut5000_f64_ba[1], 7);
    precision_analysis(fp_precision, "butter_df2_order6_cut5000_ba", "DF2", DF2_f, DF2_d, butter_df2_order6_cut5000_f32_ba[0], butter_df2_order6_cut5000_f32_ba[1], butter_df2_order6_cut5000_f64_ba[0], butter_df2_order6_cut5000_f64_ba[1], 7);
    precision_analysis(fp_precision, "butter_tdf2_order6_cut5000_ba", "TDF2", TDF2_f, TDF2_d, butter_tdf2_order6_cut5000_f32_ba[0], butter_tdf2_order6_cut5000_f32_ba[1], butter_tdf2_order6_cut5000_f64_ba[0], butter_tdf2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_cascade(fp_precision, "butter_cascade_order6_cut5000_sos", CASCADE_f, CASCADE_d, (float*)butter_cascade_order6_cut5000_f32_sos, (double*)butter_cascade_order6_cut5000_f64_sos, 3);
    precision_analysis(fp_precision, "butter_df1_order8_cut1000_ba", "DF1", DF1_f, DF1_d, butter_df1_order8_cut1000_f32_ba[0], butter_df1_order8_cut1000_f32_ba[1], butter_df1_order8_cut1000_f64_ba[0], butter_df1_order8_cut1000_f64_ba[1], 9);
    precision_analysis(fp_precision, "butter_df2_order8_cut1000_ba", "DF2", DF2_f, DF2_d, butter_df2_order8_cut1000_f32_ba[0], butter_df2_order8_cut1000_f32_ba[1], butter_df2_order8_cut1000_f64_ba[0], butter_df2_order8_cut1000_f64_ba[1], 9);
    precision_analysis(fp_precision, "butter_tdf2_order8_cut1000_ba", "TDF2", TDF2_f, TDF2_d, butter_tdf2_order8_cut1000_f32_ba[0], butter_tdf2_order8_cut1000_f32_ba[1], butter_tdf2_order8_cut1000_f64_ba[0], butter_tdf2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_cascade(fp_precision, "butter_cascade_order8_cut1000_sos", CASCADE_f, CASCADE_d, (float*)butter_cascade_order8_cut1000_f32_sos, (double*)butter_cascade_order8_cut1000_f64_sos, 4);
    precision_analysis(fp_precision, "butter_df1_order8_cut2000_ba", "DF1", DF1_f, DF1_d, butter_df1_order8_cut2000_f32_ba[0], butter_df1_order8_cut2000_f32_ba[1], butter_df1_order8_cut2000_f64_ba[0], butter_df1_order8_cut2000_f64_ba[1], 9);
    precision_analysis(fp_precision, "butter_df2_order8_cut2000_ba", "DF2", DF2_f, DF2_d, butter_df2_order8_cut2000_f32_ba[0], butter_df2_order8_cut2000_f32_ba[1], butter_df2_order8_cut2000_f64_ba[0], butter_df2_order8_cut2000_f64_ba[1], 9);
    precision_analysis(fp_precision, "butter_tdf2_order8_cut2000_ba", "TDF2", TDF2_f, TDF2_d, butter_tdf2_order8_cut2000_f32_ba[0], butter_tdf2_order8_cut2000_f32_ba[1], butter_tdf2_order8_cut2000_f64_ba[0], butter_tdf2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_cascade(fp_precision, "butter_cascade_order8_cut2000_sos", CASCADE_f, CASCADE_d, (float*)butter_cascade_order8_cut2000_f32_sos, (double*)butter_cascade_order8_cut2000_f64_sos, 4);
    precision_analysis(fp_precision, "butter_df1_order8_cut5000_ba", "DF1", DF1_f, DF1_d, butter_df1_order8_cut5000_f32_ba[0], butter_df1_order8_cut5000_f32_ba[1], butter_df1_order8_cut5000_f64_ba[0], butter_df1_order8_cut5000_f64_ba[1], 9);
    precision_analysis(fp_precision, "butter_df2_order8_cut5000_ba", "DF2", DF2_f, DF2_d, butter_df2_order8_cut5000_f32_ba[0], butter_df2_order8_cut5000_f32_ba[1], butter_df2_order8_cut5000_f64_ba[0], butter_df2_order8_cut5000_f64_ba[1], 9);
    precision_analysis(fp_precision, "butter_tdf2_order8_cut5000_ba", "TDF2", TDF2_f, TDF2_d, butter_tdf2_order8_cut5000_f32_ba[0], butter_tdf2_order8_cut5000_f32_ba[1], butter_tdf2_order8_cut5000_f64_ba[0], butter_tdf2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_cascade(fp_precision, "butter_cascade_order8_cut5000_sos", CASCADE_f, CASCADE_d, (float*)butter_cascade_order8_cut5000_f32_sos, (double*)butter_cascade_order8_cut5000_f64_sos, 4);
    precision_analysis(fp_precision, "cheby1_df1_order2_cut1000_ba", "DF1", DF1_f, DF1_d, cheby1_df1_order2_cut1000_f32_ba[0], cheby1_df1_order2_cut1000_f32_ba[1], cheby1_df1_order2_cut1000_f64_ba[0], cheby1_df1_order2_cut1000_f64_ba[1], 3);
    precision_analysis(fp_precision, "cheby1_df2_order2_cut1000_ba", "DF2", DF2_f, DF2_d, cheby1_df2_order2_cut1000_f32_ba[0], cheby1_df2_order2_cut1000_f32_ba[1], cheby1_df2_order2_cut1000_f64_ba[0], cheby1_df2_order2_cut1000_f64_ba[1], 3);
    precision_analysis(fp_precision, "cheby1_tdf2_order2_cut1000_ba", "TDF2", TDF2_f, TDF2_d, cheby1_tdf2_order2_cut1000_f32_ba[0], cheby1_tdf2_order2_cut1000_f32_ba[1], cheby1_tdf2_order2_cut1000_f64_ba[0], cheby1_tdf2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_cascade(fp_precision, "cheby1_cascade_order2_cut1000_sos", CASCADE_f, CASCADE_d, (float*)cheby1_cascade_order2_cut1000_f32_sos, (double*)cheby1_cascade_order2_cut1000_f64_sos, 1);
    precision_analysis(fp_precision, "cheby1_df1_order2_cut2000_ba", "DF1", DF1_f, DF1_d, cheby1_df1_order2_cut2000_f32_ba[0], cheby1_df1_order2_cut2000_f32_ba[1], cheby1_df1_order2_cut2000_f64_ba[0], cheby1_df1_order2_cut2000_f64_ba[1], 3);
    precision_analysis(fp_precision, "cheby1_df2_order2_cut2000_ba", "DF2", DF2_f, DF2_d, cheby1_df2_order2_cut2000_f32_ba[0], cheby1_df2_order2_cut2000_f32_ba[1], cheby1_df2_order2_cut2000_f64_ba[0], cheby1_df2_order2_cut2000_f64_ba[1], 3);
    precision_analysis(fp_precision, "cheby1_tdf2_order2_cut2000_ba", "TDF2", TDF2_f, TDF2_d, cheby1_tdf2_order2_cut2000_f32_ba[0], cheby1_tdf2_order2_cut2000_f32_ba[1], cheby1_tdf2_order2_cut2000_f64_ba[0], cheby1_tdf2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_cascade(fp_precision, "cheby1_cascade_order2_cut2000_sos", CASCADE_f, CASCADE_d, (float*)cheby1_cascade_order2_cut2000_f32_sos, (double*)cheby1_cascade_order2_cut2000_f64_sos, 1);
    precision_analysis(fp_precision, "cheby1_df1_order2_cut5000_ba", "DF1", DF1_f, DF1_d, cheby1_df1_order2_cut5000_f32_ba[0], cheby1_df1_order2_cut5000_f32_ba[1], cheby1_df1_order2_cut5000_f64_ba[0], cheby1_df1_order2_cut5000_f64_ba[1], 3);
    precision_analysis(fp_precision, "cheby1_df2_order2_cut5000_ba", "DF2", DF2_f, DF2_d, cheby1_df2_order2_cut5000_f32_ba[0], cheby1_df2_order2_cut5000_f32_ba[1], cheby1_df2_order2_cut5000_f64_ba[0], cheby1_df2_order2_cut5000_f64_ba[1], 3);
    precision_analysis(fp_precision, "cheby1_tdf2_order2_cut5000_ba", "TDF2", TDF2_f, TDF2_d, cheby1_tdf2_order2_cut5000_f32_ba[0], cheby1_tdf2_order2_cut5000_f32_ba[1], cheby1_tdf2_order2_cut5000_f64_ba[0], cheby1_tdf2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_cascade(fp_precision, "cheby1_cascade_order2_cut5000_sos", CASCADE_f, CASCADE_d, (float*)cheby1_cascade_order2_cut5000_f32_sos, (double*)cheby1_cascade_order2_cut5000_f64_sos, 1);
    precision_analysis(fp_precision, "cheby1_df1_order4_cut1000_ba", "DF1", DF1_f, DF1_d, cheby1_df1_order4_cut1000_f32_ba[0], cheby1_df1_order4_cut1000_f32_ba[1], cheby1_df1_order4_cut1000_f64_ba[0], cheby1_df1_order4_cut1000_f64_ba[1], 5);
    precision_analysis(fp_precision, "cheby1_df2_order4_cut1000_ba", "DF2", DF2_f, DF2_d, cheby1_df2_order4_cut1000_f32_ba[0], cheby1_df2_order4_cut1000_f32_ba[1], cheby1_df2_order4_cut1000_f64_ba[0], cheby1_df2_order4_cut1000_f64_ba[1], 5);
    precision_analysis(fp_precision, "cheby1_tdf2_order4_cut1000_ba", "TDF2", TDF2_f, TDF2_d, cheby1_tdf2_order4_cut1000_f32_ba[0], cheby1_tdf2_order4_cut1000_f32_ba[1], cheby1_tdf2_order4_cut1000_f64_ba[0], cheby1_tdf2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_cascade(fp_precision, "cheby1_cascade_order4_cut1000_sos", CASCADE_f, CASCADE_d, (float*)cheby1_cascade_order4_cut1000_f32_sos, (double*)cheby1_cascade_order4_cut1000_f64_sos, 2);
    precision_analysis(fp_precision, "cheby1_df1_order4_cut2000_ba", "DF1", DF1_f, DF1_d, cheby1_df1_order4_cut2000_f32_ba[0], cheby1_df1_order4_cut2000_f32_ba[1], cheby1_df1_order4_cut2000_f64_ba[0], cheby1_df1_order4_cut2000_f64_ba[1], 5);
    precision_analysis(fp_precision, "cheby1_df2_order4_cut2000_ba", "DF2", DF2_f, DF2_d, cheby1_df2_order4_cut2000_f32_ba[0], cheby1_df2_order4_cut2000_f32_ba[1], cheby1_df2_order4_cut2000_f64_ba[0], cheby1_df2_order4_cut2000_f64_ba[1], 5);
    precision_analysis(fp_precision, "cheby1_tdf2_order4_cut2000_ba", "TDF2", TDF2_f, TDF2_d, cheby1_tdf2_order4_cut2000_f32_ba[0], cheby1_tdf2_order4_cut2000_f32_ba[1], cheby1_tdf2_order4_cut2000_f64_ba[0], cheby1_tdf2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_cascade(fp_precision, "cheby1_cascade_order4_cut2000_sos", CASCADE_f, CASCADE_d, (float*)cheby1_cascade_order4_cut2000_f32_sos, (double*)cheby1_cascade_order4_cut2000_f64_sos, 2);
    precision_analysis(fp_precision, "cheby1_df1_order4_cut5000_ba", "DF1", DF1_f, DF1_d, cheby1_df1_order4_cut5000_f32_ba[0], cheby1_df1_order4_cut5000_f32_ba[1], cheby1_df1_order4_cut5000_f64_ba[0], cheby1_df1_order4_cut5000_f64_ba[1], 5);
    precision_analysis(fp_precision, "cheby1_df2_order4_cut5000_ba", "DF2", DF2_f, DF2_d, cheby1_df2_order4_cut5000_f32_ba[0], cheby1_df2_order4_cut5000_f32_ba[1], cheby1_df2_order4_cut5000_f64_ba[0], cheby1_df2_order4_cut5000_f64_ba[1], 5);
    precision_analysis(fp_precision, "cheby1_tdf2_order4_cut5000_ba", "TDF2", TDF2_f, TDF2_d, cheby1_tdf2_order4_cut5000_f32_ba[0], cheby1_tdf2_order4_cut5000_f32_ba[1], cheby1_tdf2_order4_cut5000_f64_ba[0], cheby1_tdf2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_cascade(fp_precision, "cheby1_cascade_order4_cut5000_sos", CASCADE_f, CASCADE_d, (float*)cheby1_cascade_order4_cut5000_f32_sos, (double*)cheby1_cascade_order4_cut5000_f64_sos, 2);
    precision_analysis(fp_precision, "cheby1_df1_order6_cut1000_ba", "DF1", DF1_f, DF1_d, cheby1_df1_order6_cut1000_f32_ba[0], cheby1_df1_order6_cut1000_f32_ba[1], cheby1_df1_order6_cut1000_f64_ba[0], cheby1_df1_order6_cut1000_f64_ba[1], 7);
    precision_analysis(fp_precision, "cheby1_df2_order6_cut1000_ba", "DF2", DF2_f, DF2_d, cheby1_df2_order6_cut1000_f32_ba[0], cheby1_df2_order6_cut1000_f32_ba[1], cheby1_df2_order6_cut1000_f64_ba[0], cheby1_df2_order6_cut1000_f64_ba[1], 7);
    precision_analysis(fp_precision, "cheby1_tdf2_order6_cut1000_ba", "TDF2", TDF2_f, TDF2_d, cheby1_tdf2_order6_cut1000_f32_ba[0], cheby1_tdf2_order6_cut1000_f32_ba[1], cheby1_tdf2_order6_cut1000_f64_ba[0], cheby1_tdf2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_cascade(fp_precision, "cheby1_cascade_order6_cut1000_sos", CASCADE_f, CASCADE_d, (float*)cheby1_cascade_order6_cut1000_f32_sos, (double*)cheby1_cascade_order6_cut1000_f64_sos, 3);
    precision_analysis(fp_precision, "cheby1_df1_order6_cut2000_ba", "DF1", DF1_f, DF1_d, cheby1_df1_order6_cut2000_f32_ba[0], cheby1_df1_order6_cut2000_f32_ba[1], cheby1_df1_order6_cut2000_f64_ba[0], cheby1_df1_order6_cut2000_f64_ba[1], 7);
    precision_analysis(fp_precision, "cheby1_df2_order6_cut2000_ba", "DF2", DF2_f, DF2_d, cheby1_df2_order6_cut2000_f32_ba[0], cheby1_df2_order6_cut2000_f32_ba[1], cheby1_df2_order6_cut2000_f64_ba[0], cheby1_df2_order6_cut2000_f64_ba[1], 7);
    precision_analysis(fp_precision, "cheby1_tdf2_order6_cut2000_ba", "TDF2", TDF2_f, TDF2_d, cheby1_tdf2_order6_cut2000_f32_ba[0], cheby1_tdf2_order6_cut2000_f32_ba[1], cheby1_tdf2_order6_cut2000_f64_ba[0], cheby1_tdf2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_cascade(fp_precision, "cheby1_cascade_order6_cut2000_sos", CASCADE_f, CASCADE_d, (float*)cheby1_cascade_order6_cut2000_f32_sos, (double*)cheby1_cascade_order6_cut2000_f64_sos, 3);
    precision_analysis(fp_precision, "cheby1_df1_order6_cut5000_ba", "DF1", DF1_f, DF1_d, cheby1_df1_order6_cut5000_f32_ba[0], cheby1_df1_order6_cut5000_f32_ba[1], cheby1_df1_order6_cut5000_f64_ba[0], cheby1_df1_order6_cut5000_f64_ba[1], 7);
    precision_analysis(fp_precision, "cheby1_df2_order6_cut5000_ba", "DF2", DF2_f, DF2_d, cheby1_df2_order6_cut5000_f32_ba[0], cheby1_df2_order6_cut5000_f32_ba[1], cheby1_df2_order6_cut5000_f64_ba[0], cheby1_df2_order6_cut5000_f64_ba[1], 7);
    precision_analysis(fp_precision, "cheby1_tdf2_order6_cut5000_ba", "TDF2", TDF2_f, TDF2_d, cheby1_tdf2_order6_cut5000_f32_ba[0], cheby1_tdf2_order6_cut5000_f32_ba[1], cheby1_tdf2_order6_cut5000_f64_ba[0], cheby1_tdf2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_cascade(fp_precision, "cheby1_cascade_order6_cut5000_sos", CASCADE_f, CASCADE_d, (float*)cheby1_cascade_order6_cut5000_f32_sos, (double*)cheby1_cascade_order6_cut5000_f64_sos, 3);
    precision_analysis(fp_precision, "cheby1_df1_order8_cut1000_ba", "DF1", DF1_f, DF1_d, cheby1_df1_order8_cut1000_f32_ba[0], cheby1_df1_order8_cut1000_f32_ba[1], cheby1_df1_order8_cut1000_f64_ba[0], cheby1_df1_order8_cut1000_f64_ba[1], 9);
    precision_analysis(fp_precision, "cheby1_df2_order8_cut1000_ba", "DF2", DF2_f, DF2_d, cheby1_df2_order8_cut1000_f32_ba[0], cheby1_df2_order8_cut1000_f32_ba[1], cheby1_df2_order8_cut1000_f64_ba[0], cheby1_df2_order8_cut1000_f64_ba[1], 9);
    precision_analysis(fp_precision, "cheby1_tdf2_order8_cut1000_ba", "TDF2", TDF2_f, TDF2_d, cheby1_tdf2_order8_cut1000_f32_ba[0], cheby1_tdf2_order8_cut1000_f32_ba[1], cheby1_tdf2_order8_cut1000_f64_ba[0], cheby1_tdf2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_cascade(fp_precision, "cheby1_cascade_order8_cut1000_sos", CASCADE_f, CASCADE_d, (float*)cheby1_cascade_order8_cut1000_f32_sos, (double*)cheby1_cascade_order8_cut1000_f64_sos, 4);
    precision_analysis(fp_precision, "cheby1_df1_order8_cut2000_ba", "DF1", DF1_f, DF1_d, cheby1_df1_order8_cut2000_f32_ba[0], cheby1_df1_order8_cut2000_f32_ba[1], cheby1_df1_order8_cut2000_f64_ba[0], cheby1_df1_order8_cut2000_f64_ba[1], 9);
    precision_analysis(fp_precision, "cheby1_df2_order8_cut2000_ba", "DF2", DF2_f, DF2_d, cheby1_df2_order8_cut2000_f32_ba[0], cheby1_df2_order8_cut2000_f32_ba[1], cheby1_df2_order8_cut2000_f64_ba[0], cheby1_df2_order8_cut2000_f64_ba[1], 9);
    precision_analysis(fp_precision, "cheby1_tdf2_order8_cut2000_ba", "TDF2", TDF2_f, TDF2_d, cheby1_tdf2_order8_cut2000_f32_ba[0], cheby1_tdf2_order8_cut2000_f32_ba[1], cheby1_tdf2_order8_cut2000_f64_ba[0], cheby1_tdf2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_cascade(fp_precision, "cheby1_cascade_order8_cut2000_sos", CASCADE_f, CASCADE_d, (float*)cheby1_cascade_order8_cut2000_f32_sos, (double*)cheby1_cascade_order8_cut2000_f64_sos, 4);
    precision_analysis(fp_precision, "cheby1_df1_order8_cut5000_ba", "DF1", DF1_f, DF1_d, cheby1_df1_order8_cut5000_f32_ba[0], cheby1_df1_order8_cut5000_f32_ba[1], cheby1_df1_order8_cut5000_f64_ba[0], cheby1_df1_order8_cut5000_f64_ba[1], 9);
    precision_analysis(fp_precision, "cheby1_df2_order8_cut5000_ba", "DF2", DF2_f, DF2_d, cheby1_df2_order8_cut5000_f32_ba[0], cheby1_df2_order8_cut5000_f32_ba[1], cheby1_df2_order8_cut5000_f64_ba[0], cheby1_df2_order8_cut5000_f64_ba[1], 9);
    precision_analysis(fp_precision, "cheby1_tdf2_order8_cut5000_ba", "TDF2", TDF2_f, TDF2_d, cheby1_tdf2_order8_cut5000_f32_ba[0], cheby1_tdf2_order8_cut5000_f32_ba[1], cheby1_tdf2_order8_cut5000_f64_ba[0], cheby1_tdf2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_cascade(fp_precision, "cheby1_cascade_order8_cut5000_sos", CASCADE_f, CASCADE_d, (float*)cheby1_cascade_order8_cut5000_f32_sos, (double*)cheby1_cascade_order8_cut5000_f64_sos, 4);
    precision_analysis(fp_precision, "cheby2_df1_order2_cut1000_ba", "DF1", DF1_f, DF1_d, cheby2_df1_order2_cut1000_f32_ba[0], cheby2_df1_order2_cut1000_f32_ba[1], cheby2_df1_order2_cut1000_f64_ba[0], cheby2_df1_order2_cut1000_f64_ba[1], 3);
    precision_analysis(fp_precision, "cheby2_df2_order2_cut1000_ba", "DF2", DF2_f, DF2_d, cheby2_df2_order2_cut1000_f32_ba[0], cheby2_df2_order2_cut1000_f32_ba[1], cheby2_df2_order2_cut1000_f64_ba[0], cheby2_df2_order2_cut1000_f64_ba[1], 3);
    precision_analysis(fp_precision, "cheby2_tdf2_order2_cut1000_ba", "TDF2", TDF2_f, TDF2_d, cheby2_tdf2_order2_cut1000_f32_ba[0], cheby2_tdf2_order2_cut1000_f32_ba[1], cheby2_tdf2_order2_cut1000_f64_ba[0], cheby2_tdf2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_cascade(fp_precision, "cheby2_cascade_order2_cut1000_sos", CASCADE_f, CASCADE_d, (float*)cheby2_cascade_order2_cut1000_f32_sos, (double*)cheby2_cascade_order2_cut1000_f64_sos, 1);
    precision_analysis(fp_precision, "cheby2_df1_order2_cut2000_ba", "DF1", DF1_f, DF1_d, cheby2_df1_order2_cut2000_f32_ba[0], cheby2_df1_order2_cut2000_f32_ba[1], cheby2_df1_order2_cut2000_f64_ba[0], cheby2_df1_order2_cut2000_f64_ba[1], 3);
    precision_analysis(fp_precision, "cheby2_df2_order2_cut2000_ba", "DF2", DF2_f, DF2_d, cheby2_df2_order2_cut2000_f32_ba[0], cheby2_df2_order2_cut2000_f32_ba[1], cheby2_df2_order2_cut2000_f64_ba[0], cheby2_df2_order2_cut2000_f64_ba[1], 3);
    precision_analysis(fp_precision, "cheby2_tdf2_order2_cut2000_ba", "TDF2", TDF2_f, TDF2_d, cheby2_tdf2_order2_cut2000_f32_ba[0], cheby2_tdf2_order2_cut2000_f32_ba[1], cheby2_tdf2_order2_cut2000_f64_ba[0], cheby2_tdf2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_cascade(fp_precision, "cheby2_cascade_order2_cut2000_sos", CASCADE_f, CASCADE_d, (float*)cheby2_cascade_order2_cut2000_f32_sos, (double*)cheby2_cascade_order2_cut2000_f64_sos, 1);
    precision_analysis(fp_precision, "cheby2_df1_order2_cut5000_ba", "DF1", DF1_f, DF1_d, cheby2_df1_order2_cut5000_f32_ba[0], cheby2_df1_order2_cut5000_f32_ba[1], cheby2_df1_order2_cut5000_f64_ba[0], cheby2_df1_order2_cut5000_f64_ba[1], 3);
    precision_analysis(fp_precision, "cheby2_df2_order2_cut5000_ba", "DF2", DF2_f, DF2_d, cheby2_df2_order2_cut5000_f32_ba[0], cheby2_df2_order2_cut5000_f32_ba[1], cheby2_df2_order2_cut5000_f64_ba[0], cheby2_df2_order2_cut5000_f64_ba[1], 3);
    precision_analysis(fp_precision, "cheby2_tdf2_order2_cut5000_ba", "TDF2", TDF2_f, TDF2_d, cheby2_tdf2_order2_cut5000_f32_ba[0], cheby2_tdf2_order2_cut5000_f32_ba[1], cheby2_tdf2_order2_cut5000_f64_ba[0], cheby2_tdf2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_cascade(fp_precision, "cheby2_cascade_order2_cut5000_sos", CASCADE_f, CASCADE_d, (float*)cheby2_cascade_order2_cut5000_f32_sos, (double*)cheby2_cascade_order2_cut5000_f64_sos, 1);
    precision_analysis(fp_precision, "cheby2_df1_order4_cut1000_ba", "DF1", DF1_f, DF1_d, cheby2_df1_order4_cut1000_f32_ba[0], cheby2_df1_order4_cut1000_f32_ba[1], cheby2_df1_order4_cut1000_f64_ba[0], cheby2_df1_order4_cut1000_f64_ba[1], 5);
    precision_analysis(fp_precision, "cheby2_df2_order4_cut1000_ba", "DF2", DF2_f, DF2_d, cheby2_df2_order4_cut1000_f32_ba[0], cheby2_df2_order4_cut1000_f32_ba[1], cheby2_df2_order4_cut1000_f64_ba[0], cheby2_df2_order4_cut1000_f64_ba[1], 5);
    precision_analysis(fp_precision, "cheby2_tdf2_order4_cut1000_ba", "TDF2", TDF2_f, TDF2_d, cheby2_tdf2_order4_cut1000_f32_ba[0], cheby2_tdf2_order4_cut1000_f32_ba[1], cheby2_tdf2_order4_cut1000_f64_ba[0], cheby2_tdf2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_cascade(fp_precision, "cheby2_cascade_order4_cut1000_sos", CASCADE_f, CASCADE_d, (float*)cheby2_cascade_order4_cut1000_f32_sos, (double*)cheby2_cascade_order4_cut1000_f64_sos, 2);
    precision_analysis(fp_precision, "cheby2_df1_order4_cut2000_ba", "DF1", DF1_f, DF1_d, cheby2_df1_order4_cut2000_f32_ba[0], cheby2_df1_order4_cut2000_f32_ba[1], cheby2_df1_order4_cut2000_f64_ba[0], cheby2_df1_order4_cut2000_f64_ba[1], 5);
    precision_analysis(fp_precision, "cheby2_df2_order4_cut2000_ba", "DF2", DF2_f, DF2_d, cheby2_df2_order4_cut2000_f32_ba[0], cheby2_df2_order4_cut2000_f32_ba[1], cheby2_df2_order4_cut2000_f64_ba[0], cheby2_df2_order4_cut2000_f64_ba[1], 5);
    precision_analysis(fp_precision, "cheby2_tdf2_order4_cut2000_ba", "TDF2", TDF2_f, TDF2_d, cheby2_tdf2_order4_cut2000_f32_ba[0], cheby2_tdf2_order4_cut2000_f32_ba[1], cheby2_tdf2_order4_cut2000_f64_ba[0], cheby2_tdf2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_cascade(fp_precision, "cheby2_cascade_order4_cut2000_sos", CASCADE_f, CASCADE_d, (float*)cheby2_cascade_order4_cut2000_f32_sos, (double*)cheby2_cascade_order4_cut2000_f64_sos, 2);
    precision_analysis(fp_precision, "cheby2_df1_order4_cut5000_ba", "DF1", DF1_f, DF1_d, cheby2_df1_order4_cut5000_f32_ba[0], cheby2_df1_order4_cut5000_f32_ba[1], cheby2_df1_order4_cut5000_f64_ba[0], cheby2_df1_order4_cut5000_f64_ba[1], 5);
    precision_analysis(fp_precision, "cheby2_df2_order4_cut5000_ba", "DF2", DF2_f, DF2_d, cheby2_df2_order4_cut5000_f32_ba[0], cheby2_df2_order4_cut5000_f32_ba[1], cheby2_df2_order4_cut5000_f64_ba[0], cheby2_df2_order4_cut5000_f64_ba[1], 5);
    precision_analysis(fp_precision, "cheby2_tdf2_order4_cut5000_ba", "TDF2", TDF2_f, TDF2_d, cheby2_tdf2_order4_cut5000_f32_ba[0], cheby2_tdf2_order4_cut5000_f32_ba[1], cheby2_tdf2_order4_cut5000_f64_ba[0], cheby2_tdf2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_cascade(fp_precision, "cheby2_cascade_order4_cut5000_sos", CASCADE_f, CASCADE_d, (float*)cheby2_cascade_order4_cut5000_f32_sos, (double*)cheby2_cascade_order4_cut5000_f64_sos, 2);
    precision_analysis(fp_precision, "cheby2_df1_order6_cut1000_ba", "DF1", DF1_f, DF1_d, cheby2_df1_order6_cut1000_f32_ba[0], cheby2_df1_order6_cut1000_f32_ba[1], cheby2_df1_order6_cut1000_f64_ba[0], cheby2_df1_order6_cut1000_f64_ba[1], 7);
    precision_analysis(fp_precision, "cheby2_df2_order6_cut1000_ba", "DF2", DF2_f, DF2_d, cheby2_df2_order6_cut1000_f32_ba[0], cheby2_df2_order6_cut1000_f32_ba[1], cheby2_df2_order6_cut1000_f64_ba[0], cheby2_df2_order6_cut1000_f64_ba[1], 7);
    precision_analysis(fp_precision, "cheby2_tdf2_order6_cut1000_ba", "TDF2", TDF2_f, TDF2_d, cheby2_tdf2_order6_cut1000_f32_ba[0], cheby2_tdf2_order6_cut1000_f32_ba[1], cheby2_tdf2_order6_cut1000_f64_ba[0], cheby2_tdf2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_cascade(fp_precision, "cheby2_cascade_order6_cut1000_sos", CASCADE_f, CASCADE_d, (float*)cheby2_cascade_order6_cut1000_f32_sos, (double*)cheby2_cascade_order6_cut1000_f64_sos, 3);
    precision_analysis(fp_precision, "cheby2_df1_order6_cut2000_ba", "DF1", DF1_f, DF1_d, cheby2_df1_order6_cut2000_f32_ba[0], cheby2_df1_order6_cut2000_f32_ba[1], cheby2_df1_order6_cut2000_f64_ba[0], cheby2_df1_order6_cut2000_f64_ba[1], 7);
    precision_analysis(fp_precision, "cheby2_df2_order6_cut2000_ba", "DF2", DF2_f, DF2_d, cheby2_df2_order6_cut2000_f32_ba[0], cheby2_df2_order6_cut2000_f32_ba[1], cheby2_df2_order6_cut2000_f64_ba[0], cheby2_df2_order6_cut2000_f64_ba[1], 7);
    precision_analysis(fp_precision, "cheby2_tdf2_order6_cut2000_ba", "TDF2", TDF2_f, TDF2_d, cheby2_tdf2_order6_cut2000_f32_ba[0], cheby2_tdf2_order6_cut2000_f32_ba[1], cheby2_tdf2_order6_cut2000_f64_ba[0], cheby2_tdf2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_cascade(fp_precision, "cheby2_cascade_order6_cut2000_sos", CASCADE_f, CASCADE_d, (float*)cheby2_cascade_order6_cut2000_f32_sos, (double*)cheby2_cascade_order6_cut2000_f64_sos, 3);
    precision_analysis(fp_precision, "cheby2_df1_order6_cut5000_ba", "DF1", DF1_f, DF1_d, cheby2_df1_order6_cut5000_f32_ba[0], cheby2_df1_order6_cut5000_f32_ba[1], cheby2_df1_order6_cut5000_f64_ba[0], cheby2_df1_order6_cut5000_f64_ba[1], 7);
    precision_analysis(fp_precision, "cheby2_df2_order6_cut5000_ba", "DF2", DF2_f, DF2_d, cheby2_df2_order6_cut5000_f32_ba[0], cheby2_df2_order6_cut5000_f32_ba[1], cheby2_df2_order6_cut5000_f64_ba[0], cheby2_df2_order6_cut5000_f64_ba[1], 7);
    precision_analysis(fp_precision, "cheby2_tdf2_order6_cut5000_ba", "TDF2", TDF2_f, TDF2_d, cheby2_tdf2_order6_cut5000_f32_ba[0], cheby2_tdf2_order6_cut5000_f32_ba[1], cheby2_tdf2_order6_cut5000_f64_ba[0], cheby2_tdf2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_cascade(fp_precision, "cheby2_cascade_order6_cut5000_sos", CASCADE_f, CASCADE_d, (float*)cheby2_cascade_order6_cut5000_f32_sos, (double*)cheby2_cascade_order6_cut5000_f64_sos, 3);
    precision_analysis(fp_precision, "cheby2_df1_order8_cut1000_ba", "DF1", DF1_f, DF1_d, cheby2_df1_order8_cut1000_f32_ba[0], cheby2_df1_order8_cut1000_f32_ba[1], cheby2_df1_order8_cut1000_f64_ba[0], cheby2_df1_order8_cut1000_f64_ba[1], 9);
    precision_analysis(fp_precision, "cheby2_df2_order8_cut1000_ba", "DF2", DF2_f, DF2_d, cheby2_df2_order8_cut1000_f32_ba[0], cheby2_df2_order8_cut1000_f32_ba[1], cheby2_df2_order8_cut1000_f64_ba[0], cheby2_df2_order8_cut1000_f64_ba[1], 9);
    precision_analysis(fp_precision, "cheby2_tdf2_order8_cut1000_ba", "TDF2", TDF2_f, TDF2_d, cheby2_tdf2_order8_cut1000_f32_ba[0], cheby2_tdf2_order8_cut1000_f32_ba[1], cheby2_tdf2_order8_cut1000_f64_ba[0], cheby2_tdf2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_cascade(fp_precision, "cheby2_cascade_order8_cut1000_sos", CASCADE_f, CASCADE_d, (float*)cheby2_cascade_order8_cut1000_f32_sos, (double*)cheby2_cascade_order8_cut1000_f64_sos, 4);
    precision_analysis(fp_precision, "cheby2_df1_order8_cut2000_ba", "DF1", DF1_f, DF1_d, cheby2_df1_order8_cut2000_f32_ba[0], cheby2_df1_order8_cut2000_f32_ba[1], cheby2_df1_order8_cut2000_f64_ba[0], cheby2_df1_order8_cut2000_f64_ba[1], 9);
    precision_analysis(fp_precision, "cheby2_df2_order8_cut2000_ba", "DF2", DF2_f, DF2_d, cheby2_df2_order8_cut2000_f32_ba[0], cheby2_df2_order8_cut2000_f32_ba[1], cheby2_df2_order8_cut2000_f64_ba[0], cheby2_df2_order8_cut2000_f64_ba[1], 9);
    precision_analysis(fp_precision, "cheby2_tdf2_order8_cut2000_ba", "TDF2", TDF2_f, TDF2_d, cheby2_tdf2_order8_cut2000_f32_ba[0], cheby2_tdf2_order8_cut2000_f32_ba[1], cheby2_tdf2_order8_cut2000_f64_ba[0], cheby2_tdf2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_cascade(fp_precision, "cheby2_cascade_order8_cut2000_sos", CASCADE_f, CASCADE_d, (float*)cheby2_cascade_order8_cut2000_f32_sos, (double*)cheby2_cascade_order8_cut2000_f64_sos, 4);
    precision_analysis(fp_precision, "cheby2_df1_order8_cut5000_ba", "DF1", DF1_f, DF1_d, cheby2_df1_order8_cut5000_f32_ba[0], cheby2_df1_order8_cut5000_f32_ba[1], cheby2_df1_order8_cut5000_f64_ba[0], cheby2_df1_order8_cut5000_f64_ba[1], 9);
    precision_analysis(fp_precision, "cheby2_df2_order8_cut5000_ba", "DF2", DF2_f, DF2_d, cheby2_df2_order8_cut5000_f32_ba[0], cheby2_df2_order8_cut5000_f32_ba[1], cheby2_df2_order8_cut5000_f64_ba[0], cheby2_df2_order8_cut5000_f64_ba[1], 9);
    precision_analysis(fp_precision, "cheby2_tdf2_order8_cut5000_ba", "TDF2", TDF2_f, TDF2_d, cheby2_tdf2_order8_cut5000_f32_ba[0], cheby2_tdf2_order8_cut5000_f32_ba[1], cheby2_tdf2_order8_cut5000_f64_ba[0], cheby2_tdf2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_cascade(fp_precision, "cheby2_cascade_order8_cut5000_sos", CASCADE_f, CASCADE_d, (float*)cheby2_cascade_order8_cut5000_f32_sos, (double*)cheby2_cascade_order8_cut5000_f64_sos, 4);
    precision_analysis(fp_precision, "ellip_df1_order2_cut1000_ba", "DF1", DF1_f, DF1_d, ellip_df1_order2_cut1000_f32_ba[0], ellip_df1_order2_cut1000_f32_ba[1], ellip_df1_order2_cut1000_f64_ba[0], ellip_df1_order2_cut1000_f64_ba[1], 3);
    precision_analysis(fp_precision, "ellip_df2_order2_cut1000_ba", "DF2", DF2_f, DF2_d, ellip_df2_order2_cut1000_f32_ba[0], ellip_df2_order2_cut1000_f32_ba[1], ellip_df2_order2_cut1000_f64_ba[0], ellip_df2_order2_cut1000_f64_ba[1], 3);
    precision_analysis(fp_precision, "ellip_tdf2_order2_cut1000_ba", "TDF2", TDF2_f, TDF2_d, ellip_tdf2_order2_cut1000_f32_ba[0], ellip_tdf2_order2_cut1000_f32_ba[1], ellip_tdf2_order2_cut1000_f64_ba[0], ellip_tdf2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_cascade(fp_precision, "ellip_cascade_order2_cut1000_sos", CASCADE_f, CASCADE_d, (float*)ellip_cascade_order2_cut1000_f32_sos, (double*)ellip_cascade_order2_cut1000_f64_sos, 1);
    precision_analysis(fp_precision, "ellip_df1_order2_cut2000_ba", "DF1", DF1_f, DF1_d, ellip_df1_order2_cut2000_f32_ba[0], ellip_df1_order2_cut2000_f32_ba[1], ellip_df1_order2_cut2000_f64_ba[0], ellip_df1_order2_cut2000_f64_ba[1], 3);
    precision_analysis(fp_precision, "ellip_df2_order2_cut2000_ba", "DF2", DF2_f, DF2_d, ellip_df2_order2_cut2000_f32_ba[0], ellip_df2_order2_cut2000_f32_ba[1], ellip_df2_order2_cut2000_f64_ba[0], ellip_df2_order2_cut2000_f64_ba[1], 3);
    precision_analysis(fp_precision, "ellip_tdf2_order2_cut2000_ba", "TDF2", TDF2_f, TDF2_d, ellip_tdf2_order2_cut2000_f32_ba[0], ellip_tdf2_order2_cut2000_f32_ba[1], ellip_tdf2_order2_cut2000_f64_ba[0], ellip_tdf2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_cascade(fp_precision, "ellip_cascade_order2_cut2000_sos", CASCADE_f, CASCADE_d, (float*)ellip_cascade_order2_cut2000_f32_sos, (double*)ellip_cascade_order2_cut2000_f64_sos, 1);
    precision_analysis(fp_precision, "ellip_df1_order2_cut5000_ba", "DF1", DF1_f, DF1_d, ellip_df1_order2_cut5000_f32_ba[0], ellip_df1_order2_cut5000_f32_ba[1], ellip_df1_order2_cut5000_f64_ba[0], ellip_df1_order2_cut5000_f64_ba[1], 3);
    precision_analysis(fp_precision, "ellip_df2_order2_cut5000_ba", "DF2", DF2_f, DF2_d, ellip_df2_order2_cut5000_f32_ba[0], ellip_df2_order2_cut5000_f32_ba[1], ellip_df2_order2_cut5000_f64_ba[0], ellip_df2_order2_cut5000_f64_ba[1], 3);
    precision_analysis(fp_precision, "ellip_tdf2_order2_cut5000_ba", "TDF2", TDF2_f, TDF2_d, ellip_tdf2_order2_cut5000_f32_ba[0], ellip_tdf2_order2_cut5000_f32_ba[1], ellip_tdf2_order2_cut5000_f64_ba[0], ellip_tdf2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_cascade(fp_precision, "ellip_cascade_order2_cut5000_sos", CASCADE_f, CASCADE_d, (float*)ellip_cascade_order2_cut5000_f32_sos, (double*)ellip_cascade_order2_cut5000_f64_sos, 1);
    precision_analysis(fp_precision, "ellip_df1_order4_cut1000_ba", "DF1", DF1_f, DF1_d, ellip_df1_order4_cut1000_f32_ba[0], ellip_df1_order4_cut1000_f32_ba[1], ellip_df1_order4_cut1000_f64_ba[0], ellip_df1_order4_cut1000_f64_ba[1], 5);
    precision_analysis(fp_precision, "ellip_df2_order4_cut1000_ba", "DF2", DF2_f, DF2_d, ellip_df2_order4_cut1000_f32_ba[0], ellip_df2_order4_cut1000_f32_ba[1], ellip_df2_order4_cut1000_f64_ba[0], ellip_df2_order4_cut1000_f64_ba[1], 5);
    precision_analysis(fp_precision, "ellip_tdf2_order4_cut1000_ba", "TDF2", TDF2_f, TDF2_d, ellip_tdf2_order4_cut1000_f32_ba[0], ellip_tdf2_order4_cut1000_f32_ba[1], ellip_tdf2_order4_cut1000_f64_ba[0], ellip_tdf2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_cascade(fp_precision, "ellip_cascade_order4_cut1000_sos", CASCADE_f, CASCADE_d, (float*)ellip_cascade_order4_cut1000_f32_sos, (double*)ellip_cascade_order4_cut1000_f64_sos, 2);
    precision_analysis(fp_precision, "ellip_df1_order4_cut2000_ba", "DF1", DF1_f, DF1_d, ellip_df1_order4_cut2000_f32_ba[0], ellip_df1_order4_cut2000_f32_ba[1], ellip_df1_order4_cut2000_f64_ba[0], ellip_df1_order4_cut2000_f64_ba[1], 5);
    precision_analysis(fp_precision, "ellip_df2_order4_cut2000_ba", "DF2", DF2_f, DF2_d, ellip_df2_order4_cut2000_f32_ba[0], ellip_df2_order4_cut2000_f32_ba[1], ellip_df2_order4_cut2000_f64_ba[0], ellip_df2_order4_cut2000_f64_ba[1], 5);
    precision_analysis(fp_precision, "ellip_tdf2_order4_cut2000_ba", "TDF2", TDF2_f, TDF2_d, ellip_tdf2_order4_cut2000_f32_ba[0], ellip_tdf2_order4_cut2000_f32_ba[1], ellip_tdf2_order4_cut2000_f64_ba[0], ellip_tdf2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_cascade(fp_precision, "ellip_cascade_order4_cut2000_sos", CASCADE_f, CASCADE_d, (float*)ellip_cascade_order4_cut2000_f32_sos, (double*)ellip_cascade_order4_cut2000_f64_sos, 2);
    precision_analysis(fp_precision, "ellip_df1_order4_cut5000_ba", "DF1", DF1_f, DF1_d, ellip_df1_order4_cut5000_f32_ba[0], ellip_df1_order4_cut5000_f32_ba[1], ellip_df1_order4_cut5000_f64_ba[0], ellip_df1_order4_cut5000_f64_ba[1], 5);
    precision_analysis(fp_precision, "ellip_df2_order4_cut5000_ba", "DF2", DF2_f, DF2_d, ellip_df2_order4_cut5000_f32_ba[0], ellip_df2_order4_cut5000_f32_ba[1], ellip_df2_order4_cut5000_f64_ba[0], ellip_df2_order4_cut5000_f64_ba[1], 5);
    precision_analysis(fp_precision, "ellip_tdf2_order4_cut5000_ba", "TDF2", TDF2_f, TDF2_d, ellip_tdf2_order4_cut5000_f32_ba[0], ellip_tdf2_order4_cut5000_f32_ba[1], ellip_tdf2_order4_cut5000_f64_ba[0], ellip_tdf2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_cascade(fp_precision, "ellip_cascade_order4_cut5000_sos", CASCADE_f, CASCADE_d, (float*)ellip_cascade_order4_cut5000_f32_sos, (double*)ellip_cascade_order4_cut5000_f64_sos, 2);
    precision_analysis(fp_precision, "ellip_df1_order6_cut1000_ba", "DF1", DF1_f, DF1_d, ellip_df1_order6_cut1000_f32_ba[0], ellip_df1_order6_cut1000_f32_ba[1], ellip_df1_order6_cut1000_f64_ba[0], ellip_df1_order6_cut1000_f64_ba[1], 7);
    precision_analysis(fp_precision, "ellip_df2_order6_cut1000_ba", "DF2", DF2_f, DF2_d, ellip_df2_order6_cut1000_f32_ba[0], ellip_df2_order6_cut1000_f32_ba[1], ellip_df2_order6_cut1000_f64_ba[0], ellip_df2_order6_cut1000_f64_ba[1], 7);
    precision_analysis(fp_precision, "ellip_tdf2_order6_cut1000_ba", "TDF2", TDF2_f, TDF2_d, ellip_tdf2_order6_cut1000_f32_ba[0], ellip_tdf2_order6_cut1000_f32_ba[1], ellip_tdf2_order6_cut1000_f64_ba[0], ellip_tdf2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_cascade(fp_precision, "ellip_cascade_order6_cut1000_sos", CASCADE_f, CASCADE_d, (float*)ellip_cascade_order6_cut1000_f32_sos, (double*)ellip_cascade_order6_cut1000_f64_sos, 3);
    precision_analysis(fp_precision, "ellip_df1_order6_cut2000_ba", "DF1", DF1_f, DF1_d, ellip_df1_order6_cut2000_f32_ba[0], ellip_df1_order6_cut2000_f32_ba[1], ellip_df1_order6_cut2000_f64_ba[0], ellip_df1_order6_cut2000_f64_ba[1], 7);
    precision_analysis(fp_precision, "ellip_df2_order6_cut2000_ba", "DF2", DF2_f, DF2_d, ellip_df2_order6_cut2000_f32_ba[0], ellip_df2_order6_cut2000_f32_ba[1], ellip_df2_order6_cut2000_f64_ba[0], ellip_df2_order6_cut2000_f64_ba[1], 7);
    precision_analysis(fp_precision, "ellip_tdf2_order6_cut2000_ba", "TDF2", TDF2_f, TDF2_d, ellip_tdf2_order6_cut2000_f32_ba[0], ellip_tdf2_order6_cut2000_f32_ba[1], ellip_tdf2_order6_cut2000_f64_ba[0], ellip_tdf2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_cascade(fp_precision, "ellip_cascade_order6_cut2000_sos", CASCADE_f, CASCADE_d, (float*)ellip_cascade_order6_cut2000_f32_sos, (double*)ellip_cascade_order6_cut2000_f64_sos, 3);
    precision_analysis(fp_precision, "ellip_df1_order6_cut5000_ba", "DF1", DF1_f, DF1_d, ellip_df1_order6_cut5000_f32_ba[0], ellip_df1_order6_cut5000_f32_ba[1], ellip_df1_order6_cut5000_f64_ba[0], ellip_df1_order6_cut5000_f64_ba[1], 7);
    precision_analysis(fp_precision, "ellip_df2_order6_cut5000_ba", "DF2", DF2_f, DF2_d, ellip_df2_order6_cut5000_f32_ba[0], ellip_df2_order6_cut5000_f32_ba[1], ellip_df2_order6_cut5000_f64_ba[0], ellip_df2_order6_cut5000_f64_ba[1], 7);
    precision_analysis(fp_precision, "ellip_tdf2_order6_cut5000_ba", "TDF2", TDF2_f, TDF2_d, ellip_tdf2_order6_cut5000_f32_ba[0], ellip_tdf2_order6_cut5000_f32_ba[1], ellip_tdf2_order6_cut5000_f64_ba[0], ellip_tdf2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_cascade(fp_precision, "ellip_cascade_order6_cut5000_sos", CASCADE_f, CASCADE_d, (float*)ellip_cascade_order6_cut5000_f32_sos, (double*)ellip_cascade_order6_cut5000_f64_sos, 3);
    precision_analysis(fp_precision, "ellip_df1_order8_cut1000_ba", "DF1", DF1_f, DF1_d, ellip_df1_order8_cut1000_f32_ba[0], ellip_df1_order8_cut1000_f32_ba[1], ellip_df1_order8_cut1000_f64_ba[0], ellip_df1_order8_cut1000_f64_ba[1], 9);
    precision_analysis(fp_precision, "ellip_df2_order8_cut1000_ba", "DF2", DF2_f, DF2_d, ellip_df2_order8_cut1000_f32_ba[0], ellip_df2_order8_cut1000_f32_ba[1], ellip_df2_order8_cut1000_f64_ba[0], ellip_df2_order8_cut1000_f64_ba[1], 9);
    precision_analysis(fp_precision, "ellip_tdf2_order8_cut1000_ba", "TDF2", TDF2_f, TDF2_d, ellip_tdf2_order8_cut1000_f32_ba[0], ellip_tdf2_order8_cut1000_f32_ba[1], ellip_tdf2_order8_cut1000_f64_ba[0], ellip_tdf2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_cascade(fp_precision, "ellip_cascade_order8_cut1000_sos", CASCADE_f, CASCADE_d, (float*)ellip_cascade_order8_cut1000_f32_sos, (double*)ellip_cascade_order8_cut1000_f64_sos, 4);
    precision_analysis(fp_precision, "ellip_df1_order8_cut2000_ba", "DF1", DF1_f, DF1_d, ellip_df1_order8_cut2000_f32_ba[0], ellip_df1_order8_cut2000_f32_ba[1], ellip_df1_order8_cut2000_f64_ba[0], ellip_df1_order8_cut2000_f64_ba[1], 9);
    precision_analysis(fp_precision, "ellip_df2_order8_cut2000_ba", "DF2", DF2_f, DF2_d, ellip_df2_order8_cut2000_f32_ba[0], ellip_df2_order8_cut2000_f32_ba[1], ellip_df2_order8_cut2000_f64_ba[0], ellip_df2_order8_cut2000_f64_ba[1], 9);
    precision_analysis(fp_precision, "ellip_tdf2_order8_cut2000_ba", "TDF2", TDF2_f, TDF2_d, ellip_tdf2_order8_cut2000_f32_ba[0], ellip_tdf2_order8_cut2000_f32_ba[1], ellip_tdf2_order8_cut2000_f64_ba[0], ellip_tdf2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_cascade(fp_precision, "ellip_cascade_order8_cut2000_sos", CASCADE_f, CASCADE_d, (float*)ellip_cascade_order8_cut2000_f32_sos, (double*)ellip_cascade_order8_cut2000_f64_sos, 4);
    precision_analysis(fp_precision, "ellip_df1_order8_cut5000_ba", "DF1", DF1_f, DF1_d, ellip_df1_order8_cut5000_f32_ba[0], ellip_df1_order8_cut5000_f32_ba[1], ellip_df1_order8_cut5000_f64_ba[0], ellip_df1_order8_cut5000_f64_ba[1], 9);
    precision_analysis(fp_precision, "ellip_df2_order8_cut5000_ba", "DF2", DF2_f, DF2_d, ellip_df2_order8_cut5000_f32_ba[0], ellip_df2_order8_cut5000_f32_ba[1], ellip_df2_order8_cut5000_f64_ba[0], ellip_df2_order8_cut5000_f64_ba[1], 9);
    precision_analysis(fp_precision, "ellip_tdf2_order8_cut5000_ba", "TDF2", TDF2_f, TDF2_d, ellip_tdf2_order8_cut5000_f32_ba[0], ellip_tdf2_order8_cut5000_f32_ba[1], ellip_tdf2_order8_cut5000_f64_ba[0], ellip_tdf2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_cascade(fp_precision, "ellip_cascade_order8_cut5000_sos", CASCADE_f, CASCADE_d, (float*)ellip_cascade_order8_cut5000_f32_sos, (double*)ellip_cascade_order8_cut5000_f64_sos, 4);
    precision_analysis(fp_precision, "bessel_df1_order2_cut1000_ba", "DF1", DF1_f, DF1_d, bessel_df1_order2_cut1000_f32_ba[0], bessel_df1_order2_cut1000_f32_ba[1], bessel_df1_order2_cut1000_f64_ba[0], bessel_df1_order2_cut1000_f64_ba[1], 3);
    precision_analysis(fp_precision, "bessel_df2_order2_cut1000_ba", "DF2", DF2_f, DF2_d, bessel_df2_order2_cut1000_f32_ba[0], bessel_df2_order2_cut1000_f32_ba[1], bessel_df2_order2_cut1000_f64_ba[0], bessel_df2_order2_cut1000_f64_ba[1], 3);
    precision_analysis(fp_precision, "bessel_tdf2_order2_cut1000_ba", "TDF2", TDF2_f, TDF2_d, bessel_tdf2_order2_cut1000_f32_ba[0], bessel_tdf2_order2_cut1000_f32_ba[1], bessel_tdf2_order2_cut1000_f64_ba[0], bessel_tdf2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_cascade(fp_precision, "bessel_cascade_order2_cut1000_sos", CASCADE_f, CASCADE_d, (float*)bessel_cascade_order2_cut1000_f32_sos, (double*)bessel_cascade_order2_cut1000_f64_sos, 1);
    precision_analysis(fp_precision, "bessel_df1_order2_cut2000_ba", "DF1", DF1_f, DF1_d, bessel_df1_order2_cut2000_f32_ba[0], bessel_df1_order2_cut2000_f32_ba[1], bessel_df1_order2_cut2000_f64_ba[0], bessel_df1_order2_cut2000_f64_ba[1], 3);
    precision_analysis(fp_precision, "bessel_df2_order2_cut2000_ba", "DF2", DF2_f, DF2_d, bessel_df2_order2_cut2000_f32_ba[0], bessel_df2_order2_cut2000_f32_ba[1], bessel_df2_order2_cut2000_f64_ba[0], bessel_df2_order2_cut2000_f64_ba[1], 3);
    precision_analysis(fp_precision, "bessel_tdf2_order2_cut2000_ba", "TDF2", TDF2_f, TDF2_d, bessel_tdf2_order2_cut2000_f32_ba[0], bessel_tdf2_order2_cut2000_f32_ba[1], bessel_tdf2_order2_cut2000_f64_ba[0], bessel_tdf2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_cascade(fp_precision, "bessel_cascade_order2_cut2000_sos", CASCADE_f, CASCADE_d, (float*)bessel_cascade_order2_cut2000_f32_sos, (double*)bessel_cascade_order2_cut2000_f64_sos, 1);
    precision_analysis(fp_precision, "bessel_df1_order2_cut5000_ba", "DF1", DF1_f, DF1_d, bessel_df1_order2_cut5000_f32_ba[0], bessel_df1_order2_cut5000_f32_ba[1], bessel_df1_order2_cut5000_f64_ba[0], bessel_df1_order2_cut5000_f64_ba[1], 3);
    precision_analysis(fp_precision, "bessel_df2_order2_cut5000_ba", "DF2", DF2_f, DF2_d, bessel_df2_order2_cut5000_f32_ba[0], bessel_df2_order2_cut5000_f32_ba[1], bessel_df2_order2_cut5000_f64_ba[0], bessel_df2_order2_cut5000_f64_ba[1], 3);
    precision_analysis(fp_precision, "bessel_tdf2_order2_cut5000_ba", "TDF2", TDF2_f, TDF2_d, bessel_tdf2_order2_cut5000_f32_ba[0], bessel_tdf2_order2_cut5000_f32_ba[1], bessel_tdf2_order2_cut5000_f64_ba[0], bessel_tdf2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_cascade(fp_precision, "bessel_cascade_order2_cut5000_sos", CASCADE_f, CASCADE_d, (float*)bessel_cascade_order2_cut5000_f32_sos, (double*)bessel_cascade_order2_cut5000_f64_sos, 1);
    precision_analysis(fp_precision, "bessel_df1_order4_cut1000_ba", "DF1", DF1_f, DF1_d, bessel_df1_order4_cut1000_f32_ba[0], bessel_df1_order4_cut1000_f32_ba[1], bessel_df1_order4_cut1000_f64_ba[0], bessel_df1_order4_cut1000_f64_ba[1], 5);
    precision_analysis(fp_precision, "bessel_df2_order4_cut1000_ba", "DF2", DF2_f, DF2_d, bessel_df2_order4_cut1000_f32_ba[0], bessel_df2_order4_cut1000_f32_ba[1], bessel_df2_order4_cut1000_f64_ba[0], bessel_df2_order4_cut1000_f64_ba[1], 5);
    precision_analysis(fp_precision, "bessel_tdf2_order4_cut1000_ba", "TDF2", TDF2_f, TDF2_d, bessel_tdf2_order4_cut1000_f32_ba[0], bessel_tdf2_order4_cut1000_f32_ba[1], bessel_tdf2_order4_cut1000_f64_ba[0], bessel_tdf2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_cascade(fp_precision, "bessel_cascade_order4_cut1000_sos", CASCADE_f, CASCADE_d, (float*)bessel_cascade_order4_cut1000_f32_sos, (double*)bessel_cascade_order4_cut1000_f64_sos, 2);
    precision_analysis(fp_precision, "bessel_df1_order4_cut2000_ba", "DF1", DF1_f, DF1_d, bessel_df1_order4_cut2000_f32_ba[0], bessel_df1_order4_cut2000_f32_ba[1], bessel_df1_order4_cut2000_f64_ba[0], bessel_df1_order4_cut2000_f64_ba[1], 5);
    precision_analysis(fp_precision, "bessel_df2_order4_cut2000_ba", "DF2", DF2_f, DF2_d, bessel_df2_order4_cut2000_f32_ba[0], bessel_df2_order4_cut2000_f32_ba[1], bessel_df2_order4_cut2000_f64_ba[0], bessel_df2_order4_cut2000_f64_ba[1], 5);
    precision_analysis(fp_precision, "bessel_tdf2_order4_cut2000_ba", "TDF2", TDF2_f, TDF2_d, bessel_tdf2_order4_cut2000_f32_ba[0], bessel_tdf2_order4_cut2000_f32_ba[1], bessel_tdf2_order4_cut2000_f64_ba[0], bessel_tdf2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_cascade(fp_precision, "bessel_cascade_order4_cut2000_sos", CASCADE_f, CASCADE_d, (float*)bessel_cascade_order4_cut2000_f32_sos, (double*)bessel_cascade_order4_cut2000_f64_sos, 2);
    precision_analysis(fp_precision, "bessel_df1_order4_cut5000_ba", "DF1", DF1_f, DF1_d, bessel_df1_order4_cut5000_f32_ba[0], bessel_df1_order4_cut5000_f32_ba[1], bessel_df1_order4_cut5000_f64_ba[0], bessel_df1_order4_cut5000_f64_ba[1], 5);
    precision_analysis(fp_precision, "bessel_df2_order4_cut5000_ba", "DF2", DF2_f, DF2_d, bessel_df2_order4_cut5000_f32_ba[0], bessel_df2_order4_cut5000_f32_ba[1], bessel_df2_order4_cut5000_f64_ba[0], bessel_df2_order4_cut5000_f64_ba[1], 5);
    precision_analysis(fp_precision, "bessel_tdf2_order4_cut5000_ba", "TDF2", TDF2_f, TDF2_d, bessel_tdf2_order4_cut5000_f32_ba[0], bessel_tdf2_order4_cut5000_f32_ba[1], bessel_tdf2_order4_cut5000_f64_ba[0], bessel_tdf2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_cascade(fp_precision, "bessel_cascade_order4_cut5000_sos", CASCADE_f, CASCADE_d, (float*)bessel_cascade_order4_cut5000_f32_sos, (double*)bessel_cascade_order4_cut5000_f64_sos, 2);
    precision_analysis(fp_precision, "bessel_df1_order6_cut1000_ba", "DF1", DF1_f, DF1_d, bessel_df1_order6_cut1000_f32_ba[0], bessel_df1_order6_cut1000_f32_ba[1], bessel_df1_order6_cut1000_f64_ba[0], bessel_df1_order6_cut1000_f64_ba[1], 7);
    precision_analysis(fp_precision, "bessel_df2_order6_cut1000_ba", "DF2", DF2_f, DF2_d, bessel_df2_order6_cut1000_f32_ba[0], bessel_df2_order6_cut1000_f32_ba[1], bessel_df2_order6_cut1000_f64_ba[0], bessel_df2_order6_cut1000_f64_ba[1], 7);
    precision_analysis(fp_precision, "bessel_tdf2_order6_cut1000_ba", "TDF2", TDF2_f, TDF2_d, bessel_tdf2_order6_cut1000_f32_ba[0], bessel_tdf2_order6_cut1000_f32_ba[1], bessel_tdf2_order6_cut1000_f64_ba[0], bessel_tdf2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_cascade(fp_precision, "bessel_cascade_order6_cut1000_sos", CASCADE_f, CASCADE_d, (float*)bessel_cascade_order6_cut1000_f32_sos, (double*)bessel_cascade_order6_cut1000_f64_sos, 3);
    precision_analysis(fp_precision, "bessel_df1_order6_cut2000_ba", "DF1", DF1_f, DF1_d, bessel_df1_order6_cut2000_f32_ba[0], bessel_df1_order6_cut2000_f32_ba[1], bessel_df1_order6_cut2000_f64_ba[0], bessel_df1_order6_cut2000_f64_ba[1], 7);
    precision_analysis(fp_precision, "bessel_df2_order6_cut2000_ba", "DF2", DF2_f, DF2_d, bessel_df2_order6_cut2000_f32_ba[0], bessel_df2_order6_cut2000_f32_ba[1], bessel_df2_order6_cut2000_f64_ba[0], bessel_df2_order6_cut2000_f64_ba[1], 7);
    precision_analysis(fp_precision, "bessel_tdf2_order6_cut2000_ba", "TDF2", TDF2_f, TDF2_d, bessel_tdf2_order6_cut2000_f32_ba[0], bessel_tdf2_order6_cut2000_f32_ba[1], bessel_tdf2_order6_cut2000_f64_ba[0], bessel_tdf2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_cascade(fp_precision, "bessel_cascade_order6_cut2000_sos", CASCADE_f, CASCADE_d, (float*)bessel_cascade_order6_cut2000_f32_sos, (double*)bessel_cascade_order6_cut2000_f64_sos, 3);
    precision_analysis(fp_precision, "bessel_df1_order6_cut5000_ba", "DF1", DF1_f, DF1_d, bessel_df1_order6_cut5000_f32_ba[0], bessel_df1_order6_cut5000_f32_ba[1], bessel_df1_order6_cut5000_f64_ba[0], bessel_df1_order6_cut5000_f64_ba[1], 7);
    precision_analysis(fp_precision, "bessel_df2_order6_cut5000_ba", "DF2", DF2_f, DF2_d, bessel_df2_order6_cut5000_f32_ba[0], bessel_df2_order6_cut5000_f32_ba[1], bessel_df2_order6_cut5000_f64_ba[0], bessel_df2_order6_cut5000_f64_ba[1], 7);
    precision_analysis(fp_precision, "bessel_tdf2_order6_cut5000_ba", "TDF2", TDF2_f, TDF2_d, bessel_tdf2_order6_cut5000_f32_ba[0], bessel_tdf2_order6_cut5000_f32_ba[1], bessel_tdf2_order6_cut5000_f64_ba[0], bessel_tdf2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_cascade(fp_precision, "bessel_cascade_order6_cut5000_sos", CASCADE_f, CASCADE_d, (float*)bessel_cascade_order6_cut5000_f32_sos, (double*)bessel_cascade_order6_cut5000_f64_sos, 3);
    precision_analysis(fp_precision, "bessel_df1_order8_cut1000_ba", "DF1", DF1_f, DF1_d, bessel_df1_order8_cut1000_f32_ba[0], bessel_df1_order8_cut1000_f32_ba[1], bessel_df1_order8_cut1000_f64_ba[0], bessel_df1_order8_cut1000_f64_ba[1], 9);
    precision_analysis(fp_precision, "bessel_df2_order8_cut1000_ba", "DF2", DF2_f, DF2_d, bessel_df2_order8_cut1000_f32_ba[0], bessel_df2_order8_cut1000_f32_ba[1], bessel_df2_order8_cut1000_f64_ba[0], bessel_df2_order8_cut1000_f64_ba[1], 9);
    precision_analysis(fp_precision, "bessel_tdf2_order8_cut1000_ba", "TDF2", TDF2_f, TDF2_d, bessel_tdf2_order8_cut1000_f32_ba[0], bessel_tdf2_order8_cut1000_f32_ba[1], bessel_tdf2_order8_cut1000_f64_ba[0], bessel_tdf2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_cascade(fp_precision, "bessel_cascade_order8_cut1000_sos", CASCADE_f, CASCADE_d, (float*)bessel_cascade_order8_cut1000_f32_sos, (double*)bessel_cascade_order8_cut1000_f64_sos, 4);
    precision_analysis(fp_precision, "bessel_df1_order8_cut2000_ba", "DF1", DF1_f, DF1_d, bessel_df1_order8_cut2000_f32_ba[0], bessel_df1_order8_cut2000_f32_ba[1], bessel_df1_order8_cut2000_f64_ba[0], bessel_df1_order8_cut2000_f64_ba[1], 9);
    precision_analysis(fp_precision, "bessel_df2_order8_cut2000_ba", "DF2", DF2_f, DF2_d, bessel_df2_order8_cut2000_f32_ba[0], bessel_df2_order8_cut2000_f32_ba[1], bessel_df2_order8_cut2000_f64_ba[0], bessel_df2_order8_cut2000_f64_ba[1], 9);
    precision_analysis(fp_precision, "bessel_tdf2_order8_cut2000_ba", "TDF2", TDF2_f, TDF2_d, bessel_tdf2_order8_cut2000_f32_ba[0], bessel_tdf2_order8_cut2000_f32_ba[1], bessel_tdf2_order8_cut2000_f64_ba[0], bessel_tdf2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_cascade(fp_precision, "bessel_cascade_order8_cut2000_sos", CASCADE_f, CASCADE_d, (float*)bessel_cascade_order8_cut2000_f32_sos, (double*)bessel_cascade_order8_cut2000_f64_sos, 4);
    precision_analysis(fp_precision, "bessel_df1_order8_cut5000_ba", "DF1", DF1_f, DF1_d, bessel_df1_order8_cut5000_f32_ba[0], bessel_df1_order8_cut5000_f32_ba[1], bessel_df1_order8_cut5000_f64_ba[0], bessel_df1_order8_cut5000_f64_ba[1], 9);
    precision_analysis(fp_precision, "bessel_df2_order8_cut5000_ba", "DF2", DF2_f, DF2_d, bessel_df2_order8_cut5000_f32_ba[0], bessel_df2_order8_cut5000_f32_ba[1], bessel_df2_order8_cut5000_f64_ba[0], bessel_df2_order8_cut5000_f64_ba[1], 9);
    precision_analysis(fp_precision, "bessel_tdf2_order8_cut5000_ba", "TDF2", TDF2_f, TDF2_d, bessel_tdf2_order8_cut5000_f32_ba[0], bessel_tdf2_order8_cut5000_f32_ba[1], bessel_tdf2_order8_cut5000_f64_ba[0], bessel_tdf2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_cascade(fp_precision, "bessel_cascade_order8_cut5000_sos", CASCADE_f, CASCADE_d, (float*)bessel_cascade_order8_cut5000_f32_sos, (double*)bessel_cascade_order8_cut5000_f64_sos, 4);
    
    fclose(fp_precision);
}
