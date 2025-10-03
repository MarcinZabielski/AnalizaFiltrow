#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include "./lib/structures.h"
#include "../_filtercoeffs/filtercoeffs.h"

#define FS 48000
#define DURATION_MIN 1
int N = (FS * 60 * DURATION_MIN);

// Analiza szybkości filtrowania w języku C (reprezentacja zmiennopozycyjna)
//Kompilacja: gcc -o TimeAnalysis TimeAnalysis.c ./lib/structures.c ../_filtercoeffs/filtercoeffs.c

// === Generowanie szumu ===
void generate_white_noise_f(float *x, int N) {
    for (int i = 0; i < N; ++i)
        x[i] = 2.0f * ((float)rand() / RAND_MAX) - 1.0f;
}

void generate_white_noise_d(double *x, int N) {
    for (int i = 0; i < N; ++i)
        x[i] = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
}

// === Analiza czasu ===
void benchmark_and_log(FILE *fp, const char *filter_name, const char *type, const char *structure,
                       void (*func_f)(float*, float*, float*, float*, int, int),
                       void (*func_d)(double*, double*, double*, double*, int, int),
                       float *b_f, float *a_f, double *b_d, double *a_d, int order) {

    LARGE_INTEGER freq, start, end;
    double time_spent;
    int cutoff = -1;

    char filter_type[32];
    sscanf(filter_name, "%[^_]", filter_type);
    
    char *cut_ptr = strstr(filter_name, "_cut");
    if (cut_ptr != NULL)
        sscanf(cut_ptr, "_cut%d", &cutoff);
    
    if (strcmp(type, "float") == 0 && func_f) {
        float *x = malloc(sizeof(float) * N);
        float *y = malloc(sizeof(float) * N);
        generate_white_noise_f(x, N);

        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&start);

        func_f(x, y, b_f, a_f, N, order);

        QueryPerformanceCounter(&end);
        time_spent = (double)(end.QuadPart - start.QuadPart) / (double)freq.QuadPart;

        fprintf(fp, "%s,float,%s,%d,%d,%.6f\n", filter_type, structure, cutoff, order - 1, time_spent);
        printf("[LOG] %s (float, %s, cut %d, order %d): %.6f sec\n", filter_type, structure, cutoff, order - 1, time_spent);
        free(x); free(y);
    }

    if (strcmp(type, "double") == 0 && func_d) {
        double *x = malloc(sizeof(double) * N);
        double *y = malloc(sizeof(double) * N);
        generate_white_noise_d(x, N);

        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&start);

        func_d(x, y, b_d, a_d, N, order);

        QueryPerformanceCounter(&end);
        time_spent = (double)(end.QuadPart - start.QuadPart) / (double)freq.QuadPart;

        fprintf(fp, "%s,double,%s,%d,%d,%.6f\n", filter_type, structure, cutoff, order - 1, time_spent);
        printf("[LOG] %s (double, %s, cut %d, order %d): %.6f sec\n", filter_type, structure, cutoff, order - 1, time_spent);
        free(x); free(y);
    }
}

void benchmark_cascade_and_log(FILE *fp, const char *filter_name, const char *type,
                                void (*func_f)(float*, float*, float*, int, int),
                                void (*func_d)(double*, double*, double*, int, int),
                                float *sos_f, double *sos_d, int sections) {

    LARGE_INTEGER freq, start, end;
    double time_spent;
    int cutoff = -1;

    //parsowanie filter_type
    char filter_type[32];
    sscanf(filter_name, "%[^_]", filter_type);
    
    //parsowanie cutoff
    char *cut_ptr = strstr(filter_name, "_cut");
    if (cut_ptr != NULL)
        sscanf(cut_ptr, "_cut%d", &cutoff);

    if (strcmp(type, "float") == 0 && func_f) {
        float *x = malloc(sizeof(float) * N);
        float *y = malloc(sizeof(float) * N);

        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&start);

        func_f(x, y, sos_f, N, sections);

        QueryPerformanceCounter(&end);
        time_spent = (double)(end.QuadPart - start.QuadPart) / (double)freq.QuadPart;

        fprintf(fp, "%s,float,CASCADE,%d,%d,%.6f\n", filter_type, cutoff, 2 * sections, time_spent);
        printf("[LOG] %s (float, CASCADE, cut %d, order %d): %.6f sec\n", filter_type, cutoff, 2 * sections, time_spent);
        free(x); free(y);
    }

    if (strcmp(type, "double") == 0 && func_d) {
        double *x = malloc(sizeof(double) * N);
        double *y = malloc(sizeof(double) * N);
        generate_white_noise_d(x, N);

        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&start);

        func_d(x, y, sos_d, N, sections);

        QueryPerformanceCounter(&end);
        time_spent = (double)(end.QuadPart - start.QuadPart) / (double)freq.QuadPart;

        fprintf(fp, "%s,double,CASCADE,%d,%d,%.6f\n", filter_type, cutoff, 2 * sections, time_spent);
        printf("[LOG] %s (double, CASCADE, cut %d, order %d): %.6f sec\n", filter_type, cutoff, 2 * sections, time_spent);
        free(x); free(y);
    }
}

int main() {
    FILE *fp = fopen("c_floating_time_results.csv", "w");
    if (!fp) {
        perror("Can't open CSV file");
        return 1;
    }

    fprintf(fp, "filter_name,type,structure,cutoff,order,time_seconds\n");
    
    for (size_t i = 0; i < 20; i++)
    {
        // Tu wkleić zawartość pliku generated_calls_time_floating.txt
        benchmark_and_log(fp, "butter_df1_order2_cut1000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, butter_df1_order2_cut1000_f64_ba[0], butter_df1_order2_cut1000_f64_ba[1], 3);
        benchmark_and_log(fp, "butter_df2_order2_cut1000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, butter_df2_order2_cut1000_f64_ba[0], butter_df2_order2_cut1000_f64_ba[1], 3);
        benchmark_and_log(fp, "butter_tdf2_order2_cut1000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, butter_tdf2_order2_cut1000_f64_ba[0], butter_tdf2_order2_cut1000_f64_ba[1], 3);
        benchmark_cascade_and_log(fp, "butter_cascade_order2_cut1000_f64_sos", "double", NULL, CASCADE_d, NULL, *butter_cascade_order2_cut1000_f64_sos, 1);
        benchmark_and_log(fp, "butter_df1_order2_cut1000_f32_ba", "float", "DF1", DF1_f, NULL, butter_df1_order2_cut1000_f32_ba[0], butter_df1_order2_cut1000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "butter_df2_order2_cut1000_f32_ba", "float", "DF2", DF2_f, NULL, butter_df2_order2_cut1000_f32_ba[0], butter_df2_order2_cut1000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "butter_tdf2_order2_cut1000_f32_ba", "float", "TDF2", TDF2_f, NULL, butter_tdf2_order2_cut1000_f32_ba[0], butter_tdf2_order2_cut1000_f32_ba[1], NULL, NULL, 3);
        benchmark_cascade_and_log(fp, "butter_cascade_order2_cut1000_f32_sos", "float", CASCADE_f, NULL, *butter_cascade_order2_cut1000_f32_sos, NULL, 1);
        benchmark_and_log(fp, "butter_df1_order2_cut2000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, butter_df1_order2_cut2000_f64_ba[0], butter_df1_order2_cut2000_f64_ba[1], 3);
        benchmark_and_log(fp, "butter_df2_order2_cut2000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, butter_df2_order2_cut2000_f64_ba[0], butter_df2_order2_cut2000_f64_ba[1], 3);
        benchmark_and_log(fp, "butter_tdf2_order2_cut2000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, butter_tdf2_order2_cut2000_f64_ba[0], butter_tdf2_order2_cut2000_f64_ba[1], 3);
        benchmark_cascade_and_log(fp, "butter_cascade_order2_cut2000_f64_sos", "double", NULL, CASCADE_d, NULL, *butter_cascade_order2_cut2000_f64_sos, 1);
        benchmark_and_log(fp, "butter_df1_order2_cut2000_f32_ba", "float", "DF1", DF1_f, NULL, butter_df1_order2_cut2000_f32_ba[0], butter_df1_order2_cut2000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "butter_df2_order2_cut2000_f32_ba", "float", "DF2", DF2_f, NULL, butter_df2_order2_cut2000_f32_ba[0], butter_df2_order2_cut2000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "butter_tdf2_order2_cut2000_f32_ba", "float", "TDF2", TDF2_f, NULL, butter_tdf2_order2_cut2000_f32_ba[0], butter_tdf2_order2_cut2000_f32_ba[1], NULL, NULL, 3);
        benchmark_cascade_and_log(fp, "butter_cascade_order2_cut2000_f32_sos", "float", CASCADE_f, NULL, *butter_cascade_order2_cut2000_f32_sos, NULL, 1);
        benchmark_and_log(fp, "butter_df1_order2_cut5000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, butter_df1_order2_cut5000_f64_ba[0], butter_df1_order2_cut5000_f64_ba[1], 3);
        benchmark_and_log(fp, "butter_df2_order2_cut5000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, butter_df2_order2_cut5000_f64_ba[0], butter_df2_order2_cut5000_f64_ba[1], 3);
        benchmark_and_log(fp, "butter_tdf2_order2_cut5000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, butter_tdf2_order2_cut5000_f64_ba[0], butter_tdf2_order2_cut5000_f64_ba[1], 3);
        benchmark_cascade_and_log(fp, "butter_cascade_order2_cut5000_f64_sos", "double", NULL, CASCADE_d, NULL, *butter_cascade_order2_cut5000_f64_sos, 1);
        benchmark_and_log(fp, "butter_df1_order2_cut5000_f32_ba", "float", "DF1", DF1_f, NULL, butter_df1_order2_cut5000_f32_ba[0], butter_df1_order2_cut5000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "butter_df2_order2_cut5000_f32_ba", "float", "DF2", DF2_f, NULL, butter_df2_order2_cut5000_f32_ba[0], butter_df2_order2_cut5000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "butter_tdf2_order2_cut5000_f32_ba", "float", "TDF2", TDF2_f, NULL, butter_tdf2_order2_cut5000_f32_ba[0], butter_tdf2_order2_cut5000_f32_ba[1], NULL, NULL, 3);
        benchmark_cascade_and_log(fp, "butter_cascade_order2_cut5000_f32_sos", "float", CASCADE_f, NULL, *butter_cascade_order2_cut5000_f32_sos, NULL, 1);
        benchmark_and_log(fp, "butter_df1_order4_cut1000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, butter_df1_order4_cut1000_f64_ba[0], butter_df1_order4_cut1000_f64_ba[1], 5);
        benchmark_and_log(fp, "butter_df2_order4_cut1000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, butter_df2_order4_cut1000_f64_ba[0], butter_df2_order4_cut1000_f64_ba[1], 5);
        benchmark_and_log(fp, "butter_tdf2_order4_cut1000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, butter_tdf2_order4_cut1000_f64_ba[0], butter_tdf2_order4_cut1000_f64_ba[1], 5);
        benchmark_cascade_and_log(fp, "butter_cascade_order4_cut1000_f64_sos", "double", NULL, CASCADE_d, NULL, *butter_cascade_order4_cut1000_f64_sos, 2);
        benchmark_and_log(fp, "butter_df1_order4_cut1000_f32_ba", "float", "DF1", DF1_f, NULL, butter_df1_order4_cut1000_f32_ba[0], butter_df1_order4_cut1000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "butter_df2_order4_cut1000_f32_ba", "float", "DF2", DF2_f, NULL, butter_df2_order4_cut1000_f32_ba[0], butter_df2_order4_cut1000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "butter_tdf2_order4_cut1000_f32_ba", "float", "TDF2", TDF2_f, NULL, butter_tdf2_order4_cut1000_f32_ba[0], butter_tdf2_order4_cut1000_f32_ba[1], NULL, NULL, 5);
        benchmark_cascade_and_log(fp, "butter_cascade_order4_cut1000_f32_sos", "float", CASCADE_f, NULL, *butter_cascade_order4_cut1000_f32_sos, NULL, 2);
        benchmark_and_log(fp, "butter_df1_order4_cut2000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, butter_df1_order4_cut2000_f64_ba[0], butter_df1_order4_cut2000_f64_ba[1], 5);
        benchmark_and_log(fp, "butter_df2_order4_cut2000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, butter_df2_order4_cut2000_f64_ba[0], butter_df2_order4_cut2000_f64_ba[1], 5);
        benchmark_and_log(fp, "butter_tdf2_order4_cut2000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, butter_tdf2_order4_cut2000_f64_ba[0], butter_tdf2_order4_cut2000_f64_ba[1], 5);
        benchmark_cascade_and_log(fp, "butter_cascade_order4_cut2000_f64_sos", "double", NULL, CASCADE_d, NULL, *butter_cascade_order4_cut2000_f64_sos, 2);
        benchmark_and_log(fp, "butter_df1_order4_cut2000_f32_ba", "float", "DF1", DF1_f, NULL, butter_df1_order4_cut2000_f32_ba[0], butter_df1_order4_cut2000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "butter_df2_order4_cut2000_f32_ba", "float", "DF2", DF2_f, NULL, butter_df2_order4_cut2000_f32_ba[0], butter_df2_order4_cut2000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "butter_tdf2_order4_cut2000_f32_ba", "float", "TDF2", TDF2_f, NULL, butter_tdf2_order4_cut2000_f32_ba[0], butter_tdf2_order4_cut2000_f32_ba[1], NULL, NULL, 5);
        benchmark_cascade_and_log(fp, "butter_cascade_order4_cut2000_f32_sos", "float", CASCADE_f, NULL, *butter_cascade_order4_cut2000_f32_sos, NULL, 2);
        benchmark_and_log(fp, "butter_df1_order4_cut5000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, butter_df1_order4_cut5000_f64_ba[0], butter_df1_order4_cut5000_f64_ba[1], 5);
        benchmark_and_log(fp, "butter_df2_order4_cut5000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, butter_df2_order4_cut5000_f64_ba[0], butter_df2_order4_cut5000_f64_ba[1], 5);
        benchmark_and_log(fp, "butter_tdf2_order4_cut5000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, butter_tdf2_order4_cut5000_f64_ba[0], butter_tdf2_order4_cut5000_f64_ba[1], 5);
        benchmark_cascade_and_log(fp, "butter_cascade_order4_cut5000_f64_sos", "double", NULL, CASCADE_d, NULL, *butter_cascade_order4_cut5000_f64_sos, 2);
        benchmark_and_log(fp, "butter_df1_order4_cut5000_f32_ba", "float", "DF1", DF1_f, NULL, butter_df1_order4_cut5000_f32_ba[0], butter_df1_order4_cut5000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "butter_df2_order4_cut5000_f32_ba", "float", "DF2", DF2_f, NULL, butter_df2_order4_cut5000_f32_ba[0], butter_df2_order4_cut5000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "butter_tdf2_order4_cut5000_f32_ba", "float", "TDF2", TDF2_f, NULL, butter_tdf2_order4_cut5000_f32_ba[0], butter_tdf2_order4_cut5000_f32_ba[1], NULL, NULL, 5);
        benchmark_cascade_and_log(fp, "butter_cascade_order4_cut5000_f32_sos", "float", CASCADE_f, NULL, *butter_cascade_order4_cut5000_f32_sos, NULL, 2);
        benchmark_and_log(fp, "butter_df1_order6_cut1000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, butter_df1_order6_cut1000_f64_ba[0], butter_df1_order6_cut1000_f64_ba[1], 7);
        benchmark_and_log(fp, "butter_df2_order6_cut1000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, butter_df2_order6_cut1000_f64_ba[0], butter_df2_order6_cut1000_f64_ba[1], 7);
        benchmark_and_log(fp, "butter_tdf2_order6_cut1000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, butter_tdf2_order6_cut1000_f64_ba[0], butter_tdf2_order6_cut1000_f64_ba[1], 7);
        benchmark_cascade_and_log(fp, "butter_cascade_order6_cut1000_f64_sos", "double", NULL, CASCADE_d, NULL, *butter_cascade_order6_cut1000_f64_sos, 3);
        benchmark_and_log(fp, "butter_df1_order6_cut1000_f32_ba", "float", "DF1", DF1_f, NULL, butter_df1_order6_cut1000_f32_ba[0], butter_df1_order6_cut1000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "butter_df2_order6_cut1000_f32_ba", "float", "DF2", DF2_f, NULL, butter_df2_order6_cut1000_f32_ba[0], butter_df2_order6_cut1000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "butter_tdf2_order6_cut1000_f32_ba", "float", "TDF2", TDF2_f, NULL, butter_tdf2_order6_cut1000_f32_ba[0], butter_tdf2_order6_cut1000_f32_ba[1], NULL, NULL, 7);
        benchmark_cascade_and_log(fp, "butter_cascade_order6_cut1000_f32_sos", "float", CASCADE_f, NULL, *butter_cascade_order6_cut1000_f32_sos, NULL, 3);
        benchmark_and_log(fp, "butter_df1_order6_cut2000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, butter_df1_order6_cut2000_f64_ba[0], butter_df1_order6_cut2000_f64_ba[1], 7);
        benchmark_and_log(fp, "butter_df2_order6_cut2000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, butter_df2_order6_cut2000_f64_ba[0], butter_df2_order6_cut2000_f64_ba[1], 7);
        benchmark_and_log(fp, "butter_tdf2_order6_cut2000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, butter_tdf2_order6_cut2000_f64_ba[0], butter_tdf2_order6_cut2000_f64_ba[1], 7);
        benchmark_cascade_and_log(fp, "butter_cascade_order6_cut2000_f64_sos", "double", NULL, CASCADE_d, NULL, *butter_cascade_order6_cut2000_f64_sos, 3);
        benchmark_and_log(fp, "butter_df1_order6_cut2000_f32_ba", "float", "DF1", DF1_f, NULL, butter_df1_order6_cut2000_f32_ba[0], butter_df1_order6_cut2000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "butter_df2_order6_cut2000_f32_ba", "float", "DF2", DF2_f, NULL, butter_df2_order6_cut2000_f32_ba[0], butter_df2_order6_cut2000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "butter_tdf2_order6_cut2000_f32_ba", "float", "TDF2", TDF2_f, NULL, butter_tdf2_order6_cut2000_f32_ba[0], butter_tdf2_order6_cut2000_f32_ba[1], NULL, NULL, 7);
        benchmark_cascade_and_log(fp, "butter_cascade_order6_cut2000_f32_sos", "float", CASCADE_f, NULL, *butter_cascade_order6_cut2000_f32_sos, NULL, 3);
        benchmark_and_log(fp, "butter_df1_order6_cut5000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, butter_df1_order6_cut5000_f64_ba[0], butter_df1_order6_cut5000_f64_ba[1], 7);
        benchmark_and_log(fp, "butter_df2_order6_cut5000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, butter_df2_order6_cut5000_f64_ba[0], butter_df2_order6_cut5000_f64_ba[1], 7);
        benchmark_and_log(fp, "butter_tdf2_order6_cut5000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, butter_tdf2_order6_cut5000_f64_ba[0], butter_tdf2_order6_cut5000_f64_ba[1], 7);
        benchmark_cascade_and_log(fp, "butter_cascade_order6_cut5000_f64_sos", "double", NULL, CASCADE_d, NULL, *butter_cascade_order6_cut5000_f64_sos, 3);
        benchmark_and_log(fp, "butter_df1_order6_cut5000_f32_ba", "float", "DF1", DF1_f, NULL, butter_df1_order6_cut5000_f32_ba[0], butter_df1_order6_cut5000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "butter_df2_order6_cut5000_f32_ba", "float", "DF2", DF2_f, NULL, butter_df2_order6_cut5000_f32_ba[0], butter_df2_order6_cut5000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "butter_tdf2_order6_cut5000_f32_ba", "float", "TDF2", TDF2_f, NULL, butter_tdf2_order6_cut5000_f32_ba[0], butter_tdf2_order6_cut5000_f32_ba[1], NULL, NULL, 7);
        benchmark_cascade_and_log(fp, "butter_cascade_order6_cut5000_f32_sos", "float", CASCADE_f, NULL, *butter_cascade_order6_cut5000_f32_sos, NULL, 3);
        benchmark_and_log(fp, "butter_df1_order8_cut1000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, butter_df1_order8_cut1000_f64_ba[0], butter_df1_order8_cut1000_f64_ba[1], 9);
        benchmark_and_log(fp, "butter_df2_order8_cut1000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, butter_df2_order8_cut1000_f64_ba[0], butter_df2_order8_cut1000_f64_ba[1], 9);
        benchmark_and_log(fp, "butter_tdf2_order8_cut1000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, butter_tdf2_order8_cut1000_f64_ba[0], butter_tdf2_order8_cut1000_f64_ba[1], 9);
        benchmark_cascade_and_log(fp, "butter_cascade_order8_cut1000_f64_sos", "double", NULL, CASCADE_d, NULL, *butter_cascade_order8_cut1000_f64_sos, 4);
        benchmark_and_log(fp, "butter_df1_order8_cut1000_f32_ba", "float", "DF1", DF1_f, NULL, butter_df1_order8_cut1000_f32_ba[0], butter_df1_order8_cut1000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "butter_df2_order8_cut1000_f32_ba", "float", "DF2", DF2_f, NULL, butter_df2_order8_cut1000_f32_ba[0], butter_df2_order8_cut1000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "butter_tdf2_order8_cut1000_f32_ba", "float", "TDF2", TDF2_f, NULL, butter_tdf2_order8_cut1000_f32_ba[0], butter_tdf2_order8_cut1000_f32_ba[1], NULL, NULL, 9);
        benchmark_cascade_and_log(fp, "butter_cascade_order8_cut1000_f32_sos", "float", CASCADE_f, NULL, *butter_cascade_order8_cut1000_f32_sos, NULL, 4);
        benchmark_and_log(fp, "butter_df1_order8_cut2000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, butter_df1_order8_cut2000_f64_ba[0], butter_df1_order8_cut2000_f64_ba[1], 9);
        benchmark_and_log(fp, "butter_df2_order8_cut2000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, butter_df2_order8_cut2000_f64_ba[0], butter_df2_order8_cut2000_f64_ba[1], 9);
        benchmark_and_log(fp, "butter_tdf2_order8_cut2000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, butter_tdf2_order8_cut2000_f64_ba[0], butter_tdf2_order8_cut2000_f64_ba[1], 9);
        benchmark_cascade_and_log(fp, "butter_cascade_order8_cut2000_f64_sos", "double", NULL, CASCADE_d, NULL, *butter_cascade_order8_cut2000_f64_sos, 4);
        benchmark_and_log(fp, "butter_df1_order8_cut2000_f32_ba", "float", "DF1", DF1_f, NULL, butter_df1_order8_cut2000_f32_ba[0], butter_df1_order8_cut2000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "butter_df2_order8_cut2000_f32_ba", "float", "DF2", DF2_f, NULL, butter_df2_order8_cut2000_f32_ba[0], butter_df2_order8_cut2000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "butter_tdf2_order8_cut2000_f32_ba", "float", "TDF2", TDF2_f, NULL, butter_tdf2_order8_cut2000_f32_ba[0], butter_tdf2_order8_cut2000_f32_ba[1], NULL, NULL, 9);
        benchmark_cascade_and_log(fp, "butter_cascade_order8_cut2000_f32_sos", "float", CASCADE_f, NULL, *butter_cascade_order8_cut2000_f32_sos, NULL, 4);
        benchmark_and_log(fp, "butter_df1_order8_cut5000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, butter_df1_order8_cut5000_f64_ba[0], butter_df1_order8_cut5000_f64_ba[1], 9);
        benchmark_and_log(fp, "butter_df2_order8_cut5000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, butter_df2_order8_cut5000_f64_ba[0], butter_df2_order8_cut5000_f64_ba[1], 9);
        benchmark_and_log(fp, "butter_tdf2_order8_cut5000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, butter_tdf2_order8_cut5000_f64_ba[0], butter_tdf2_order8_cut5000_f64_ba[1], 9);
        benchmark_cascade_and_log(fp, "butter_cascade_order8_cut5000_f64_sos", "double", NULL, CASCADE_d, NULL, *butter_cascade_order8_cut5000_f64_sos, 4);
        benchmark_and_log(fp, "butter_df1_order8_cut5000_f32_ba", "float", "DF1", DF1_f, NULL, butter_df1_order8_cut5000_f32_ba[0], butter_df1_order8_cut5000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "butter_df2_order8_cut5000_f32_ba", "float", "DF2", DF2_f, NULL, butter_df2_order8_cut5000_f32_ba[0], butter_df2_order8_cut5000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "butter_tdf2_order8_cut5000_f32_ba", "float", "TDF2", TDF2_f, NULL, butter_tdf2_order8_cut5000_f32_ba[0], butter_tdf2_order8_cut5000_f32_ba[1], NULL, NULL, 9);
        benchmark_cascade_and_log(fp, "butter_cascade_order8_cut5000_f32_sos", "float", CASCADE_f, NULL, *butter_cascade_order8_cut5000_f32_sos, NULL, 4);
        benchmark_and_log(fp, "cheby1_df1_order2_cut1000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby1_df1_order2_cut1000_f64_ba[0], cheby1_df1_order2_cut1000_f64_ba[1], 3);
        benchmark_and_log(fp, "cheby1_df2_order2_cut1000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby1_df2_order2_cut1000_f64_ba[0], cheby1_df2_order2_cut1000_f64_ba[1], 3);
        benchmark_and_log(fp, "cheby1_tdf2_order2_cut1000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby1_tdf2_order2_cut1000_f64_ba[0], cheby1_tdf2_order2_cut1000_f64_ba[1], 3);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order2_cut1000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby1_cascade_order2_cut1000_f64_sos, 1);
        benchmark_and_log(fp, "cheby1_df1_order2_cut1000_f32_ba", "float", "DF1", DF1_f, NULL, cheby1_df1_order2_cut1000_f32_ba[0], cheby1_df1_order2_cut1000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "cheby1_df2_order2_cut1000_f32_ba", "float", "DF2", DF2_f, NULL, cheby1_df2_order2_cut1000_f32_ba[0], cheby1_df2_order2_cut1000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "cheby1_tdf2_order2_cut1000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby1_tdf2_order2_cut1000_f32_ba[0], cheby1_tdf2_order2_cut1000_f32_ba[1], NULL, NULL, 3);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order2_cut1000_f32_sos", "float", CASCADE_f, NULL, *cheby1_cascade_order2_cut1000_f32_sos, NULL, 1);
        benchmark_and_log(fp, "cheby1_df1_order2_cut2000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby1_df1_order2_cut2000_f64_ba[0], cheby1_df1_order2_cut2000_f64_ba[1], 3);
        benchmark_and_log(fp, "cheby1_df2_order2_cut2000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby1_df2_order2_cut2000_f64_ba[0], cheby1_df2_order2_cut2000_f64_ba[1], 3);
        benchmark_and_log(fp, "cheby1_tdf2_order2_cut2000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby1_tdf2_order2_cut2000_f64_ba[0], cheby1_tdf2_order2_cut2000_f64_ba[1], 3);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order2_cut2000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby1_cascade_order2_cut2000_f64_sos, 1);
        benchmark_and_log(fp, "cheby1_df1_order2_cut2000_f32_ba", "float", "DF1", DF1_f, NULL, cheby1_df1_order2_cut2000_f32_ba[0], cheby1_df1_order2_cut2000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "cheby1_df2_order2_cut2000_f32_ba", "float", "DF2", DF2_f, NULL, cheby1_df2_order2_cut2000_f32_ba[0], cheby1_df2_order2_cut2000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "cheby1_tdf2_order2_cut2000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby1_tdf2_order2_cut2000_f32_ba[0], cheby1_tdf2_order2_cut2000_f32_ba[1], NULL, NULL, 3);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order2_cut2000_f32_sos", "float", CASCADE_f, NULL, *cheby1_cascade_order2_cut2000_f32_sos, NULL, 1);
        benchmark_and_log(fp, "cheby1_df1_order2_cut5000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby1_df1_order2_cut5000_f64_ba[0], cheby1_df1_order2_cut5000_f64_ba[1], 3);
        benchmark_and_log(fp, "cheby1_df2_order2_cut5000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby1_df2_order2_cut5000_f64_ba[0], cheby1_df2_order2_cut5000_f64_ba[1], 3);
        benchmark_and_log(fp, "cheby1_tdf2_order2_cut5000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby1_tdf2_order2_cut5000_f64_ba[0], cheby1_tdf2_order2_cut5000_f64_ba[1], 3);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order2_cut5000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby1_cascade_order2_cut5000_f64_sos, 1);
        benchmark_and_log(fp, "cheby1_df1_order2_cut5000_f32_ba", "float", "DF1", DF1_f, NULL, cheby1_df1_order2_cut5000_f32_ba[0], cheby1_df1_order2_cut5000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "cheby1_df2_order2_cut5000_f32_ba", "float", "DF2", DF2_f, NULL, cheby1_df2_order2_cut5000_f32_ba[0], cheby1_df2_order2_cut5000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "cheby1_tdf2_order2_cut5000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby1_tdf2_order2_cut5000_f32_ba[0], cheby1_tdf2_order2_cut5000_f32_ba[1], NULL, NULL, 3);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order2_cut5000_f32_sos", "float", CASCADE_f, NULL, *cheby1_cascade_order2_cut5000_f32_sos, NULL, 1);
        benchmark_and_log(fp, "cheby1_df1_order4_cut1000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby1_df1_order4_cut1000_f64_ba[0], cheby1_df1_order4_cut1000_f64_ba[1], 5);
        benchmark_and_log(fp, "cheby1_df2_order4_cut1000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby1_df2_order4_cut1000_f64_ba[0], cheby1_df2_order4_cut1000_f64_ba[1], 5);
        benchmark_and_log(fp, "cheby1_tdf2_order4_cut1000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby1_tdf2_order4_cut1000_f64_ba[0], cheby1_tdf2_order4_cut1000_f64_ba[1], 5);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order4_cut1000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby1_cascade_order4_cut1000_f64_sos, 2);
        benchmark_and_log(fp, "cheby1_df1_order4_cut1000_f32_ba", "float", "DF1", DF1_f, NULL, cheby1_df1_order4_cut1000_f32_ba[0], cheby1_df1_order4_cut1000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "cheby1_df2_order4_cut1000_f32_ba", "float", "DF2", DF2_f, NULL, cheby1_df2_order4_cut1000_f32_ba[0], cheby1_df2_order4_cut1000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "cheby1_tdf2_order4_cut1000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby1_tdf2_order4_cut1000_f32_ba[0], cheby1_tdf2_order4_cut1000_f32_ba[1], NULL, NULL, 5);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order4_cut1000_f32_sos", "float", CASCADE_f, NULL, *cheby1_cascade_order4_cut1000_f32_sos, NULL, 2);
        benchmark_and_log(fp, "cheby1_df1_order4_cut2000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby1_df1_order4_cut2000_f64_ba[0], cheby1_df1_order4_cut2000_f64_ba[1], 5);
        benchmark_and_log(fp, "cheby1_df2_order4_cut2000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby1_df2_order4_cut2000_f64_ba[0], cheby1_df2_order4_cut2000_f64_ba[1], 5);
        benchmark_and_log(fp, "cheby1_tdf2_order4_cut2000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby1_tdf2_order4_cut2000_f64_ba[0], cheby1_tdf2_order4_cut2000_f64_ba[1], 5);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order4_cut2000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby1_cascade_order4_cut2000_f64_sos, 2);
        benchmark_and_log(fp, "cheby1_df1_order4_cut2000_f32_ba", "float", "DF1", DF1_f, NULL, cheby1_df1_order4_cut2000_f32_ba[0], cheby1_df1_order4_cut2000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "cheby1_df2_order4_cut2000_f32_ba", "float", "DF2", DF2_f, NULL, cheby1_df2_order4_cut2000_f32_ba[0], cheby1_df2_order4_cut2000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "cheby1_tdf2_order4_cut2000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby1_tdf2_order4_cut2000_f32_ba[0], cheby1_tdf2_order4_cut2000_f32_ba[1], NULL, NULL, 5);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order4_cut2000_f32_sos", "float", CASCADE_f, NULL, *cheby1_cascade_order4_cut2000_f32_sos, NULL, 2);
        benchmark_and_log(fp, "cheby1_df1_order4_cut5000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby1_df1_order4_cut5000_f64_ba[0], cheby1_df1_order4_cut5000_f64_ba[1], 5);
        benchmark_and_log(fp, "cheby1_df2_order4_cut5000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby1_df2_order4_cut5000_f64_ba[0], cheby1_df2_order4_cut5000_f64_ba[1], 5);
        benchmark_and_log(fp, "cheby1_tdf2_order4_cut5000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby1_tdf2_order4_cut5000_f64_ba[0], cheby1_tdf2_order4_cut5000_f64_ba[1], 5);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order4_cut5000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby1_cascade_order4_cut5000_f64_sos, 2);
        benchmark_and_log(fp, "cheby1_df1_order4_cut5000_f32_ba", "float", "DF1", DF1_f, NULL, cheby1_df1_order4_cut5000_f32_ba[0], cheby1_df1_order4_cut5000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "cheby1_df2_order4_cut5000_f32_ba", "float", "DF2", DF2_f, NULL, cheby1_df2_order4_cut5000_f32_ba[0], cheby1_df2_order4_cut5000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "cheby1_tdf2_order4_cut5000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby1_tdf2_order4_cut5000_f32_ba[0], cheby1_tdf2_order4_cut5000_f32_ba[1], NULL, NULL, 5);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order4_cut5000_f32_sos", "float", CASCADE_f, NULL, *cheby1_cascade_order4_cut5000_f32_sos, NULL, 2);
        benchmark_and_log(fp, "cheby1_df1_order6_cut1000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby1_df1_order6_cut1000_f64_ba[0], cheby1_df1_order6_cut1000_f64_ba[1], 7);
        benchmark_and_log(fp, "cheby1_df2_order6_cut1000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby1_df2_order6_cut1000_f64_ba[0], cheby1_df2_order6_cut1000_f64_ba[1], 7);
        benchmark_and_log(fp, "cheby1_tdf2_order6_cut1000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby1_tdf2_order6_cut1000_f64_ba[0], cheby1_tdf2_order6_cut1000_f64_ba[1], 7);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order6_cut1000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby1_cascade_order6_cut1000_f64_sos, 3);
        benchmark_and_log(fp, "cheby1_df1_order6_cut1000_f32_ba", "float", "DF1", DF1_f, NULL, cheby1_df1_order6_cut1000_f32_ba[0], cheby1_df1_order6_cut1000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "cheby1_df2_order6_cut1000_f32_ba", "float", "DF2", DF2_f, NULL, cheby1_df2_order6_cut1000_f32_ba[0], cheby1_df2_order6_cut1000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "cheby1_tdf2_order6_cut1000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby1_tdf2_order6_cut1000_f32_ba[0], cheby1_tdf2_order6_cut1000_f32_ba[1], NULL, NULL, 7);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order6_cut1000_f32_sos", "float", CASCADE_f, NULL, *cheby1_cascade_order6_cut1000_f32_sos, NULL, 3);
        benchmark_and_log(fp, "cheby1_df1_order6_cut2000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby1_df1_order6_cut2000_f64_ba[0], cheby1_df1_order6_cut2000_f64_ba[1], 7);
        benchmark_and_log(fp, "cheby1_df2_order6_cut2000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby1_df2_order6_cut2000_f64_ba[0], cheby1_df2_order6_cut2000_f64_ba[1], 7);
        benchmark_and_log(fp, "cheby1_tdf2_order6_cut2000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby1_tdf2_order6_cut2000_f64_ba[0], cheby1_tdf2_order6_cut2000_f64_ba[1], 7);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order6_cut2000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby1_cascade_order6_cut2000_f64_sos, 3);
        benchmark_and_log(fp, "cheby1_df1_order6_cut2000_f32_ba", "float", "DF1", DF1_f, NULL, cheby1_df1_order6_cut2000_f32_ba[0], cheby1_df1_order6_cut2000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "cheby1_df2_order6_cut2000_f32_ba", "float", "DF2", DF2_f, NULL, cheby1_df2_order6_cut2000_f32_ba[0], cheby1_df2_order6_cut2000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "cheby1_tdf2_order6_cut2000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby1_tdf2_order6_cut2000_f32_ba[0], cheby1_tdf2_order6_cut2000_f32_ba[1], NULL, NULL, 7);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order6_cut2000_f32_sos", "float", CASCADE_f, NULL, *cheby1_cascade_order6_cut2000_f32_sos, NULL, 3);
        benchmark_and_log(fp, "cheby1_df1_order6_cut5000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby1_df1_order6_cut5000_f64_ba[0], cheby1_df1_order6_cut5000_f64_ba[1], 7);
        benchmark_and_log(fp, "cheby1_df2_order6_cut5000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby1_df2_order6_cut5000_f64_ba[0], cheby1_df2_order6_cut5000_f64_ba[1], 7);
        benchmark_and_log(fp, "cheby1_tdf2_order6_cut5000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby1_tdf2_order6_cut5000_f64_ba[0], cheby1_tdf2_order6_cut5000_f64_ba[1], 7);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order6_cut5000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby1_cascade_order6_cut5000_f64_sos, 3);
        benchmark_and_log(fp, "cheby1_df1_order6_cut5000_f32_ba", "float", "DF1", DF1_f, NULL, cheby1_df1_order6_cut5000_f32_ba[0], cheby1_df1_order6_cut5000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "cheby1_df2_order6_cut5000_f32_ba", "float", "DF2", DF2_f, NULL, cheby1_df2_order6_cut5000_f32_ba[0], cheby1_df2_order6_cut5000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "cheby1_tdf2_order6_cut5000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby1_tdf2_order6_cut5000_f32_ba[0], cheby1_tdf2_order6_cut5000_f32_ba[1], NULL, NULL, 7);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order6_cut5000_f32_sos", "float", CASCADE_f, NULL, *cheby1_cascade_order6_cut5000_f32_sos, NULL, 3);
        benchmark_and_log(fp, "cheby1_df1_order8_cut1000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby1_df1_order8_cut1000_f64_ba[0], cheby1_df1_order8_cut1000_f64_ba[1], 9);
        benchmark_and_log(fp, "cheby1_df2_order8_cut1000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby1_df2_order8_cut1000_f64_ba[0], cheby1_df2_order8_cut1000_f64_ba[1], 9);
        benchmark_and_log(fp, "cheby1_tdf2_order8_cut1000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby1_tdf2_order8_cut1000_f64_ba[0], cheby1_tdf2_order8_cut1000_f64_ba[1], 9);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order8_cut1000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby1_cascade_order8_cut1000_f64_sos, 4);
        benchmark_and_log(fp, "cheby1_df1_order8_cut1000_f32_ba", "float", "DF1", DF1_f, NULL, cheby1_df1_order8_cut1000_f32_ba[0], cheby1_df1_order8_cut1000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "cheby1_df2_order8_cut1000_f32_ba", "float", "DF2", DF2_f, NULL, cheby1_df2_order8_cut1000_f32_ba[0], cheby1_df2_order8_cut1000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "cheby1_tdf2_order8_cut1000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby1_tdf2_order8_cut1000_f32_ba[0], cheby1_tdf2_order8_cut1000_f32_ba[1], NULL, NULL, 9);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order8_cut1000_f32_sos", "float", CASCADE_f, NULL, *cheby1_cascade_order8_cut1000_f32_sos, NULL, 4);
        benchmark_and_log(fp, "cheby1_df1_order8_cut2000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby1_df1_order8_cut2000_f64_ba[0], cheby1_df1_order8_cut2000_f64_ba[1], 9);
        benchmark_and_log(fp, "cheby1_df2_order8_cut2000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby1_df2_order8_cut2000_f64_ba[0], cheby1_df2_order8_cut2000_f64_ba[1], 9);
        benchmark_and_log(fp, "cheby1_tdf2_order8_cut2000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby1_tdf2_order8_cut2000_f64_ba[0], cheby1_tdf2_order8_cut2000_f64_ba[1], 9);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order8_cut2000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby1_cascade_order8_cut2000_f64_sos, 4);
        benchmark_and_log(fp, "cheby1_df1_order8_cut2000_f32_ba", "float", "DF1", DF1_f, NULL, cheby1_df1_order8_cut2000_f32_ba[0], cheby1_df1_order8_cut2000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "cheby1_df2_order8_cut2000_f32_ba", "float", "DF2", DF2_f, NULL, cheby1_df2_order8_cut2000_f32_ba[0], cheby1_df2_order8_cut2000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "cheby1_tdf2_order8_cut2000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby1_tdf2_order8_cut2000_f32_ba[0], cheby1_tdf2_order8_cut2000_f32_ba[1], NULL, NULL, 9);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order8_cut2000_f32_sos", "float", CASCADE_f, NULL, *cheby1_cascade_order8_cut2000_f32_sos, NULL, 4);
        benchmark_and_log(fp, "cheby1_df1_order8_cut5000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby1_df1_order8_cut5000_f64_ba[0], cheby1_df1_order8_cut5000_f64_ba[1], 9);
        benchmark_and_log(fp, "cheby1_df2_order8_cut5000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby1_df2_order8_cut5000_f64_ba[0], cheby1_df2_order8_cut5000_f64_ba[1], 9);
        benchmark_and_log(fp, "cheby1_tdf2_order8_cut5000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby1_tdf2_order8_cut5000_f64_ba[0], cheby1_tdf2_order8_cut5000_f64_ba[1], 9);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order8_cut5000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby1_cascade_order8_cut5000_f64_sos, 4);
        benchmark_and_log(fp, "cheby1_df1_order8_cut5000_f32_ba", "float", "DF1", DF1_f, NULL, cheby1_df1_order8_cut5000_f32_ba[0], cheby1_df1_order8_cut5000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "cheby1_df2_order8_cut5000_f32_ba", "float", "DF2", DF2_f, NULL, cheby1_df2_order8_cut5000_f32_ba[0], cheby1_df2_order8_cut5000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "cheby1_tdf2_order8_cut5000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby1_tdf2_order8_cut5000_f32_ba[0], cheby1_tdf2_order8_cut5000_f32_ba[1], NULL, NULL, 9);
        benchmark_cascade_and_log(fp, "cheby1_cascade_order8_cut5000_f32_sos", "float", CASCADE_f, NULL, *cheby1_cascade_order8_cut5000_f32_sos, NULL, 4);
        benchmark_and_log(fp, "cheby2_df1_order2_cut1000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby2_df1_order2_cut1000_f64_ba[0], cheby2_df1_order2_cut1000_f64_ba[1], 3);
        benchmark_and_log(fp, "cheby2_df2_order2_cut1000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby2_df2_order2_cut1000_f64_ba[0], cheby2_df2_order2_cut1000_f64_ba[1], 3);
        benchmark_and_log(fp, "cheby2_tdf2_order2_cut1000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby2_tdf2_order2_cut1000_f64_ba[0], cheby2_tdf2_order2_cut1000_f64_ba[1], 3);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order2_cut1000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby2_cascade_order2_cut1000_f64_sos, 1);
        benchmark_and_log(fp, "cheby2_df1_order2_cut1000_f32_ba", "float", "DF1", DF1_f, NULL, cheby2_df1_order2_cut1000_f32_ba[0], cheby2_df1_order2_cut1000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "cheby2_df2_order2_cut1000_f32_ba", "float", "DF2", DF2_f, NULL, cheby2_df2_order2_cut1000_f32_ba[0], cheby2_df2_order2_cut1000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "cheby2_tdf2_order2_cut1000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby2_tdf2_order2_cut1000_f32_ba[0], cheby2_tdf2_order2_cut1000_f32_ba[1], NULL, NULL, 3);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order2_cut1000_f32_sos", "float", CASCADE_f, NULL, *cheby2_cascade_order2_cut1000_f32_sos, NULL, 1);
        benchmark_and_log(fp, "cheby2_df1_order2_cut2000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby2_df1_order2_cut2000_f64_ba[0], cheby2_df1_order2_cut2000_f64_ba[1], 3);
        benchmark_and_log(fp, "cheby2_df2_order2_cut2000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby2_df2_order2_cut2000_f64_ba[0], cheby2_df2_order2_cut2000_f64_ba[1], 3);
        benchmark_and_log(fp, "cheby2_tdf2_order2_cut2000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby2_tdf2_order2_cut2000_f64_ba[0], cheby2_tdf2_order2_cut2000_f64_ba[1], 3);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order2_cut2000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby2_cascade_order2_cut2000_f64_sos, 1);
        benchmark_and_log(fp, "cheby2_df1_order2_cut2000_f32_ba", "float", "DF1", DF1_f, NULL, cheby2_df1_order2_cut2000_f32_ba[0], cheby2_df1_order2_cut2000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "cheby2_df2_order2_cut2000_f32_ba", "float", "DF2", DF2_f, NULL, cheby2_df2_order2_cut2000_f32_ba[0], cheby2_df2_order2_cut2000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "cheby2_tdf2_order2_cut2000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby2_tdf2_order2_cut2000_f32_ba[0], cheby2_tdf2_order2_cut2000_f32_ba[1], NULL, NULL, 3);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order2_cut2000_f32_sos", "float", CASCADE_f, NULL, *cheby2_cascade_order2_cut2000_f32_sos, NULL, 1);
        benchmark_and_log(fp, "cheby2_df1_order2_cut5000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby2_df1_order2_cut5000_f64_ba[0], cheby2_df1_order2_cut5000_f64_ba[1], 3);
        benchmark_and_log(fp, "cheby2_df2_order2_cut5000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby2_df2_order2_cut5000_f64_ba[0], cheby2_df2_order2_cut5000_f64_ba[1], 3);
        benchmark_and_log(fp, "cheby2_tdf2_order2_cut5000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby2_tdf2_order2_cut5000_f64_ba[0], cheby2_tdf2_order2_cut5000_f64_ba[1], 3);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order2_cut5000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby2_cascade_order2_cut5000_f64_sos, 1);
        benchmark_and_log(fp, "cheby2_df1_order2_cut5000_f32_ba", "float", "DF1", DF1_f, NULL, cheby2_df1_order2_cut5000_f32_ba[0], cheby2_df1_order2_cut5000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "cheby2_df2_order2_cut5000_f32_ba", "float", "DF2", DF2_f, NULL, cheby2_df2_order2_cut5000_f32_ba[0], cheby2_df2_order2_cut5000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "cheby2_tdf2_order2_cut5000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby2_tdf2_order2_cut5000_f32_ba[0], cheby2_tdf2_order2_cut5000_f32_ba[1], NULL, NULL, 3);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order2_cut5000_f32_sos", "float", CASCADE_f, NULL, *cheby2_cascade_order2_cut5000_f32_sos, NULL, 1);
        benchmark_and_log(fp, "cheby2_df1_order4_cut1000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby2_df1_order4_cut1000_f64_ba[0], cheby2_df1_order4_cut1000_f64_ba[1], 5);
        benchmark_and_log(fp, "cheby2_df2_order4_cut1000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby2_df2_order4_cut1000_f64_ba[0], cheby2_df2_order4_cut1000_f64_ba[1], 5);
        benchmark_and_log(fp, "cheby2_tdf2_order4_cut1000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby2_tdf2_order4_cut1000_f64_ba[0], cheby2_tdf2_order4_cut1000_f64_ba[1], 5);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order4_cut1000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby2_cascade_order4_cut1000_f64_sos, 2);
        benchmark_and_log(fp, "cheby2_df1_order4_cut1000_f32_ba", "float", "DF1", DF1_f, NULL, cheby2_df1_order4_cut1000_f32_ba[0], cheby2_df1_order4_cut1000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "cheby2_df2_order4_cut1000_f32_ba", "float", "DF2", DF2_f, NULL, cheby2_df2_order4_cut1000_f32_ba[0], cheby2_df2_order4_cut1000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "cheby2_tdf2_order4_cut1000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby2_tdf2_order4_cut1000_f32_ba[0], cheby2_tdf2_order4_cut1000_f32_ba[1], NULL, NULL, 5);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order4_cut1000_f32_sos", "float", CASCADE_f, NULL, *cheby2_cascade_order4_cut1000_f32_sos, NULL, 2);
        benchmark_and_log(fp, "cheby2_df1_order4_cut2000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby2_df1_order4_cut2000_f64_ba[0], cheby2_df1_order4_cut2000_f64_ba[1], 5);
        benchmark_and_log(fp, "cheby2_df2_order4_cut2000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby2_df2_order4_cut2000_f64_ba[0], cheby2_df2_order4_cut2000_f64_ba[1], 5);
        benchmark_and_log(fp, "cheby2_tdf2_order4_cut2000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby2_tdf2_order4_cut2000_f64_ba[0], cheby2_tdf2_order4_cut2000_f64_ba[1], 5);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order4_cut2000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby2_cascade_order4_cut2000_f64_sos, 2);
        benchmark_and_log(fp, "cheby2_df1_order4_cut2000_f32_ba", "float", "DF1", DF1_f, NULL, cheby2_df1_order4_cut2000_f32_ba[0], cheby2_df1_order4_cut2000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "cheby2_df2_order4_cut2000_f32_ba", "float", "DF2", DF2_f, NULL, cheby2_df2_order4_cut2000_f32_ba[0], cheby2_df2_order4_cut2000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "cheby2_tdf2_order4_cut2000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby2_tdf2_order4_cut2000_f32_ba[0], cheby2_tdf2_order4_cut2000_f32_ba[1], NULL, NULL, 5);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order4_cut2000_f32_sos", "float", CASCADE_f, NULL, *cheby2_cascade_order4_cut2000_f32_sos, NULL, 2);
        benchmark_and_log(fp, "cheby2_df1_order4_cut5000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby2_df1_order4_cut5000_f64_ba[0], cheby2_df1_order4_cut5000_f64_ba[1], 5);
        benchmark_and_log(fp, "cheby2_df2_order4_cut5000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby2_df2_order4_cut5000_f64_ba[0], cheby2_df2_order4_cut5000_f64_ba[1], 5);
        benchmark_and_log(fp, "cheby2_tdf2_order4_cut5000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby2_tdf2_order4_cut5000_f64_ba[0], cheby2_tdf2_order4_cut5000_f64_ba[1], 5);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order4_cut5000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby2_cascade_order4_cut5000_f64_sos, 2);
        benchmark_and_log(fp, "cheby2_df1_order4_cut5000_f32_ba", "float", "DF1", DF1_f, NULL, cheby2_df1_order4_cut5000_f32_ba[0], cheby2_df1_order4_cut5000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "cheby2_df2_order4_cut5000_f32_ba", "float", "DF2", DF2_f, NULL, cheby2_df2_order4_cut5000_f32_ba[0], cheby2_df2_order4_cut5000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "cheby2_tdf2_order4_cut5000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby2_tdf2_order4_cut5000_f32_ba[0], cheby2_tdf2_order4_cut5000_f32_ba[1], NULL, NULL, 5);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order4_cut5000_f32_sos", "float", CASCADE_f, NULL, *cheby2_cascade_order4_cut5000_f32_sos, NULL, 2);
        benchmark_and_log(fp, "cheby2_df1_order6_cut1000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby2_df1_order6_cut1000_f64_ba[0], cheby2_df1_order6_cut1000_f64_ba[1], 7);
        benchmark_and_log(fp, "cheby2_df2_order6_cut1000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby2_df2_order6_cut1000_f64_ba[0], cheby2_df2_order6_cut1000_f64_ba[1], 7);
        benchmark_and_log(fp, "cheby2_tdf2_order6_cut1000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby2_tdf2_order6_cut1000_f64_ba[0], cheby2_tdf2_order6_cut1000_f64_ba[1], 7);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order6_cut1000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby2_cascade_order6_cut1000_f64_sos, 3);
        benchmark_and_log(fp, "cheby2_df1_order6_cut1000_f32_ba", "float", "DF1", DF1_f, NULL, cheby2_df1_order6_cut1000_f32_ba[0], cheby2_df1_order6_cut1000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "cheby2_df2_order6_cut1000_f32_ba", "float", "DF2", DF2_f, NULL, cheby2_df2_order6_cut1000_f32_ba[0], cheby2_df2_order6_cut1000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "cheby2_tdf2_order6_cut1000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby2_tdf2_order6_cut1000_f32_ba[0], cheby2_tdf2_order6_cut1000_f32_ba[1], NULL, NULL, 7);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order6_cut1000_f32_sos", "float", CASCADE_f, NULL, *cheby2_cascade_order6_cut1000_f32_sos, NULL, 3);
        benchmark_and_log(fp, "cheby2_df1_order6_cut2000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby2_df1_order6_cut2000_f64_ba[0], cheby2_df1_order6_cut2000_f64_ba[1], 7);
        benchmark_and_log(fp, "cheby2_df2_order6_cut2000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby2_df2_order6_cut2000_f64_ba[0], cheby2_df2_order6_cut2000_f64_ba[1], 7);
        benchmark_and_log(fp, "cheby2_tdf2_order6_cut2000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby2_tdf2_order6_cut2000_f64_ba[0], cheby2_tdf2_order6_cut2000_f64_ba[1], 7);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order6_cut2000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby2_cascade_order6_cut2000_f64_sos, 3);
        benchmark_and_log(fp, "cheby2_df1_order6_cut2000_f32_ba", "float", "DF1", DF1_f, NULL, cheby2_df1_order6_cut2000_f32_ba[0], cheby2_df1_order6_cut2000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "cheby2_df2_order6_cut2000_f32_ba", "float", "DF2", DF2_f, NULL, cheby2_df2_order6_cut2000_f32_ba[0], cheby2_df2_order6_cut2000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "cheby2_tdf2_order6_cut2000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby2_tdf2_order6_cut2000_f32_ba[0], cheby2_tdf2_order6_cut2000_f32_ba[1], NULL, NULL, 7);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order6_cut2000_f32_sos", "float", CASCADE_f, NULL, *cheby2_cascade_order6_cut2000_f32_sos, NULL, 3);
        benchmark_and_log(fp, "cheby2_df1_order6_cut5000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby2_df1_order6_cut5000_f64_ba[0], cheby2_df1_order6_cut5000_f64_ba[1], 7);
        benchmark_and_log(fp, "cheby2_df2_order6_cut5000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby2_df2_order6_cut5000_f64_ba[0], cheby2_df2_order6_cut5000_f64_ba[1], 7);
        benchmark_and_log(fp, "cheby2_tdf2_order6_cut5000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby2_tdf2_order6_cut5000_f64_ba[0], cheby2_tdf2_order6_cut5000_f64_ba[1], 7);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order6_cut5000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby2_cascade_order6_cut5000_f64_sos, 3);
        benchmark_and_log(fp, "cheby2_df1_order6_cut5000_f32_ba", "float", "DF1", DF1_f, NULL, cheby2_df1_order6_cut5000_f32_ba[0], cheby2_df1_order6_cut5000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "cheby2_df2_order6_cut5000_f32_ba", "float", "DF2", DF2_f, NULL, cheby2_df2_order6_cut5000_f32_ba[0], cheby2_df2_order6_cut5000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "cheby2_tdf2_order6_cut5000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby2_tdf2_order6_cut5000_f32_ba[0], cheby2_tdf2_order6_cut5000_f32_ba[1], NULL, NULL, 7);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order6_cut5000_f32_sos", "float", CASCADE_f, NULL, *cheby2_cascade_order6_cut5000_f32_sos, NULL, 3);
        benchmark_and_log(fp, "cheby2_df1_order8_cut1000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby2_df1_order8_cut1000_f64_ba[0], cheby2_df1_order8_cut1000_f64_ba[1], 9);
        benchmark_and_log(fp, "cheby2_df2_order8_cut1000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby2_df2_order8_cut1000_f64_ba[0], cheby2_df2_order8_cut1000_f64_ba[1], 9);
        benchmark_and_log(fp, "cheby2_tdf2_order8_cut1000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby2_tdf2_order8_cut1000_f64_ba[0], cheby2_tdf2_order8_cut1000_f64_ba[1], 9);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order8_cut1000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby2_cascade_order8_cut1000_f64_sos, 4);
        benchmark_and_log(fp, "cheby2_df1_order8_cut1000_f32_ba", "float", "DF1", DF1_f, NULL, cheby2_df1_order8_cut1000_f32_ba[0], cheby2_df1_order8_cut1000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "cheby2_df2_order8_cut1000_f32_ba", "float", "DF2", DF2_f, NULL, cheby2_df2_order8_cut1000_f32_ba[0], cheby2_df2_order8_cut1000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "cheby2_tdf2_order8_cut1000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby2_tdf2_order8_cut1000_f32_ba[0], cheby2_tdf2_order8_cut1000_f32_ba[1], NULL, NULL, 9);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order8_cut1000_f32_sos", "float", CASCADE_f, NULL, *cheby2_cascade_order8_cut1000_f32_sos, NULL, 4);
        benchmark_and_log(fp, "cheby2_df1_order8_cut2000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby2_df1_order8_cut2000_f64_ba[0], cheby2_df1_order8_cut2000_f64_ba[1], 9);
        benchmark_and_log(fp, "cheby2_df2_order8_cut2000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby2_df2_order8_cut2000_f64_ba[0], cheby2_df2_order8_cut2000_f64_ba[1], 9);
        benchmark_and_log(fp, "cheby2_tdf2_order8_cut2000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby2_tdf2_order8_cut2000_f64_ba[0], cheby2_tdf2_order8_cut2000_f64_ba[1], 9);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order8_cut2000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby2_cascade_order8_cut2000_f64_sos, 4);
        benchmark_and_log(fp, "cheby2_df1_order8_cut2000_f32_ba", "float", "DF1", DF1_f, NULL, cheby2_df1_order8_cut2000_f32_ba[0], cheby2_df1_order8_cut2000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "cheby2_df2_order8_cut2000_f32_ba", "float", "DF2", DF2_f, NULL, cheby2_df2_order8_cut2000_f32_ba[0], cheby2_df2_order8_cut2000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "cheby2_tdf2_order8_cut2000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby2_tdf2_order8_cut2000_f32_ba[0], cheby2_tdf2_order8_cut2000_f32_ba[1], NULL, NULL, 9);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order8_cut2000_f32_sos", "float", CASCADE_f, NULL, *cheby2_cascade_order8_cut2000_f32_sos, NULL, 4);
        benchmark_and_log(fp, "cheby2_df1_order8_cut5000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, cheby2_df1_order8_cut5000_f64_ba[0], cheby2_df1_order8_cut5000_f64_ba[1], 9);
        benchmark_and_log(fp, "cheby2_df2_order8_cut5000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, cheby2_df2_order8_cut5000_f64_ba[0], cheby2_df2_order8_cut5000_f64_ba[1], 9);
        benchmark_and_log(fp, "cheby2_tdf2_order8_cut5000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, cheby2_tdf2_order8_cut5000_f64_ba[0], cheby2_tdf2_order8_cut5000_f64_ba[1], 9);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order8_cut5000_f64_sos", "double", NULL, CASCADE_d, NULL, *cheby2_cascade_order8_cut5000_f64_sos, 4);
        benchmark_and_log(fp, "cheby2_df1_order8_cut5000_f32_ba", "float", "DF1", DF1_f, NULL, cheby2_df1_order8_cut5000_f32_ba[0], cheby2_df1_order8_cut5000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "cheby2_df2_order8_cut5000_f32_ba", "float", "DF2", DF2_f, NULL, cheby2_df2_order8_cut5000_f32_ba[0], cheby2_df2_order8_cut5000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "cheby2_tdf2_order8_cut5000_f32_ba", "float", "TDF2", TDF2_f, NULL, cheby2_tdf2_order8_cut5000_f32_ba[0], cheby2_tdf2_order8_cut5000_f32_ba[1], NULL, NULL, 9);
        benchmark_cascade_and_log(fp, "cheby2_cascade_order8_cut5000_f32_sos", "float", CASCADE_f, NULL, *cheby2_cascade_order8_cut5000_f32_sos, NULL, 4);
        benchmark_and_log(fp, "ellip_df1_order2_cut1000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, ellip_df1_order2_cut1000_f64_ba[0], ellip_df1_order2_cut1000_f64_ba[1], 3);
        benchmark_and_log(fp, "ellip_df2_order2_cut1000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, ellip_df2_order2_cut1000_f64_ba[0], ellip_df2_order2_cut1000_f64_ba[1], 3);
        benchmark_and_log(fp, "ellip_tdf2_order2_cut1000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, ellip_tdf2_order2_cut1000_f64_ba[0], ellip_tdf2_order2_cut1000_f64_ba[1], 3);
        benchmark_cascade_and_log(fp, "ellip_cascade_order2_cut1000_f64_sos", "double", NULL, CASCADE_d, NULL, *ellip_cascade_order2_cut1000_f64_sos, 1);
        benchmark_and_log(fp, "ellip_df1_order2_cut1000_f32_ba", "float", "DF1", DF1_f, NULL, ellip_df1_order2_cut1000_f32_ba[0], ellip_df1_order2_cut1000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "ellip_df2_order2_cut1000_f32_ba", "float", "DF2", DF2_f, NULL, ellip_df2_order2_cut1000_f32_ba[0], ellip_df2_order2_cut1000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "ellip_tdf2_order2_cut1000_f32_ba", "float", "TDF2", TDF2_f, NULL, ellip_tdf2_order2_cut1000_f32_ba[0], ellip_tdf2_order2_cut1000_f32_ba[1], NULL, NULL, 3);
        benchmark_cascade_and_log(fp, "ellip_cascade_order2_cut1000_f32_sos", "float", CASCADE_f, NULL, *ellip_cascade_order2_cut1000_f32_sos, NULL, 1);
        benchmark_and_log(fp, "ellip_df1_order2_cut2000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, ellip_df1_order2_cut2000_f64_ba[0], ellip_df1_order2_cut2000_f64_ba[1], 3);
        benchmark_and_log(fp, "ellip_df2_order2_cut2000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, ellip_df2_order2_cut2000_f64_ba[0], ellip_df2_order2_cut2000_f64_ba[1], 3);
        benchmark_and_log(fp, "ellip_tdf2_order2_cut2000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, ellip_tdf2_order2_cut2000_f64_ba[0], ellip_tdf2_order2_cut2000_f64_ba[1], 3);
        benchmark_cascade_and_log(fp, "ellip_cascade_order2_cut2000_f64_sos", "double", NULL, CASCADE_d, NULL, *ellip_cascade_order2_cut2000_f64_sos, 1);
        benchmark_and_log(fp, "ellip_df1_order2_cut2000_f32_ba", "float", "DF1", DF1_f, NULL, ellip_df1_order2_cut2000_f32_ba[0], ellip_df1_order2_cut2000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "ellip_df2_order2_cut2000_f32_ba", "float", "DF2", DF2_f, NULL, ellip_df2_order2_cut2000_f32_ba[0], ellip_df2_order2_cut2000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "ellip_tdf2_order2_cut2000_f32_ba", "float", "TDF2", TDF2_f, NULL, ellip_tdf2_order2_cut2000_f32_ba[0], ellip_tdf2_order2_cut2000_f32_ba[1], NULL, NULL, 3);
        benchmark_cascade_and_log(fp, "ellip_cascade_order2_cut2000_f32_sos", "float", CASCADE_f, NULL, *ellip_cascade_order2_cut2000_f32_sos, NULL, 1);
        benchmark_and_log(fp, "ellip_df1_order2_cut5000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, ellip_df1_order2_cut5000_f64_ba[0], ellip_df1_order2_cut5000_f64_ba[1], 3);
        benchmark_and_log(fp, "ellip_df2_order2_cut5000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, ellip_df2_order2_cut5000_f64_ba[0], ellip_df2_order2_cut5000_f64_ba[1], 3);
        benchmark_and_log(fp, "ellip_tdf2_order2_cut5000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, ellip_tdf2_order2_cut5000_f64_ba[0], ellip_tdf2_order2_cut5000_f64_ba[1], 3);
        benchmark_cascade_and_log(fp, "ellip_cascade_order2_cut5000_f64_sos", "double", NULL, CASCADE_d, NULL, *ellip_cascade_order2_cut5000_f64_sos, 1);
        benchmark_and_log(fp, "ellip_df1_order2_cut5000_f32_ba", "float", "DF1", DF1_f, NULL, ellip_df1_order2_cut5000_f32_ba[0], ellip_df1_order2_cut5000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "ellip_df2_order2_cut5000_f32_ba", "float", "DF2", DF2_f, NULL, ellip_df2_order2_cut5000_f32_ba[0], ellip_df2_order2_cut5000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "ellip_tdf2_order2_cut5000_f32_ba", "float", "TDF2", TDF2_f, NULL, ellip_tdf2_order2_cut5000_f32_ba[0], ellip_tdf2_order2_cut5000_f32_ba[1], NULL, NULL, 3);
        benchmark_cascade_and_log(fp, "ellip_cascade_order2_cut5000_f32_sos", "float", CASCADE_f, NULL, *ellip_cascade_order2_cut5000_f32_sos, NULL, 1);
        benchmark_and_log(fp, "ellip_df1_order4_cut1000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, ellip_df1_order4_cut1000_f64_ba[0], ellip_df1_order4_cut1000_f64_ba[1], 5);
        benchmark_and_log(fp, "ellip_df2_order4_cut1000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, ellip_df2_order4_cut1000_f64_ba[0], ellip_df2_order4_cut1000_f64_ba[1], 5);
        benchmark_and_log(fp, "ellip_tdf2_order4_cut1000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, ellip_tdf2_order4_cut1000_f64_ba[0], ellip_tdf2_order4_cut1000_f64_ba[1], 5);
        benchmark_cascade_and_log(fp, "ellip_cascade_order4_cut1000_f64_sos", "double", NULL, CASCADE_d, NULL, *ellip_cascade_order4_cut1000_f64_sos, 2);
        benchmark_and_log(fp, "ellip_df1_order4_cut1000_f32_ba", "float", "DF1", DF1_f, NULL, ellip_df1_order4_cut1000_f32_ba[0], ellip_df1_order4_cut1000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "ellip_df2_order4_cut1000_f32_ba", "float", "DF2", DF2_f, NULL, ellip_df2_order4_cut1000_f32_ba[0], ellip_df2_order4_cut1000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "ellip_tdf2_order4_cut1000_f32_ba", "float", "TDF2", TDF2_f, NULL, ellip_tdf2_order4_cut1000_f32_ba[0], ellip_tdf2_order4_cut1000_f32_ba[1], NULL, NULL, 5);
        benchmark_cascade_and_log(fp, "ellip_cascade_order4_cut1000_f32_sos", "float", CASCADE_f, NULL, *ellip_cascade_order4_cut1000_f32_sos, NULL, 2);
        benchmark_and_log(fp, "ellip_df1_order4_cut2000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, ellip_df1_order4_cut2000_f64_ba[0], ellip_df1_order4_cut2000_f64_ba[1], 5);
        benchmark_and_log(fp, "ellip_df2_order4_cut2000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, ellip_df2_order4_cut2000_f64_ba[0], ellip_df2_order4_cut2000_f64_ba[1], 5);
        benchmark_and_log(fp, "ellip_tdf2_order4_cut2000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, ellip_tdf2_order4_cut2000_f64_ba[0], ellip_tdf2_order4_cut2000_f64_ba[1], 5);
        benchmark_cascade_and_log(fp, "ellip_cascade_order4_cut2000_f64_sos", "double", NULL, CASCADE_d, NULL, *ellip_cascade_order4_cut2000_f64_sos, 2);
        benchmark_and_log(fp, "ellip_df1_order4_cut2000_f32_ba", "float", "DF1", DF1_f, NULL, ellip_df1_order4_cut2000_f32_ba[0], ellip_df1_order4_cut2000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "ellip_df2_order4_cut2000_f32_ba", "float", "DF2", DF2_f, NULL, ellip_df2_order4_cut2000_f32_ba[0], ellip_df2_order4_cut2000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "ellip_tdf2_order4_cut2000_f32_ba", "float", "TDF2", TDF2_f, NULL, ellip_tdf2_order4_cut2000_f32_ba[0], ellip_tdf2_order4_cut2000_f32_ba[1], NULL, NULL, 5);
        benchmark_cascade_and_log(fp, "ellip_cascade_order4_cut2000_f32_sos", "float", CASCADE_f, NULL, *ellip_cascade_order4_cut2000_f32_sos, NULL, 2);
        benchmark_and_log(fp, "ellip_df1_order4_cut5000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, ellip_df1_order4_cut5000_f64_ba[0], ellip_df1_order4_cut5000_f64_ba[1], 5);
        benchmark_and_log(fp, "ellip_df2_order4_cut5000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, ellip_df2_order4_cut5000_f64_ba[0], ellip_df2_order4_cut5000_f64_ba[1], 5);
        benchmark_and_log(fp, "ellip_tdf2_order4_cut5000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, ellip_tdf2_order4_cut5000_f64_ba[0], ellip_tdf2_order4_cut5000_f64_ba[1], 5);
        benchmark_cascade_and_log(fp, "ellip_cascade_order4_cut5000_f64_sos", "double", NULL, CASCADE_d, NULL, *ellip_cascade_order4_cut5000_f64_sos, 2);
        benchmark_and_log(fp, "ellip_df1_order4_cut5000_f32_ba", "float", "DF1", DF1_f, NULL, ellip_df1_order4_cut5000_f32_ba[0], ellip_df1_order4_cut5000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "ellip_df2_order4_cut5000_f32_ba", "float", "DF2", DF2_f, NULL, ellip_df2_order4_cut5000_f32_ba[0], ellip_df2_order4_cut5000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "ellip_tdf2_order4_cut5000_f32_ba", "float", "TDF2", TDF2_f, NULL, ellip_tdf2_order4_cut5000_f32_ba[0], ellip_tdf2_order4_cut5000_f32_ba[1], NULL, NULL, 5);
        benchmark_cascade_and_log(fp, "ellip_cascade_order4_cut5000_f32_sos", "float", CASCADE_f, NULL, *ellip_cascade_order4_cut5000_f32_sos, NULL, 2);
        benchmark_and_log(fp, "ellip_df1_order6_cut1000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, ellip_df1_order6_cut1000_f64_ba[0], ellip_df1_order6_cut1000_f64_ba[1], 7);
        benchmark_and_log(fp, "ellip_df2_order6_cut1000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, ellip_df2_order6_cut1000_f64_ba[0], ellip_df2_order6_cut1000_f64_ba[1], 7);
        benchmark_and_log(fp, "ellip_tdf2_order6_cut1000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, ellip_tdf2_order6_cut1000_f64_ba[0], ellip_tdf2_order6_cut1000_f64_ba[1], 7);
        benchmark_cascade_and_log(fp, "ellip_cascade_order6_cut1000_f64_sos", "double", NULL, CASCADE_d, NULL, *ellip_cascade_order6_cut1000_f64_sos, 3);
        benchmark_and_log(fp, "ellip_df1_order6_cut1000_f32_ba", "float", "DF1", DF1_f, NULL, ellip_df1_order6_cut1000_f32_ba[0], ellip_df1_order6_cut1000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "ellip_df2_order6_cut1000_f32_ba", "float", "DF2", DF2_f, NULL, ellip_df2_order6_cut1000_f32_ba[0], ellip_df2_order6_cut1000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "ellip_tdf2_order6_cut1000_f32_ba", "float", "TDF2", TDF2_f, NULL, ellip_tdf2_order6_cut1000_f32_ba[0], ellip_tdf2_order6_cut1000_f32_ba[1], NULL, NULL, 7);
        benchmark_cascade_and_log(fp, "ellip_cascade_order6_cut1000_f32_sos", "float", CASCADE_f, NULL, *ellip_cascade_order6_cut1000_f32_sos, NULL, 3);
        benchmark_and_log(fp, "ellip_df1_order6_cut2000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, ellip_df1_order6_cut2000_f64_ba[0], ellip_df1_order6_cut2000_f64_ba[1], 7);
        benchmark_and_log(fp, "ellip_df2_order6_cut2000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, ellip_df2_order6_cut2000_f64_ba[0], ellip_df2_order6_cut2000_f64_ba[1], 7);
        benchmark_and_log(fp, "ellip_tdf2_order6_cut2000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, ellip_tdf2_order6_cut2000_f64_ba[0], ellip_tdf2_order6_cut2000_f64_ba[1], 7);
        benchmark_cascade_and_log(fp, "ellip_cascade_order6_cut2000_f64_sos", "double", NULL, CASCADE_d, NULL, *ellip_cascade_order6_cut2000_f64_sos, 3);
        benchmark_and_log(fp, "ellip_df1_order6_cut2000_f32_ba", "float", "DF1", DF1_f, NULL, ellip_df1_order6_cut2000_f32_ba[0], ellip_df1_order6_cut2000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "ellip_df2_order6_cut2000_f32_ba", "float", "DF2", DF2_f, NULL, ellip_df2_order6_cut2000_f32_ba[0], ellip_df2_order6_cut2000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "ellip_tdf2_order6_cut2000_f32_ba", "float", "TDF2", TDF2_f, NULL, ellip_tdf2_order6_cut2000_f32_ba[0], ellip_tdf2_order6_cut2000_f32_ba[1], NULL, NULL, 7);
        benchmark_cascade_and_log(fp, "ellip_cascade_order6_cut2000_f32_sos", "float", CASCADE_f, NULL, *ellip_cascade_order6_cut2000_f32_sos, NULL, 3);
        benchmark_and_log(fp, "ellip_df1_order6_cut5000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, ellip_df1_order6_cut5000_f64_ba[0], ellip_df1_order6_cut5000_f64_ba[1], 7);
        benchmark_and_log(fp, "ellip_df2_order6_cut5000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, ellip_df2_order6_cut5000_f64_ba[0], ellip_df2_order6_cut5000_f64_ba[1], 7);
        benchmark_and_log(fp, "ellip_tdf2_order6_cut5000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, ellip_tdf2_order6_cut5000_f64_ba[0], ellip_tdf2_order6_cut5000_f64_ba[1], 7);
        benchmark_cascade_and_log(fp, "ellip_cascade_order6_cut5000_f64_sos", "double", NULL, CASCADE_d, NULL, *ellip_cascade_order6_cut5000_f64_sos, 3);
        benchmark_and_log(fp, "ellip_df1_order6_cut5000_f32_ba", "float", "DF1", DF1_f, NULL, ellip_df1_order6_cut5000_f32_ba[0], ellip_df1_order6_cut5000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "ellip_df2_order6_cut5000_f32_ba", "float", "DF2", DF2_f, NULL, ellip_df2_order6_cut5000_f32_ba[0], ellip_df2_order6_cut5000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "ellip_tdf2_order6_cut5000_f32_ba", "float", "TDF2", TDF2_f, NULL, ellip_tdf2_order6_cut5000_f32_ba[0], ellip_tdf2_order6_cut5000_f32_ba[1], NULL, NULL, 7);
        benchmark_cascade_and_log(fp, "ellip_cascade_order6_cut5000_f32_sos", "float", CASCADE_f, NULL, *ellip_cascade_order6_cut5000_f32_sos, NULL, 3);
        benchmark_and_log(fp, "ellip_df1_order8_cut1000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, ellip_df1_order8_cut1000_f64_ba[0], ellip_df1_order8_cut1000_f64_ba[1], 9);
        benchmark_and_log(fp, "ellip_df2_order8_cut1000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, ellip_df2_order8_cut1000_f64_ba[0], ellip_df2_order8_cut1000_f64_ba[1], 9);
        benchmark_and_log(fp, "ellip_tdf2_order8_cut1000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, ellip_tdf2_order8_cut1000_f64_ba[0], ellip_tdf2_order8_cut1000_f64_ba[1], 9);
        benchmark_cascade_and_log(fp, "ellip_cascade_order8_cut1000_f64_sos", "double", NULL, CASCADE_d, NULL, *ellip_cascade_order8_cut1000_f64_sos, 4);
        benchmark_and_log(fp, "ellip_df1_order8_cut1000_f32_ba", "float", "DF1", DF1_f, NULL, ellip_df1_order8_cut1000_f32_ba[0], ellip_df1_order8_cut1000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "ellip_df2_order8_cut1000_f32_ba", "float", "DF2", DF2_f, NULL, ellip_df2_order8_cut1000_f32_ba[0], ellip_df2_order8_cut1000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "ellip_tdf2_order8_cut1000_f32_ba", "float", "TDF2", TDF2_f, NULL, ellip_tdf2_order8_cut1000_f32_ba[0], ellip_tdf2_order8_cut1000_f32_ba[1], NULL, NULL, 9);
        benchmark_cascade_and_log(fp, "ellip_cascade_order8_cut1000_f32_sos", "float", CASCADE_f, NULL, *ellip_cascade_order8_cut1000_f32_sos, NULL, 4);
        benchmark_and_log(fp, "ellip_df1_order8_cut2000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, ellip_df1_order8_cut2000_f64_ba[0], ellip_df1_order8_cut2000_f64_ba[1], 9);
        benchmark_and_log(fp, "ellip_df2_order8_cut2000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, ellip_df2_order8_cut2000_f64_ba[0], ellip_df2_order8_cut2000_f64_ba[1], 9);
        benchmark_and_log(fp, "ellip_tdf2_order8_cut2000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, ellip_tdf2_order8_cut2000_f64_ba[0], ellip_tdf2_order8_cut2000_f64_ba[1], 9);
        benchmark_cascade_and_log(fp, "ellip_cascade_order8_cut2000_f64_sos", "double", NULL, CASCADE_d, NULL, *ellip_cascade_order8_cut2000_f64_sos, 4);
        benchmark_and_log(fp, "ellip_df1_order8_cut2000_f32_ba", "float", "DF1", DF1_f, NULL, ellip_df1_order8_cut2000_f32_ba[0], ellip_df1_order8_cut2000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "ellip_df2_order8_cut2000_f32_ba", "float", "DF2", DF2_f, NULL, ellip_df2_order8_cut2000_f32_ba[0], ellip_df2_order8_cut2000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "ellip_tdf2_order8_cut2000_f32_ba", "float", "TDF2", TDF2_f, NULL, ellip_tdf2_order8_cut2000_f32_ba[0], ellip_tdf2_order8_cut2000_f32_ba[1], NULL, NULL, 9);
        benchmark_cascade_and_log(fp, "ellip_cascade_order8_cut2000_f32_sos", "float", CASCADE_f, NULL, *ellip_cascade_order8_cut2000_f32_sos, NULL, 4);
        benchmark_and_log(fp, "ellip_df1_order8_cut5000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, ellip_df1_order8_cut5000_f64_ba[0], ellip_df1_order8_cut5000_f64_ba[1], 9);
        benchmark_and_log(fp, "ellip_df2_order8_cut5000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, ellip_df2_order8_cut5000_f64_ba[0], ellip_df2_order8_cut5000_f64_ba[1], 9);
        benchmark_and_log(fp, "ellip_tdf2_order8_cut5000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, ellip_tdf2_order8_cut5000_f64_ba[0], ellip_tdf2_order8_cut5000_f64_ba[1], 9);
        benchmark_cascade_and_log(fp, "ellip_cascade_order8_cut5000_f64_sos", "double", NULL, CASCADE_d, NULL, *ellip_cascade_order8_cut5000_f64_sos, 4);
        benchmark_and_log(fp, "ellip_df1_order8_cut5000_f32_ba", "float", "DF1", DF1_f, NULL, ellip_df1_order8_cut5000_f32_ba[0], ellip_df1_order8_cut5000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "ellip_df2_order8_cut5000_f32_ba", "float", "DF2", DF2_f, NULL, ellip_df2_order8_cut5000_f32_ba[0], ellip_df2_order8_cut5000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "ellip_tdf2_order8_cut5000_f32_ba", "float", "TDF2", TDF2_f, NULL, ellip_tdf2_order8_cut5000_f32_ba[0], ellip_tdf2_order8_cut5000_f32_ba[1], NULL, NULL, 9);
        benchmark_cascade_and_log(fp, "ellip_cascade_order8_cut5000_f32_sos", "float", CASCADE_f, NULL, *ellip_cascade_order8_cut5000_f32_sos, NULL, 4);
        benchmark_and_log(fp, "bessel_df1_order2_cut1000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, bessel_df1_order2_cut1000_f64_ba[0], bessel_df1_order2_cut1000_f64_ba[1], 3);
        benchmark_and_log(fp, "bessel_df2_order2_cut1000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, bessel_df2_order2_cut1000_f64_ba[0], bessel_df2_order2_cut1000_f64_ba[1], 3);
        benchmark_and_log(fp, "bessel_tdf2_order2_cut1000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, bessel_tdf2_order2_cut1000_f64_ba[0], bessel_tdf2_order2_cut1000_f64_ba[1], 3);
        benchmark_cascade_and_log(fp, "bessel_cascade_order2_cut1000_f64_sos", "double", NULL, CASCADE_d, NULL, *bessel_cascade_order2_cut1000_f64_sos, 1);
        benchmark_and_log(fp, "bessel_df1_order2_cut1000_f32_ba", "float", "DF1", DF1_f, NULL, bessel_df1_order2_cut1000_f32_ba[0], bessel_df1_order2_cut1000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "bessel_df2_order2_cut1000_f32_ba", "float", "DF2", DF2_f, NULL, bessel_df2_order2_cut1000_f32_ba[0], bessel_df2_order2_cut1000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "bessel_tdf2_order2_cut1000_f32_ba", "float", "TDF2", TDF2_f, NULL, bessel_tdf2_order2_cut1000_f32_ba[0], bessel_tdf2_order2_cut1000_f32_ba[1], NULL, NULL, 3);
        benchmark_cascade_and_log(fp, "bessel_cascade_order2_cut1000_f32_sos", "float", CASCADE_f, NULL, *bessel_cascade_order2_cut1000_f32_sos, NULL, 1);
        benchmark_and_log(fp, "bessel_df1_order2_cut2000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, bessel_df1_order2_cut2000_f64_ba[0], bessel_df1_order2_cut2000_f64_ba[1], 3);
        benchmark_and_log(fp, "bessel_df2_order2_cut2000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, bessel_df2_order2_cut2000_f64_ba[0], bessel_df2_order2_cut2000_f64_ba[1], 3);
        benchmark_and_log(fp, "bessel_tdf2_order2_cut2000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, bessel_tdf2_order2_cut2000_f64_ba[0], bessel_tdf2_order2_cut2000_f64_ba[1], 3);
        benchmark_cascade_and_log(fp, "bessel_cascade_order2_cut2000_f64_sos", "double", NULL, CASCADE_d, NULL, *bessel_cascade_order2_cut2000_f64_sos, 1);
        benchmark_and_log(fp, "bessel_df1_order2_cut2000_f32_ba", "float", "DF1", DF1_f, NULL, bessel_df1_order2_cut2000_f32_ba[0], bessel_df1_order2_cut2000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "bessel_df2_order2_cut2000_f32_ba", "float", "DF2", DF2_f, NULL, bessel_df2_order2_cut2000_f32_ba[0], bessel_df2_order2_cut2000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "bessel_tdf2_order2_cut2000_f32_ba", "float", "TDF2", TDF2_f, NULL, bessel_tdf2_order2_cut2000_f32_ba[0], bessel_tdf2_order2_cut2000_f32_ba[1], NULL, NULL, 3);
        benchmark_cascade_and_log(fp, "bessel_cascade_order2_cut2000_f32_sos", "float", CASCADE_f, NULL, *bessel_cascade_order2_cut2000_f32_sos, NULL, 1);
        benchmark_and_log(fp, "bessel_df1_order2_cut5000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, bessel_df1_order2_cut5000_f64_ba[0], bessel_df1_order2_cut5000_f64_ba[1], 3);
        benchmark_and_log(fp, "bessel_df2_order2_cut5000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, bessel_df2_order2_cut5000_f64_ba[0], bessel_df2_order2_cut5000_f64_ba[1], 3);
        benchmark_and_log(fp, "bessel_tdf2_order2_cut5000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, bessel_tdf2_order2_cut5000_f64_ba[0], bessel_tdf2_order2_cut5000_f64_ba[1], 3);
        benchmark_cascade_and_log(fp, "bessel_cascade_order2_cut5000_f64_sos", "double", NULL, CASCADE_d, NULL, *bessel_cascade_order2_cut5000_f64_sos, 1);
        benchmark_and_log(fp, "bessel_df1_order2_cut5000_f32_ba", "float", "DF1", DF1_f, NULL, bessel_df1_order2_cut5000_f32_ba[0], bessel_df1_order2_cut5000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "bessel_df2_order2_cut5000_f32_ba", "float", "DF2", DF2_f, NULL, bessel_df2_order2_cut5000_f32_ba[0], bessel_df2_order2_cut5000_f32_ba[1], NULL, NULL, 3);
        benchmark_and_log(fp, "bessel_tdf2_order2_cut5000_f32_ba", "float", "TDF2", TDF2_f, NULL, bessel_tdf2_order2_cut5000_f32_ba[0], bessel_tdf2_order2_cut5000_f32_ba[1], NULL, NULL, 3);
        benchmark_cascade_and_log(fp, "bessel_cascade_order2_cut5000_f32_sos", "float", CASCADE_f, NULL, *bessel_cascade_order2_cut5000_f32_sos, NULL, 1);
        benchmark_and_log(fp, "bessel_df1_order4_cut1000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, bessel_df1_order4_cut1000_f64_ba[0], bessel_df1_order4_cut1000_f64_ba[1], 5);
        benchmark_and_log(fp, "bessel_df2_order4_cut1000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, bessel_df2_order4_cut1000_f64_ba[0], bessel_df2_order4_cut1000_f64_ba[1], 5);
        benchmark_and_log(fp, "bessel_tdf2_order4_cut1000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, bessel_tdf2_order4_cut1000_f64_ba[0], bessel_tdf2_order4_cut1000_f64_ba[1], 5);
        benchmark_cascade_and_log(fp, "bessel_cascade_order4_cut1000_f64_sos", "double", NULL, CASCADE_d, NULL, *bessel_cascade_order4_cut1000_f64_sos, 2);
        benchmark_and_log(fp, "bessel_df1_order4_cut1000_f32_ba", "float", "DF1", DF1_f, NULL, bessel_df1_order4_cut1000_f32_ba[0], bessel_df1_order4_cut1000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "bessel_df2_order4_cut1000_f32_ba", "float", "DF2", DF2_f, NULL, bessel_df2_order4_cut1000_f32_ba[0], bessel_df2_order4_cut1000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "bessel_tdf2_order4_cut1000_f32_ba", "float", "TDF2", TDF2_f, NULL, bessel_tdf2_order4_cut1000_f32_ba[0], bessel_tdf2_order4_cut1000_f32_ba[1], NULL, NULL, 5);
        benchmark_cascade_and_log(fp, "bessel_cascade_order4_cut1000_f32_sos", "float", CASCADE_f, NULL, *bessel_cascade_order4_cut1000_f32_sos, NULL, 2);
        benchmark_and_log(fp, "bessel_df1_order4_cut2000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, bessel_df1_order4_cut2000_f64_ba[0], bessel_df1_order4_cut2000_f64_ba[1], 5);
        benchmark_and_log(fp, "bessel_df2_order4_cut2000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, bessel_df2_order4_cut2000_f64_ba[0], bessel_df2_order4_cut2000_f64_ba[1], 5);
        benchmark_and_log(fp, "bessel_tdf2_order4_cut2000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, bessel_tdf2_order4_cut2000_f64_ba[0], bessel_tdf2_order4_cut2000_f64_ba[1], 5);
        benchmark_cascade_and_log(fp, "bessel_cascade_order4_cut2000_f64_sos", "double", NULL, CASCADE_d, NULL, *bessel_cascade_order4_cut2000_f64_sos, 2);
        benchmark_and_log(fp, "bessel_df1_order4_cut2000_f32_ba", "float", "DF1", DF1_f, NULL, bessel_df1_order4_cut2000_f32_ba[0], bessel_df1_order4_cut2000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "bessel_df2_order4_cut2000_f32_ba", "float", "DF2", DF2_f, NULL, bessel_df2_order4_cut2000_f32_ba[0], bessel_df2_order4_cut2000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "bessel_tdf2_order4_cut2000_f32_ba", "float", "TDF2", TDF2_f, NULL, bessel_tdf2_order4_cut2000_f32_ba[0], bessel_tdf2_order4_cut2000_f32_ba[1], NULL, NULL, 5);
        benchmark_cascade_and_log(fp, "bessel_cascade_order4_cut2000_f32_sos", "float", CASCADE_f, NULL, *bessel_cascade_order4_cut2000_f32_sos, NULL, 2);
        benchmark_and_log(fp, "bessel_df1_order4_cut5000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, bessel_df1_order4_cut5000_f64_ba[0], bessel_df1_order4_cut5000_f64_ba[1], 5);
        benchmark_and_log(fp, "bessel_df2_order4_cut5000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, bessel_df2_order4_cut5000_f64_ba[0], bessel_df2_order4_cut5000_f64_ba[1], 5);
        benchmark_and_log(fp, "bessel_tdf2_order4_cut5000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, bessel_tdf2_order4_cut5000_f64_ba[0], bessel_tdf2_order4_cut5000_f64_ba[1], 5);
        benchmark_cascade_and_log(fp, "bessel_cascade_order4_cut5000_f64_sos", "double", NULL, CASCADE_d, NULL, *bessel_cascade_order4_cut5000_f64_sos, 2);
        benchmark_and_log(fp, "bessel_df1_order4_cut5000_f32_ba", "float", "DF1", DF1_f, NULL, bessel_df1_order4_cut5000_f32_ba[0], bessel_df1_order4_cut5000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "bessel_df2_order4_cut5000_f32_ba", "float", "DF2", DF2_f, NULL, bessel_df2_order4_cut5000_f32_ba[0], bessel_df2_order4_cut5000_f32_ba[1], NULL, NULL, 5);
        benchmark_and_log(fp, "bessel_tdf2_order4_cut5000_f32_ba", "float", "TDF2", TDF2_f, NULL, bessel_tdf2_order4_cut5000_f32_ba[0], bessel_tdf2_order4_cut5000_f32_ba[1], NULL, NULL, 5);
        benchmark_cascade_and_log(fp, "bessel_cascade_order4_cut5000_f32_sos", "float", CASCADE_f, NULL, *bessel_cascade_order4_cut5000_f32_sos, NULL, 2);
        benchmark_and_log(fp, "bessel_df1_order6_cut1000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, bessel_df1_order6_cut1000_f64_ba[0], bessel_df1_order6_cut1000_f64_ba[1], 7);
        benchmark_and_log(fp, "bessel_df2_order6_cut1000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, bessel_df2_order6_cut1000_f64_ba[0], bessel_df2_order6_cut1000_f64_ba[1], 7);
        benchmark_and_log(fp, "bessel_tdf2_order6_cut1000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, bessel_tdf2_order6_cut1000_f64_ba[0], bessel_tdf2_order6_cut1000_f64_ba[1], 7);
        benchmark_cascade_and_log(fp, "bessel_cascade_order6_cut1000_f64_sos", "double", NULL, CASCADE_d, NULL, *bessel_cascade_order6_cut1000_f64_sos, 3);
        benchmark_and_log(fp, "bessel_df1_order6_cut1000_f32_ba", "float", "DF1", DF1_f, NULL, bessel_df1_order6_cut1000_f32_ba[0], bessel_df1_order6_cut1000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "bessel_df2_order6_cut1000_f32_ba", "float", "DF2", DF2_f, NULL, bessel_df2_order6_cut1000_f32_ba[0], bessel_df2_order6_cut1000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "bessel_tdf2_order6_cut1000_f32_ba", "float", "TDF2", TDF2_f, NULL, bessel_tdf2_order6_cut1000_f32_ba[0], bessel_tdf2_order6_cut1000_f32_ba[1], NULL, NULL, 7);
        benchmark_cascade_and_log(fp, "bessel_cascade_order6_cut1000_f32_sos", "float", CASCADE_f, NULL, *bessel_cascade_order6_cut1000_f32_sos, NULL, 3);
        benchmark_and_log(fp, "bessel_df1_order6_cut2000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, bessel_df1_order6_cut2000_f64_ba[0], bessel_df1_order6_cut2000_f64_ba[1], 7);
        benchmark_and_log(fp, "bessel_df2_order6_cut2000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, bessel_df2_order6_cut2000_f64_ba[0], bessel_df2_order6_cut2000_f64_ba[1], 7);
        benchmark_and_log(fp, "bessel_tdf2_order6_cut2000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, bessel_tdf2_order6_cut2000_f64_ba[0], bessel_tdf2_order6_cut2000_f64_ba[1], 7);
        benchmark_cascade_and_log(fp, "bessel_cascade_order6_cut2000_f64_sos", "double", NULL, CASCADE_d, NULL, *bessel_cascade_order6_cut2000_f64_sos, 3);
        benchmark_and_log(fp, "bessel_df1_order6_cut2000_f32_ba", "float", "DF1", DF1_f, NULL, bessel_df1_order6_cut2000_f32_ba[0], bessel_df1_order6_cut2000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "bessel_df2_order6_cut2000_f32_ba", "float", "DF2", DF2_f, NULL, bessel_df2_order6_cut2000_f32_ba[0], bessel_df2_order6_cut2000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "bessel_tdf2_order6_cut2000_f32_ba", "float", "TDF2", TDF2_f, NULL, bessel_tdf2_order6_cut2000_f32_ba[0], bessel_tdf2_order6_cut2000_f32_ba[1], NULL, NULL, 7);
        benchmark_cascade_and_log(fp, "bessel_cascade_order6_cut2000_f32_sos", "float", CASCADE_f, NULL, *bessel_cascade_order6_cut2000_f32_sos, NULL, 3);
        benchmark_and_log(fp, "bessel_df1_order6_cut5000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, bessel_df1_order6_cut5000_f64_ba[0], bessel_df1_order6_cut5000_f64_ba[1], 7);
        benchmark_and_log(fp, "bessel_df2_order6_cut5000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, bessel_df2_order6_cut5000_f64_ba[0], bessel_df2_order6_cut5000_f64_ba[1], 7);
        benchmark_and_log(fp, "bessel_tdf2_order6_cut5000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, bessel_tdf2_order6_cut5000_f64_ba[0], bessel_tdf2_order6_cut5000_f64_ba[1], 7);
        benchmark_cascade_and_log(fp, "bessel_cascade_order6_cut5000_f64_sos", "double", NULL, CASCADE_d, NULL, *bessel_cascade_order6_cut5000_f64_sos, 3);
        benchmark_and_log(fp, "bessel_df1_order6_cut5000_f32_ba", "float", "DF1", DF1_f, NULL, bessel_df1_order6_cut5000_f32_ba[0], bessel_df1_order6_cut5000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "bessel_df2_order6_cut5000_f32_ba", "float", "DF2", DF2_f, NULL, bessel_df2_order6_cut5000_f32_ba[0], bessel_df2_order6_cut5000_f32_ba[1], NULL, NULL, 7);
        benchmark_and_log(fp, "bessel_tdf2_order6_cut5000_f32_ba", "float", "TDF2", TDF2_f, NULL, bessel_tdf2_order6_cut5000_f32_ba[0], bessel_tdf2_order6_cut5000_f32_ba[1], NULL, NULL, 7);
        benchmark_cascade_and_log(fp, "bessel_cascade_order6_cut5000_f32_sos", "float", CASCADE_f, NULL, *bessel_cascade_order6_cut5000_f32_sos, NULL, 3);
        benchmark_and_log(fp, "bessel_df1_order8_cut1000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, bessel_df1_order8_cut1000_f64_ba[0], bessel_df1_order8_cut1000_f64_ba[1], 9);
        benchmark_and_log(fp, "bessel_df2_order8_cut1000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, bessel_df2_order8_cut1000_f64_ba[0], bessel_df2_order8_cut1000_f64_ba[1], 9);
        benchmark_and_log(fp, "bessel_tdf2_order8_cut1000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, bessel_tdf2_order8_cut1000_f64_ba[0], bessel_tdf2_order8_cut1000_f64_ba[1], 9);
        benchmark_cascade_and_log(fp, "bessel_cascade_order8_cut1000_f64_sos", "double", NULL, CASCADE_d, NULL, *bessel_cascade_order8_cut1000_f64_sos, 4);
        benchmark_and_log(fp, "bessel_df1_order8_cut1000_f32_ba", "float", "DF1", DF1_f, NULL, bessel_df1_order8_cut1000_f32_ba[0], bessel_df1_order8_cut1000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "bessel_df2_order8_cut1000_f32_ba", "float", "DF2", DF2_f, NULL, bessel_df2_order8_cut1000_f32_ba[0], bessel_df2_order8_cut1000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "bessel_tdf2_order8_cut1000_f32_ba", "float", "TDF2", TDF2_f, NULL, bessel_tdf2_order8_cut1000_f32_ba[0], bessel_tdf2_order8_cut1000_f32_ba[1], NULL, NULL, 9);
        benchmark_cascade_and_log(fp, "bessel_cascade_order8_cut1000_f32_sos", "float", CASCADE_f, NULL, *bessel_cascade_order8_cut1000_f32_sos, NULL, 4);
        benchmark_and_log(fp, "bessel_df1_order8_cut2000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, bessel_df1_order8_cut2000_f64_ba[0], bessel_df1_order8_cut2000_f64_ba[1], 9);
        benchmark_and_log(fp, "bessel_df2_order8_cut2000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, bessel_df2_order8_cut2000_f64_ba[0], bessel_df2_order8_cut2000_f64_ba[1], 9);
        benchmark_and_log(fp, "bessel_tdf2_order8_cut2000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, bessel_tdf2_order8_cut2000_f64_ba[0], bessel_tdf2_order8_cut2000_f64_ba[1], 9);
        benchmark_cascade_and_log(fp, "bessel_cascade_order8_cut2000_f64_sos", "double", NULL, CASCADE_d, NULL, *bessel_cascade_order8_cut2000_f64_sos, 4);
        benchmark_and_log(fp, "bessel_df1_order8_cut2000_f32_ba", "float", "DF1", DF1_f, NULL, bessel_df1_order8_cut2000_f32_ba[0], bessel_df1_order8_cut2000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "bessel_df2_order8_cut2000_f32_ba", "float", "DF2", DF2_f, NULL, bessel_df2_order8_cut2000_f32_ba[0], bessel_df2_order8_cut2000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "bessel_tdf2_order8_cut2000_f32_ba", "float", "TDF2", TDF2_f, NULL, bessel_tdf2_order8_cut2000_f32_ba[0], bessel_tdf2_order8_cut2000_f32_ba[1], NULL, NULL, 9);
        benchmark_cascade_and_log(fp, "bessel_cascade_order8_cut2000_f32_sos", "float", CASCADE_f, NULL, *bessel_cascade_order8_cut2000_f32_sos, NULL, 4);
        benchmark_and_log(fp, "bessel_df1_order8_cut5000_f64_ba", "double", "DF1", NULL, DF1_d, NULL, NULL, bessel_df1_order8_cut5000_f64_ba[0], bessel_df1_order8_cut5000_f64_ba[1], 9);
        benchmark_and_log(fp, "bessel_df2_order8_cut5000_f64_ba", "double", "DF2", NULL, DF2_d, NULL, NULL, bessel_df2_order8_cut5000_f64_ba[0], bessel_df2_order8_cut5000_f64_ba[1], 9);
        benchmark_and_log(fp, "bessel_tdf2_order8_cut5000_f64_ba", "double", "TDF2", NULL, TDF2_d, NULL, NULL, bessel_tdf2_order8_cut5000_f64_ba[0], bessel_tdf2_order8_cut5000_f64_ba[1], 9);
        benchmark_cascade_and_log(fp, "bessel_cascade_order8_cut5000_f64_sos", "double", NULL, CASCADE_d, NULL, *bessel_cascade_order8_cut5000_f64_sos, 4);
        benchmark_and_log(fp, "bessel_df1_order8_cut5000_f32_ba", "float", "DF1", DF1_f, NULL, bessel_df1_order8_cut5000_f32_ba[0], bessel_df1_order8_cut5000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "bessel_df2_order8_cut5000_f32_ba", "float", "DF2", DF2_f, NULL, bessel_df2_order8_cut5000_f32_ba[0], bessel_df2_order8_cut5000_f32_ba[1], NULL, NULL, 9);
        benchmark_and_log(fp, "bessel_tdf2_order8_cut5000_f32_ba", "float", "TDF2", TDF2_f, NULL, bessel_tdf2_order8_cut5000_f32_ba[0], bessel_tdf2_order8_cut5000_f32_ba[1], NULL, NULL, 9);
        benchmark_cascade_and_log(fp, "bessel_cascade_order8_cut5000_f32_sos", "float", CASCADE_f, NULL, *bessel_cascade_order8_cut5000_f32_sos, NULL, 4);
    }
    
    

    fclose(fp);
    return 0;
}
