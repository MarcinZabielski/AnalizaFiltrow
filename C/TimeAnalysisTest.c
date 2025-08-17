#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "./lib/structures.h"
#include "../_filtercoeffs/filtercoeffs.h"

#define SAMPLE_RATE 48000
#define DURATION_SEC 10
#define N (SAMPLE_RATE * DURATION_SEC)

//Kompilacja: gcc -o TimeAnalysisTest TimeAnalysisTest.c ./lib/structures.c ../_filtercoeffs/filtercoeffs.c

void generate_white_noise_f(float *x, int n) {
    for (int i = 0; i < n; i++)
        x[i] = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
}

void generate_white_noise_d(double *x, int n) {
    for (int i = 0; i < n; i++)
        x[i] = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
}

void run_and_export_df_struct(const char *filename,
                                void (*func_f)(float*, float*, float*, float*, int, int),
                                void (*func_d)(double*, double*, double*, double*, int, int),
                                float *b_f, float *a_f, double *b_d, double *a_d,
                                int order) {
    float *x_f = malloc(sizeof(float) * N);
    double *x_d = malloc(sizeof(double) * N);
    float *y_f = malloc(sizeof(float) * N);
    double *y_d = malloc(sizeof(double) * N);

    generate_white_noise_f(x_f, N);
    for (int i = 0; i < N; i++) x_d[i] = x_f[i];

    clock_t start, end;
    double time_f, time_d;

    start = clock();
    func_f(x_f, y_f, b_f, a_f, N, order);
    end = clock();
    time_f = (double)(end - start) / CLOCKS_PER_SEC;

    start = clock();
    func_d(x_d, y_d, b_d, a_d, N, order);
    end = clock();
    time_d = (double)(end - start) / CLOCKS_PER_SEC;

    FILE *fp = fopen(filename, "w");
    fprintf(fp, "index,y_float,y_double\n");
    for (int i = 0; i < N; i++)
    fprintf(fp, "%d,%.10f,%.10lf\n", i, y_f[i], y_d[i]);
    fclose(fp);

    printf(" [%s] Time float: %.6f s, Time double: %.6f s\n", filename, time_f, time_d);

    free(x_f); free(x_d); free(y_f); free(y_d);
}


void run_and_export_cascade(const char *filename,
                             void (*func_f)(float*, float*, float*, int, int),
                             void (*func_d)(double*, double*, double*, int, int),
                             float sos_f[][6], double sos_d[][6], int sections) {
    float *x_f = malloc(sizeof(float) * N);
    double *x_d = malloc(sizeof(double) * N);
    float *y_f = malloc(sizeof(float) * N);
    double *y_d = malloc(sizeof(double) * N);

    generate_white_noise_f(x_f, N);
    for (int i = 0; i < N; i++) x_d[i] = x_f[i];

    func_f(x_f, y_f, (float*)sos_f, N, sections);
    func_d(x_d, y_d, (double*)sos_d, N, sections);

    FILE *fp = fopen(filename, "w");
    fprintf(fp, "index,y_float,y_double\n");
    for (int i = 0; i < N; i++)
        fprintf(fp, "%d,%.10f,%.10lf\n", i, y_f[i], y_d[i]);
    fclose(fp);

    free(x_f); free(x_d); free(y_f); free(y_d);
}

int main() {
    int order = 3;  // dla Butterworth order2

    run_and_export_df_struct("df1.csv", DF1_f, DF1_d,
                             butter_df1_order2_cut1000_f32_ba[0], butter_df1_order2_cut1000_f32_ba[1],
                             butter_df1_order2_cut1000_f64_ba[0], butter_df1_order2_cut1000_f64_ba[1],
                             order);
    
    run_and_export_df_struct("df2.csv", DF2_f, DF2_d,
                             butter_df1_order2_cut1000_f32_ba[0], butter_df1_order2_cut1000_f32_ba[1],
                             butter_df1_order2_cut1000_f64_ba[0], butter_df1_order2_cut1000_f64_ba[1],
                             order);
    
    run_and_export_df_struct("tdf2.csv", TDF2_f, TDF2_d,
                             butter_df1_order2_cut1000_f32_ba[0], butter_df1_order2_cut1000_f32_ba[1],
                             butter_df1_order2_cut1000_f64_ba[0], butter_df1_order2_cut1000_f64_ba[1],
                             order);

    run_and_export_cascade("cascade.csv", CASCADE_f, CASCADE_d, butter_cascade_order2_cut1000_f32_sos, butter_cascade_order2_cut1000_f64_sos, 1);

    return 0;
}
