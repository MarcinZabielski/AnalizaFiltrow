#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "./lib/fixedpointQ24.h"
#include "./lib/structuresQ24s.h"
#include "./lib/fixedpointQ12.h"
#include "./lib/structuresQ12s.h"
#include "../_filtercoeffs/filtercoeffs.h"

#define FS 48000
#define DURATION_MIN 10
int N = (FS * 60 * DURATION_MIN);

// Analiza szybkości filtrowania w języku C (reprezentacja stałopozycyjna)

//Kompilacja: gcc -o TimeAnalysis_fixed TimeAnalysis_fixed.c ./lib/fixedpointQ24.c ./lib/fixedpointQ12.c ./lib/structuresQ24s.c ./lib/structuresQ12s.c ../_filtercoeffs/filtercoeffs.c

// === Globalne liczniki (nieuzywane tu) ===
int q24_overflow_count = 0;
int q12_overflow_count = 0;

int q24_underflow_count = 0;
int q12_underflow_count = 0;

// === Szum ===
void generate_white_noise_q24(q24 *x, int N) {
    for (int i = 0; i < N; ++i) {
        double r = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
        x[i] = double_to_q24(r);
    }
}

void generate_white_noise_q12(q12 *x, int N) {
    for (int i = 0; i < N; ++i) {
        double r = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
        x[i] = double_to_q12(r);
    }
}

// === Konwersje współczynników ===
void convert_ba_to_q24(const double *b_d, const double *a_d, q24 *b_q, q24 *a_q, int order) {
    for (int i = 0; i < order; ++i) {
        b_q[i] = double_to_q24((double)b_d[i]);
        a_q[i] = double_to_q24((double)a_d[i]);
    }
}

void convert_sos_to_q24(const double sos_d[][6], q24 sos_q[][6], int sections) {
    for (int i = 0; i < sections; ++i) {
        for (int j = 0; j < 6; ++j) {
            sos_q[i][j] = double_to_q24((double)sos_d[i][j]);
        }
    }
}

void convert_ba_to_q12(const double *b_d, const double *a_d, q12 *b_q, q12 *a_q, int order) {
    for (int i = 0; i < order; ++i) {
        b_q[i] = double_to_q12((double)b_d[i]);
        a_q[i] = double_to_q12((double)a_d[i]);
    }
}

void convert_sos_to_q12(const double sos_d[][6], q12 sos_q[][6], int sections) {
    for (int i = 0; i < sections; ++i) {
        for (int j = 0; j < 6; ++j) {
            sos_q[i][j] = double_to_q12((double)sos_d[i][j]);
        }
    }
}

// === Q8.23 ===
void benchmark_fixed_q24(FILE *fp, const char *filter_name, const char *structure,
                         void (*func)(q24*, q24*, q24*, q24*, int, int),
                         const double *b_d, const double *a_d, int order) {
    clock_t start, end;
    double time_spent;
    int cutoff = -1;

    char filter_type[32];
    sscanf(filter_name, "%[^_]", filter_type);

    char *cut_ptr = strstr(filter_name, "_cut");
    if (cut_ptr) sscanf(cut_ptr, "_cut%d", &cutoff);

    q24 *x = malloc(sizeof(q24) * N);
    q24 *y = malloc(sizeof(q24) * N);
    q24 *b_q = malloc(sizeof(q24) * order);
    q24 *a_q = malloc(sizeof(q24) * order);

    convert_ba_to_q24(b_d, a_d, b_q, a_q, order);
    generate_white_noise_q24(x, N);

    start = clock();
    func(x, y, b_q, a_q, N, order);
    end = clock();
    time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    fprintf(fp, "%s,q24,%s,%d,%d,%.6f\n", filter_type, structure, cutoff, order - 1, time_spent);
    printf("[LOG] %s (q24, %s, cut %d, order %d): %.6f sec\n", filter_type, structure, cutoff, order - 1, time_spent);

    free(x); free(y); free(b_q); free(a_q);
}

void benchmark_cascade_q24(FILE *fp, const char *filter_name,
                            void (*func)(q24*, q24*, q24*, int, int),
                            const double sos_d[][6], int sections) {
    clock_t start, end;
    double time_spent;
    int cutoff = -1;

    char filter_type[32];
    sscanf(filter_name, "%[^_]", filter_type);

    char *cut_ptr = strstr(filter_name, "_cut");
    if (cut_ptr) sscanf(cut_ptr, "_cut%d", &cutoff);

    q24 *x = malloc(sizeof(q24) * N);
    q24 *y = malloc(sizeof(q24) * N);
    q24 (*sos_q)[6] = malloc(sizeof(q24) * 6 * sections);

    convert_sos_to_q24(sos_d, sos_q, sections);
    generate_white_noise_q24(x, N);

    start = clock();
    func(x, y, (q24*)sos_q, N, sections);
    end = clock();
    time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    fprintf(fp, "%s,q24,CASCADE,%d,%d,%.6f\n", filter_type, cutoff, 2 * sections, time_spent);
    printf("[LOG] %s (q24, CASCADE, cut %d, order %d): %.6f sec\n", filter_type, cutoff, 2 * sections, time_spent);

    free(x); free(y); free(sos_q);
}

// === Q5.11 ===
void benchmark_fixed_q12(FILE *fp, const char *filter_name, const char *structure,
                         void (*func)(q12*, q12*, q12*, q12*, int, int),
                         const double *b_d, const double *a_d, int order) {
    clock_t start, end;
    double time_spent;
    int cutoff = -1;

    char filter_type[32];
    sscanf(filter_name, "%[^_]", filter_type);

    char *cut_ptr = strstr(filter_name, "_cut");
    if (cut_ptr) sscanf(cut_ptr, "_cut%d", &cutoff);

    q12 *x = malloc(sizeof(q12) * N);
    q12 *y = malloc(sizeof(q12) * N);
    q12 *b_q = malloc(sizeof(q12) * order);
    q12 *a_q = malloc(sizeof(q12) * order);

    convert_ba_to_q12(b_d, a_d, b_q, a_q, order);
    generate_white_noise_q12(x, N);

    start = clock();
    func(x, y, b_q, a_q, N, order);
    end = clock();
    time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    fprintf(fp, "%s,q12,%s,%d,%d,%.6f\n", filter_type, structure, cutoff, order - 1, time_spent);
    printf("[LOG] %s (q12, %s, cut %d, order %d): %.6f sec\n", filter_type, structure, cutoff, order - 1, time_spent);

    free(x); free(y); free(b_q); free(a_q);
}

void benchmark_cascade_q12(FILE *fp, const char *filter_name,
                            void (*func)(q12*, q12*, q12*, int, int),
                            const double sos_d[][6], int sections) {
    clock_t start, end;
    double time_spent;
    int cutoff = -1;

    char filter_type[32];
    sscanf(filter_name, "%[^_]", filter_type);

    char *cut_ptr = strstr(filter_name, "_cut");
    if (cut_ptr) sscanf(cut_ptr, "_cut%d", &cutoff);

    q12 *x = malloc(sizeof(q12) * N);
    q12 *y = malloc(sizeof(q12) * N);
    q12 (*sos_q)[6] = malloc(sizeof(q12) * 6 * sections);

    convert_sos_to_q12(sos_d, sos_q, sections);
    generate_white_noise_q12(x, N);

    start = clock();
    func(x, y, (q12*)sos_q, N, sections);
    end = clock();
    time_spent = (double)(end - start) / CLOCKS_PER_SEC;

    fprintf(fp, "%s,q12,CASCADE,%d,%d,%.6f\n", filter_type, cutoff, 2 * sections, time_spent);
    printf("[LOG] %s (q12, CASCADE, cut %d, order %d): %.6f sec\n", filter_type, cutoff, 2 * sections, time_spent);

    free(x); free(y); free(sos_q);
}


int main() {
    FILE *fp = fopen("c_fixed_time_results.csv", "w");
    fprintf(fp, "filter_name,type,structure,cutoff,order,time_seconds\n");

    // Tu wkleić zawartość pliku generated_calls_time_fixed.txt
    // === AUTO-GENERATED BENCHMARK CALLS ===
    // === AUTO-GENERATED BENCHMARK CALLS ===
    // --- FIXED BA ---
    benchmark_fixed_q24(fp, "butter_df1_order2_cut1000_f64_ba", "DF1", DF1_q24, butter_df1_order2_cut1000_f64_ba[0], butter_df1_order2_cut1000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "butter_df1_order2_cut1000_f64_ba", "DF1", DF1_q12, butter_df1_order2_cut1000_f64_ba[0], butter_df1_order2_cut1000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "butter_df2_order2_cut1000_f64_ba", "DF2", DF2_q24, butter_df2_order2_cut1000_f64_ba[0], butter_df2_order2_cut1000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "butter_df2_order2_cut1000_f64_ba", "DF2", DF2_q12, butter_df2_order2_cut1000_f64_ba[0], butter_df2_order2_cut1000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "butter_tdf2_order2_cut1000_f64_ba", "TDF2", TDF2_q24, butter_tdf2_order2_cut1000_f64_ba[0], butter_tdf2_order2_cut1000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "butter_tdf2_order2_cut1000_f64_ba", "TDF2", TDF2_q12, butter_tdf2_order2_cut1000_f64_ba[0], butter_tdf2_order2_cut1000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "butter_df1_order2_cut2000_f64_ba", "DF1", DF1_q24, butter_df1_order2_cut2000_f64_ba[0], butter_df1_order2_cut2000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "butter_df1_order2_cut2000_f64_ba", "DF1", DF1_q12, butter_df1_order2_cut2000_f64_ba[0], butter_df1_order2_cut2000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "butter_df2_order2_cut2000_f64_ba", "DF2", DF2_q24, butter_df2_order2_cut2000_f64_ba[0], butter_df2_order2_cut2000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "butter_df2_order2_cut2000_f64_ba", "DF2", DF2_q12, butter_df2_order2_cut2000_f64_ba[0], butter_df2_order2_cut2000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "butter_tdf2_order2_cut2000_f64_ba", "TDF2", TDF2_q24, butter_tdf2_order2_cut2000_f64_ba[0], butter_tdf2_order2_cut2000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "butter_tdf2_order2_cut2000_f64_ba", "TDF2", TDF2_q12, butter_tdf2_order2_cut2000_f64_ba[0], butter_tdf2_order2_cut2000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "butter_df1_order2_cut5000_f64_ba", "DF1", DF1_q24, butter_df1_order2_cut5000_f64_ba[0], butter_df1_order2_cut5000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "butter_df1_order2_cut5000_f64_ba", "DF1", DF1_q12, butter_df1_order2_cut5000_f64_ba[0], butter_df1_order2_cut5000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "butter_df2_order2_cut5000_f64_ba", "DF2", DF2_q24, butter_df2_order2_cut5000_f64_ba[0], butter_df2_order2_cut5000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "butter_df2_order2_cut5000_f64_ba", "DF2", DF2_q12, butter_df2_order2_cut5000_f64_ba[0], butter_df2_order2_cut5000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "butter_tdf2_order2_cut5000_f64_ba", "TDF2", TDF2_q24, butter_tdf2_order2_cut5000_f64_ba[0], butter_tdf2_order2_cut5000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "butter_tdf2_order2_cut5000_f64_ba", "TDF2", TDF2_q12, butter_tdf2_order2_cut5000_f64_ba[0], butter_tdf2_order2_cut5000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "butter_df1_order4_cut1000_f64_ba", "DF1", DF1_q24, butter_df1_order4_cut1000_f64_ba[0], butter_df1_order4_cut1000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "butter_df1_order4_cut1000_f64_ba", "DF1", DF1_q12, butter_df1_order4_cut1000_f64_ba[0], butter_df1_order4_cut1000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "butter_df2_order4_cut1000_f64_ba", "DF2", DF2_q24, butter_df2_order4_cut1000_f64_ba[0], butter_df2_order4_cut1000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "butter_df2_order4_cut1000_f64_ba", "DF2", DF2_q12, butter_df2_order4_cut1000_f64_ba[0], butter_df2_order4_cut1000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "butter_tdf2_order4_cut1000_f64_ba", "TDF2", TDF2_q24, butter_tdf2_order4_cut1000_f64_ba[0], butter_tdf2_order4_cut1000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "butter_tdf2_order4_cut1000_f64_ba", "TDF2", TDF2_q12, butter_tdf2_order4_cut1000_f64_ba[0], butter_tdf2_order4_cut1000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "butter_df1_order4_cut2000_f64_ba", "DF1", DF1_q24, butter_df1_order4_cut2000_f64_ba[0], butter_df1_order4_cut2000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "butter_df1_order4_cut2000_f64_ba", "DF1", DF1_q12, butter_df1_order4_cut2000_f64_ba[0], butter_df1_order4_cut2000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "butter_df2_order4_cut2000_f64_ba", "DF2", DF2_q24, butter_df2_order4_cut2000_f64_ba[0], butter_df2_order4_cut2000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "butter_df2_order4_cut2000_f64_ba", "DF2", DF2_q12, butter_df2_order4_cut2000_f64_ba[0], butter_df2_order4_cut2000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "butter_tdf2_order4_cut2000_f64_ba", "TDF2", TDF2_q24, butter_tdf2_order4_cut2000_f64_ba[0], butter_tdf2_order4_cut2000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "butter_tdf2_order4_cut2000_f64_ba", "TDF2", TDF2_q12, butter_tdf2_order4_cut2000_f64_ba[0], butter_tdf2_order4_cut2000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "butter_df1_order4_cut5000_f64_ba", "DF1", DF1_q24, butter_df1_order4_cut5000_f64_ba[0], butter_df1_order4_cut5000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "butter_df1_order4_cut5000_f64_ba", "DF1", DF1_q12, butter_df1_order4_cut5000_f64_ba[0], butter_df1_order4_cut5000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "butter_df2_order4_cut5000_f64_ba", "DF2", DF2_q24, butter_df2_order4_cut5000_f64_ba[0], butter_df2_order4_cut5000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "butter_df2_order4_cut5000_f64_ba", "DF2", DF2_q12, butter_df2_order4_cut5000_f64_ba[0], butter_df2_order4_cut5000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "butter_tdf2_order4_cut5000_f64_ba", "TDF2", TDF2_q24, butter_tdf2_order4_cut5000_f64_ba[0], butter_tdf2_order4_cut5000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "butter_tdf2_order4_cut5000_f64_ba", "TDF2", TDF2_q12, butter_tdf2_order4_cut5000_f64_ba[0], butter_tdf2_order4_cut5000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "butter_df1_order6_cut1000_f64_ba", "DF1", DF1_q24, butter_df1_order6_cut1000_f64_ba[0], butter_df1_order6_cut1000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "butter_df1_order6_cut1000_f64_ba", "DF1", DF1_q12, butter_df1_order6_cut1000_f64_ba[0], butter_df1_order6_cut1000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "butter_df2_order6_cut1000_f64_ba", "DF2", DF2_q24, butter_df2_order6_cut1000_f64_ba[0], butter_df2_order6_cut1000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "butter_df2_order6_cut1000_f64_ba", "DF2", DF2_q12, butter_df2_order6_cut1000_f64_ba[0], butter_df2_order6_cut1000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "butter_tdf2_order6_cut1000_f64_ba", "TDF2", TDF2_q24, butter_tdf2_order6_cut1000_f64_ba[0], butter_tdf2_order6_cut1000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "butter_tdf2_order6_cut1000_f64_ba", "TDF2", TDF2_q12, butter_tdf2_order6_cut1000_f64_ba[0], butter_tdf2_order6_cut1000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "butter_df1_order6_cut2000_f64_ba", "DF1", DF1_q24, butter_df1_order6_cut2000_f64_ba[0], butter_df1_order6_cut2000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "butter_df1_order6_cut2000_f64_ba", "DF1", DF1_q12, butter_df1_order6_cut2000_f64_ba[0], butter_df1_order6_cut2000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "butter_df2_order6_cut2000_f64_ba", "DF2", DF2_q24, butter_df2_order6_cut2000_f64_ba[0], butter_df2_order6_cut2000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "butter_df2_order6_cut2000_f64_ba", "DF2", DF2_q12, butter_df2_order6_cut2000_f64_ba[0], butter_df2_order6_cut2000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "butter_tdf2_order6_cut2000_f64_ba", "TDF2", TDF2_q24, butter_tdf2_order6_cut2000_f64_ba[0], butter_tdf2_order6_cut2000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "butter_tdf2_order6_cut2000_f64_ba", "TDF2", TDF2_q12, butter_tdf2_order6_cut2000_f64_ba[0], butter_tdf2_order6_cut2000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "butter_df1_order6_cut5000_f64_ba", "DF1", DF1_q24, butter_df1_order6_cut5000_f64_ba[0], butter_df1_order6_cut5000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "butter_df1_order6_cut5000_f64_ba", "DF1", DF1_q12, butter_df1_order6_cut5000_f64_ba[0], butter_df1_order6_cut5000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "butter_df2_order6_cut5000_f64_ba", "DF2", DF2_q24, butter_df2_order6_cut5000_f64_ba[0], butter_df2_order6_cut5000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "butter_df2_order6_cut5000_f64_ba", "DF2", DF2_q12, butter_df2_order6_cut5000_f64_ba[0], butter_df2_order6_cut5000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "butter_tdf2_order6_cut5000_f64_ba", "TDF2", TDF2_q24, butter_tdf2_order6_cut5000_f64_ba[0], butter_tdf2_order6_cut5000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "butter_tdf2_order6_cut5000_f64_ba", "TDF2", TDF2_q12, butter_tdf2_order6_cut5000_f64_ba[0], butter_tdf2_order6_cut5000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "butter_df1_order8_cut1000_f64_ba", "DF1", DF1_q24, butter_df1_order8_cut1000_f64_ba[0], butter_df1_order8_cut1000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "butter_df1_order8_cut1000_f64_ba", "DF1", DF1_q12, butter_df1_order8_cut1000_f64_ba[0], butter_df1_order8_cut1000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "butter_df2_order8_cut1000_f64_ba", "DF2", DF2_q24, butter_df2_order8_cut1000_f64_ba[0], butter_df2_order8_cut1000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "butter_df2_order8_cut1000_f64_ba", "DF2", DF2_q12, butter_df2_order8_cut1000_f64_ba[0], butter_df2_order8_cut1000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "butter_tdf2_order8_cut1000_f64_ba", "TDF2", TDF2_q24, butter_tdf2_order8_cut1000_f64_ba[0], butter_tdf2_order8_cut1000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "butter_tdf2_order8_cut1000_f64_ba", "TDF2", TDF2_q12, butter_tdf2_order8_cut1000_f64_ba[0], butter_tdf2_order8_cut1000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "butter_df1_order8_cut2000_f64_ba", "DF1", DF1_q24, butter_df1_order8_cut2000_f64_ba[0], butter_df1_order8_cut2000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "butter_df1_order8_cut2000_f64_ba", "DF1", DF1_q12, butter_df1_order8_cut2000_f64_ba[0], butter_df1_order8_cut2000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "butter_df2_order8_cut2000_f64_ba", "DF2", DF2_q24, butter_df2_order8_cut2000_f64_ba[0], butter_df2_order8_cut2000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "butter_df2_order8_cut2000_f64_ba", "DF2", DF2_q12, butter_df2_order8_cut2000_f64_ba[0], butter_df2_order8_cut2000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "butter_tdf2_order8_cut2000_f64_ba", "TDF2", TDF2_q24, butter_tdf2_order8_cut2000_f64_ba[0], butter_tdf2_order8_cut2000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "butter_tdf2_order8_cut2000_f64_ba", "TDF2", TDF2_q12, butter_tdf2_order8_cut2000_f64_ba[0], butter_tdf2_order8_cut2000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "butter_df1_order8_cut5000_f64_ba", "DF1", DF1_q24, butter_df1_order8_cut5000_f64_ba[0], butter_df1_order8_cut5000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "butter_df1_order8_cut5000_f64_ba", "DF1", DF1_q12, butter_df1_order8_cut5000_f64_ba[0], butter_df1_order8_cut5000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "butter_df2_order8_cut5000_f64_ba", "DF2", DF2_q24, butter_df2_order8_cut5000_f64_ba[0], butter_df2_order8_cut5000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "butter_df2_order8_cut5000_f64_ba", "DF2", DF2_q12, butter_df2_order8_cut5000_f64_ba[0], butter_df2_order8_cut5000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "butter_tdf2_order8_cut5000_f64_ba", "TDF2", TDF2_q24, butter_tdf2_order8_cut5000_f64_ba[0], butter_tdf2_order8_cut5000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "butter_tdf2_order8_cut5000_f64_ba", "TDF2", TDF2_q12, butter_tdf2_order8_cut5000_f64_ba[0], butter_tdf2_order8_cut5000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "cheby1_df1_order2_cut1000_f64_ba", "DF1", DF1_q24, cheby1_df1_order2_cut1000_f64_ba[0], cheby1_df1_order2_cut1000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "cheby1_df1_order2_cut1000_f64_ba", "DF1", DF1_q12, cheby1_df1_order2_cut1000_f64_ba[0], cheby1_df1_order2_cut1000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "cheby1_df2_order2_cut1000_f64_ba", "DF2", DF2_q24, cheby1_df2_order2_cut1000_f64_ba[0], cheby1_df2_order2_cut1000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "cheby1_df2_order2_cut1000_f64_ba", "DF2", DF2_q12, cheby1_df2_order2_cut1000_f64_ba[0], cheby1_df2_order2_cut1000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "cheby1_tdf2_order2_cut1000_f64_ba", "TDF2", TDF2_q24, cheby1_tdf2_order2_cut1000_f64_ba[0], cheby1_tdf2_order2_cut1000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "cheby1_tdf2_order2_cut1000_f64_ba", "TDF2", TDF2_q12, cheby1_tdf2_order2_cut1000_f64_ba[0], cheby1_tdf2_order2_cut1000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "cheby1_df1_order2_cut2000_f64_ba", "DF1", DF1_q24, cheby1_df1_order2_cut2000_f64_ba[0], cheby1_df1_order2_cut2000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "cheby1_df1_order2_cut2000_f64_ba", "DF1", DF1_q12, cheby1_df1_order2_cut2000_f64_ba[0], cheby1_df1_order2_cut2000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "cheby1_df2_order2_cut2000_f64_ba", "DF2", DF2_q24, cheby1_df2_order2_cut2000_f64_ba[0], cheby1_df2_order2_cut2000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "cheby1_df2_order2_cut2000_f64_ba", "DF2", DF2_q12, cheby1_df2_order2_cut2000_f64_ba[0], cheby1_df2_order2_cut2000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "cheby1_tdf2_order2_cut2000_f64_ba", "TDF2", TDF2_q24, cheby1_tdf2_order2_cut2000_f64_ba[0], cheby1_tdf2_order2_cut2000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "cheby1_tdf2_order2_cut2000_f64_ba", "TDF2", TDF2_q12, cheby1_tdf2_order2_cut2000_f64_ba[0], cheby1_tdf2_order2_cut2000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "cheby1_df1_order2_cut5000_f64_ba", "DF1", DF1_q24, cheby1_df1_order2_cut5000_f64_ba[0], cheby1_df1_order2_cut5000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "cheby1_df1_order2_cut5000_f64_ba", "DF1", DF1_q12, cheby1_df1_order2_cut5000_f64_ba[0], cheby1_df1_order2_cut5000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "cheby1_df2_order2_cut5000_f64_ba", "DF2", DF2_q24, cheby1_df2_order2_cut5000_f64_ba[0], cheby1_df2_order2_cut5000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "cheby1_df2_order2_cut5000_f64_ba", "DF2", DF2_q12, cheby1_df2_order2_cut5000_f64_ba[0], cheby1_df2_order2_cut5000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "cheby1_tdf2_order2_cut5000_f64_ba", "TDF2", TDF2_q24, cheby1_tdf2_order2_cut5000_f64_ba[0], cheby1_tdf2_order2_cut5000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "cheby1_tdf2_order2_cut5000_f64_ba", "TDF2", TDF2_q12, cheby1_tdf2_order2_cut5000_f64_ba[0], cheby1_tdf2_order2_cut5000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "cheby1_df1_order4_cut1000_f64_ba", "DF1", DF1_q24, cheby1_df1_order4_cut1000_f64_ba[0], cheby1_df1_order4_cut1000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "cheby1_df1_order4_cut1000_f64_ba", "DF1", DF1_q12, cheby1_df1_order4_cut1000_f64_ba[0], cheby1_df1_order4_cut1000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "cheby1_df2_order4_cut1000_f64_ba", "DF2", DF2_q24, cheby1_df2_order4_cut1000_f64_ba[0], cheby1_df2_order4_cut1000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "cheby1_df2_order4_cut1000_f64_ba", "DF2", DF2_q12, cheby1_df2_order4_cut1000_f64_ba[0], cheby1_df2_order4_cut1000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "cheby1_tdf2_order4_cut1000_f64_ba", "TDF2", TDF2_q24, cheby1_tdf2_order4_cut1000_f64_ba[0], cheby1_tdf2_order4_cut1000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "cheby1_tdf2_order4_cut1000_f64_ba", "TDF2", TDF2_q12, cheby1_tdf2_order4_cut1000_f64_ba[0], cheby1_tdf2_order4_cut1000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "cheby1_df1_order4_cut2000_f64_ba", "DF1", DF1_q24, cheby1_df1_order4_cut2000_f64_ba[0], cheby1_df1_order4_cut2000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "cheby1_df1_order4_cut2000_f64_ba", "DF1", DF1_q12, cheby1_df1_order4_cut2000_f64_ba[0], cheby1_df1_order4_cut2000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "cheby1_df2_order4_cut2000_f64_ba", "DF2", DF2_q24, cheby1_df2_order4_cut2000_f64_ba[0], cheby1_df2_order4_cut2000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "cheby1_df2_order4_cut2000_f64_ba", "DF2", DF2_q12, cheby1_df2_order4_cut2000_f64_ba[0], cheby1_df2_order4_cut2000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "cheby1_tdf2_order4_cut2000_f64_ba", "TDF2", TDF2_q24, cheby1_tdf2_order4_cut2000_f64_ba[0], cheby1_tdf2_order4_cut2000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "cheby1_tdf2_order4_cut2000_f64_ba", "TDF2", TDF2_q12, cheby1_tdf2_order4_cut2000_f64_ba[0], cheby1_tdf2_order4_cut2000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "cheby1_df1_order4_cut5000_f64_ba", "DF1", DF1_q24, cheby1_df1_order4_cut5000_f64_ba[0], cheby1_df1_order4_cut5000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "cheby1_df1_order4_cut5000_f64_ba", "DF1", DF1_q12, cheby1_df1_order4_cut5000_f64_ba[0], cheby1_df1_order4_cut5000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "cheby1_df2_order4_cut5000_f64_ba", "DF2", DF2_q24, cheby1_df2_order4_cut5000_f64_ba[0], cheby1_df2_order4_cut5000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "cheby1_df2_order4_cut5000_f64_ba", "DF2", DF2_q12, cheby1_df2_order4_cut5000_f64_ba[0], cheby1_df2_order4_cut5000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "cheby1_tdf2_order4_cut5000_f64_ba", "TDF2", TDF2_q24, cheby1_tdf2_order4_cut5000_f64_ba[0], cheby1_tdf2_order4_cut5000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "cheby1_tdf2_order4_cut5000_f64_ba", "TDF2", TDF2_q12, cheby1_tdf2_order4_cut5000_f64_ba[0], cheby1_tdf2_order4_cut5000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "cheby1_df1_order6_cut1000_f64_ba", "DF1", DF1_q24, cheby1_df1_order6_cut1000_f64_ba[0], cheby1_df1_order6_cut1000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "cheby1_df1_order6_cut1000_f64_ba", "DF1", DF1_q12, cheby1_df1_order6_cut1000_f64_ba[0], cheby1_df1_order6_cut1000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "cheby1_df2_order6_cut1000_f64_ba", "DF2", DF2_q24, cheby1_df2_order6_cut1000_f64_ba[0], cheby1_df2_order6_cut1000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "cheby1_df2_order6_cut1000_f64_ba", "DF2", DF2_q12, cheby1_df2_order6_cut1000_f64_ba[0], cheby1_df2_order6_cut1000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "cheby1_tdf2_order6_cut1000_f64_ba", "TDF2", TDF2_q24, cheby1_tdf2_order6_cut1000_f64_ba[0], cheby1_tdf2_order6_cut1000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "cheby1_tdf2_order6_cut1000_f64_ba", "TDF2", TDF2_q12, cheby1_tdf2_order6_cut1000_f64_ba[0], cheby1_tdf2_order6_cut1000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "cheby1_df1_order6_cut2000_f64_ba", "DF1", DF1_q24, cheby1_df1_order6_cut2000_f64_ba[0], cheby1_df1_order6_cut2000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "cheby1_df1_order6_cut2000_f64_ba", "DF1", DF1_q12, cheby1_df1_order6_cut2000_f64_ba[0], cheby1_df1_order6_cut2000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "cheby1_df2_order6_cut2000_f64_ba", "DF2", DF2_q24, cheby1_df2_order6_cut2000_f64_ba[0], cheby1_df2_order6_cut2000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "cheby1_df2_order6_cut2000_f64_ba", "DF2", DF2_q12, cheby1_df2_order6_cut2000_f64_ba[0], cheby1_df2_order6_cut2000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "cheby1_tdf2_order6_cut2000_f64_ba", "TDF2", TDF2_q24, cheby1_tdf2_order6_cut2000_f64_ba[0], cheby1_tdf2_order6_cut2000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "cheby1_tdf2_order6_cut2000_f64_ba", "TDF2", TDF2_q12, cheby1_tdf2_order6_cut2000_f64_ba[0], cheby1_tdf2_order6_cut2000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "cheby1_df1_order6_cut5000_f64_ba", "DF1", DF1_q24, cheby1_df1_order6_cut5000_f64_ba[0], cheby1_df1_order6_cut5000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "cheby1_df1_order6_cut5000_f64_ba", "DF1", DF1_q12, cheby1_df1_order6_cut5000_f64_ba[0], cheby1_df1_order6_cut5000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "cheby1_df2_order6_cut5000_f64_ba", "DF2", DF2_q24, cheby1_df2_order6_cut5000_f64_ba[0], cheby1_df2_order6_cut5000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "cheby1_df2_order6_cut5000_f64_ba", "DF2", DF2_q12, cheby1_df2_order6_cut5000_f64_ba[0], cheby1_df2_order6_cut5000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "cheby1_tdf2_order6_cut5000_f64_ba", "TDF2", TDF2_q24, cheby1_tdf2_order6_cut5000_f64_ba[0], cheby1_tdf2_order6_cut5000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "cheby1_tdf2_order6_cut5000_f64_ba", "TDF2", TDF2_q12, cheby1_tdf2_order6_cut5000_f64_ba[0], cheby1_tdf2_order6_cut5000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "cheby1_df1_order8_cut1000_f64_ba", "DF1", DF1_q24, cheby1_df1_order8_cut1000_f64_ba[0], cheby1_df1_order8_cut1000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "cheby1_df1_order8_cut1000_f64_ba", "DF1", DF1_q12, cheby1_df1_order8_cut1000_f64_ba[0], cheby1_df1_order8_cut1000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "cheby1_df2_order8_cut1000_f64_ba", "DF2", DF2_q24, cheby1_df2_order8_cut1000_f64_ba[0], cheby1_df2_order8_cut1000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "cheby1_df2_order8_cut1000_f64_ba", "DF2", DF2_q12, cheby1_df2_order8_cut1000_f64_ba[0], cheby1_df2_order8_cut1000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "cheby1_tdf2_order8_cut1000_f64_ba", "TDF2", TDF2_q24, cheby1_tdf2_order8_cut1000_f64_ba[0], cheby1_tdf2_order8_cut1000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "cheby1_tdf2_order8_cut1000_f64_ba", "TDF2", TDF2_q12, cheby1_tdf2_order8_cut1000_f64_ba[0], cheby1_tdf2_order8_cut1000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "cheby1_df1_order8_cut2000_f64_ba", "DF1", DF1_q24, cheby1_df1_order8_cut2000_f64_ba[0], cheby1_df1_order8_cut2000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "cheby1_df1_order8_cut2000_f64_ba", "DF1", DF1_q12, cheby1_df1_order8_cut2000_f64_ba[0], cheby1_df1_order8_cut2000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "cheby1_df2_order8_cut2000_f64_ba", "DF2", DF2_q24, cheby1_df2_order8_cut2000_f64_ba[0], cheby1_df2_order8_cut2000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "cheby1_df2_order8_cut2000_f64_ba", "DF2", DF2_q12, cheby1_df2_order8_cut2000_f64_ba[0], cheby1_df2_order8_cut2000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "cheby1_tdf2_order8_cut2000_f64_ba", "TDF2", TDF2_q24, cheby1_tdf2_order8_cut2000_f64_ba[0], cheby1_tdf2_order8_cut2000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "cheby1_tdf2_order8_cut2000_f64_ba", "TDF2", TDF2_q12, cheby1_tdf2_order8_cut2000_f64_ba[0], cheby1_tdf2_order8_cut2000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "cheby1_df1_order8_cut5000_f64_ba", "DF1", DF1_q24, cheby1_df1_order8_cut5000_f64_ba[0], cheby1_df1_order8_cut5000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "cheby1_df1_order8_cut5000_f64_ba", "DF1", DF1_q12, cheby1_df1_order8_cut5000_f64_ba[0], cheby1_df1_order8_cut5000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "cheby1_df2_order8_cut5000_f64_ba", "DF2", DF2_q24, cheby1_df2_order8_cut5000_f64_ba[0], cheby1_df2_order8_cut5000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "cheby1_df2_order8_cut5000_f64_ba", "DF2", DF2_q12, cheby1_df2_order8_cut5000_f64_ba[0], cheby1_df2_order8_cut5000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "cheby1_tdf2_order8_cut5000_f64_ba", "TDF2", TDF2_q24, cheby1_tdf2_order8_cut5000_f64_ba[0], cheby1_tdf2_order8_cut5000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "cheby1_tdf2_order8_cut5000_f64_ba", "TDF2", TDF2_q12, cheby1_tdf2_order8_cut5000_f64_ba[0], cheby1_tdf2_order8_cut5000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "cheby2_df1_order2_cut1000_f64_ba", "DF1", DF1_q24, cheby2_df1_order2_cut1000_f64_ba[0], cheby2_df1_order2_cut1000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "cheby2_df1_order2_cut1000_f64_ba", "DF1", DF1_q12, cheby2_df1_order2_cut1000_f64_ba[0], cheby2_df1_order2_cut1000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "cheby2_df2_order2_cut1000_f64_ba", "DF2", DF2_q24, cheby2_df2_order2_cut1000_f64_ba[0], cheby2_df2_order2_cut1000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "cheby2_df2_order2_cut1000_f64_ba", "DF2", DF2_q12, cheby2_df2_order2_cut1000_f64_ba[0], cheby2_df2_order2_cut1000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "cheby2_tdf2_order2_cut1000_f64_ba", "TDF2", TDF2_q24, cheby2_tdf2_order2_cut1000_f64_ba[0], cheby2_tdf2_order2_cut1000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "cheby2_tdf2_order2_cut1000_f64_ba", "TDF2", TDF2_q12, cheby2_tdf2_order2_cut1000_f64_ba[0], cheby2_tdf2_order2_cut1000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "cheby2_df1_order2_cut2000_f64_ba", "DF1", DF1_q24, cheby2_df1_order2_cut2000_f64_ba[0], cheby2_df1_order2_cut2000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "cheby2_df1_order2_cut2000_f64_ba", "DF1", DF1_q12, cheby2_df1_order2_cut2000_f64_ba[0], cheby2_df1_order2_cut2000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "cheby2_df2_order2_cut2000_f64_ba", "DF2", DF2_q24, cheby2_df2_order2_cut2000_f64_ba[0], cheby2_df2_order2_cut2000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "cheby2_df2_order2_cut2000_f64_ba", "DF2", DF2_q12, cheby2_df2_order2_cut2000_f64_ba[0], cheby2_df2_order2_cut2000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "cheby2_tdf2_order2_cut2000_f64_ba", "TDF2", TDF2_q24, cheby2_tdf2_order2_cut2000_f64_ba[0], cheby2_tdf2_order2_cut2000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "cheby2_tdf2_order2_cut2000_f64_ba", "TDF2", TDF2_q12, cheby2_tdf2_order2_cut2000_f64_ba[0], cheby2_tdf2_order2_cut2000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "cheby2_df1_order2_cut5000_f64_ba", "DF1", DF1_q24, cheby2_df1_order2_cut5000_f64_ba[0], cheby2_df1_order2_cut5000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "cheby2_df1_order2_cut5000_f64_ba", "DF1", DF1_q12, cheby2_df1_order2_cut5000_f64_ba[0], cheby2_df1_order2_cut5000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "cheby2_df2_order2_cut5000_f64_ba", "DF2", DF2_q24, cheby2_df2_order2_cut5000_f64_ba[0], cheby2_df2_order2_cut5000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "cheby2_df2_order2_cut5000_f64_ba", "DF2", DF2_q12, cheby2_df2_order2_cut5000_f64_ba[0], cheby2_df2_order2_cut5000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "cheby2_tdf2_order2_cut5000_f64_ba", "TDF2", TDF2_q24, cheby2_tdf2_order2_cut5000_f64_ba[0], cheby2_tdf2_order2_cut5000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "cheby2_tdf2_order2_cut5000_f64_ba", "TDF2", TDF2_q12, cheby2_tdf2_order2_cut5000_f64_ba[0], cheby2_tdf2_order2_cut5000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "cheby2_df1_order4_cut1000_f64_ba", "DF1", DF1_q24, cheby2_df1_order4_cut1000_f64_ba[0], cheby2_df1_order4_cut1000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "cheby2_df1_order4_cut1000_f64_ba", "DF1", DF1_q12, cheby2_df1_order4_cut1000_f64_ba[0], cheby2_df1_order4_cut1000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "cheby2_df2_order4_cut1000_f64_ba", "DF2", DF2_q24, cheby2_df2_order4_cut1000_f64_ba[0], cheby2_df2_order4_cut1000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "cheby2_df2_order4_cut1000_f64_ba", "DF2", DF2_q12, cheby2_df2_order4_cut1000_f64_ba[0], cheby2_df2_order4_cut1000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "cheby2_tdf2_order4_cut1000_f64_ba", "TDF2", TDF2_q24, cheby2_tdf2_order4_cut1000_f64_ba[0], cheby2_tdf2_order4_cut1000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "cheby2_tdf2_order4_cut1000_f64_ba", "TDF2", TDF2_q12, cheby2_tdf2_order4_cut1000_f64_ba[0], cheby2_tdf2_order4_cut1000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "cheby2_df1_order4_cut2000_f64_ba", "DF1", DF1_q24, cheby2_df1_order4_cut2000_f64_ba[0], cheby2_df1_order4_cut2000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "cheby2_df1_order4_cut2000_f64_ba", "DF1", DF1_q12, cheby2_df1_order4_cut2000_f64_ba[0], cheby2_df1_order4_cut2000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "cheby2_df2_order4_cut2000_f64_ba", "DF2", DF2_q24, cheby2_df2_order4_cut2000_f64_ba[0], cheby2_df2_order4_cut2000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "cheby2_df2_order4_cut2000_f64_ba", "DF2", DF2_q12, cheby2_df2_order4_cut2000_f64_ba[0], cheby2_df2_order4_cut2000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "cheby2_tdf2_order4_cut2000_f64_ba", "TDF2", TDF2_q24, cheby2_tdf2_order4_cut2000_f64_ba[0], cheby2_tdf2_order4_cut2000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "cheby2_tdf2_order4_cut2000_f64_ba", "TDF2", TDF2_q12, cheby2_tdf2_order4_cut2000_f64_ba[0], cheby2_tdf2_order4_cut2000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "cheby2_df1_order4_cut5000_f64_ba", "DF1", DF1_q24, cheby2_df1_order4_cut5000_f64_ba[0], cheby2_df1_order4_cut5000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "cheby2_df1_order4_cut5000_f64_ba", "DF1", DF1_q12, cheby2_df1_order4_cut5000_f64_ba[0], cheby2_df1_order4_cut5000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "cheby2_df2_order4_cut5000_f64_ba", "DF2", DF2_q24, cheby2_df2_order4_cut5000_f64_ba[0], cheby2_df2_order4_cut5000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "cheby2_df2_order4_cut5000_f64_ba", "DF2", DF2_q12, cheby2_df2_order4_cut5000_f64_ba[0], cheby2_df2_order4_cut5000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "cheby2_tdf2_order4_cut5000_f64_ba", "TDF2", TDF2_q24, cheby2_tdf2_order4_cut5000_f64_ba[0], cheby2_tdf2_order4_cut5000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "cheby2_tdf2_order4_cut5000_f64_ba", "TDF2", TDF2_q12, cheby2_tdf2_order4_cut5000_f64_ba[0], cheby2_tdf2_order4_cut5000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "cheby2_df1_order6_cut1000_f64_ba", "DF1", DF1_q24, cheby2_df1_order6_cut1000_f64_ba[0], cheby2_df1_order6_cut1000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "cheby2_df1_order6_cut1000_f64_ba", "DF1", DF1_q12, cheby2_df1_order6_cut1000_f64_ba[0], cheby2_df1_order6_cut1000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "cheby2_df2_order6_cut1000_f64_ba", "DF2", DF2_q24, cheby2_df2_order6_cut1000_f64_ba[0], cheby2_df2_order6_cut1000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "cheby2_df2_order6_cut1000_f64_ba", "DF2", DF2_q12, cheby2_df2_order6_cut1000_f64_ba[0], cheby2_df2_order6_cut1000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "cheby2_tdf2_order6_cut1000_f64_ba", "TDF2", TDF2_q24, cheby2_tdf2_order6_cut1000_f64_ba[0], cheby2_tdf2_order6_cut1000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "cheby2_tdf2_order6_cut1000_f64_ba", "TDF2", TDF2_q12, cheby2_tdf2_order6_cut1000_f64_ba[0], cheby2_tdf2_order6_cut1000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "cheby2_df1_order6_cut2000_f64_ba", "DF1", DF1_q24, cheby2_df1_order6_cut2000_f64_ba[0], cheby2_df1_order6_cut2000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "cheby2_df1_order6_cut2000_f64_ba", "DF1", DF1_q12, cheby2_df1_order6_cut2000_f64_ba[0], cheby2_df1_order6_cut2000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "cheby2_df2_order6_cut2000_f64_ba", "DF2", DF2_q24, cheby2_df2_order6_cut2000_f64_ba[0], cheby2_df2_order6_cut2000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "cheby2_df2_order6_cut2000_f64_ba", "DF2", DF2_q12, cheby2_df2_order6_cut2000_f64_ba[0], cheby2_df2_order6_cut2000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "cheby2_tdf2_order6_cut2000_f64_ba", "TDF2", TDF2_q24, cheby2_tdf2_order6_cut2000_f64_ba[0], cheby2_tdf2_order6_cut2000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "cheby2_tdf2_order6_cut2000_f64_ba", "TDF2", TDF2_q12, cheby2_tdf2_order6_cut2000_f64_ba[0], cheby2_tdf2_order6_cut2000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "cheby2_df1_order6_cut5000_f64_ba", "DF1", DF1_q24, cheby2_df1_order6_cut5000_f64_ba[0], cheby2_df1_order6_cut5000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "cheby2_df1_order6_cut5000_f64_ba", "DF1", DF1_q12, cheby2_df1_order6_cut5000_f64_ba[0], cheby2_df1_order6_cut5000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "cheby2_df2_order6_cut5000_f64_ba", "DF2", DF2_q24, cheby2_df2_order6_cut5000_f64_ba[0], cheby2_df2_order6_cut5000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "cheby2_df2_order6_cut5000_f64_ba", "DF2", DF2_q12, cheby2_df2_order6_cut5000_f64_ba[0], cheby2_df2_order6_cut5000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "cheby2_tdf2_order6_cut5000_f64_ba", "TDF2", TDF2_q24, cheby2_tdf2_order6_cut5000_f64_ba[0], cheby2_tdf2_order6_cut5000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "cheby2_tdf2_order6_cut5000_f64_ba", "TDF2", TDF2_q12, cheby2_tdf2_order6_cut5000_f64_ba[0], cheby2_tdf2_order6_cut5000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "cheby2_df1_order8_cut1000_f64_ba", "DF1", DF1_q24, cheby2_df1_order8_cut1000_f64_ba[0], cheby2_df1_order8_cut1000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "cheby2_df1_order8_cut1000_f64_ba", "DF1", DF1_q12, cheby2_df1_order8_cut1000_f64_ba[0], cheby2_df1_order8_cut1000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "cheby2_df2_order8_cut1000_f64_ba", "DF2", DF2_q24, cheby2_df2_order8_cut1000_f64_ba[0], cheby2_df2_order8_cut1000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "cheby2_df2_order8_cut1000_f64_ba", "DF2", DF2_q12, cheby2_df2_order8_cut1000_f64_ba[0], cheby2_df2_order8_cut1000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "cheby2_tdf2_order8_cut1000_f64_ba", "TDF2", TDF2_q24, cheby2_tdf2_order8_cut1000_f64_ba[0], cheby2_tdf2_order8_cut1000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "cheby2_tdf2_order8_cut1000_f64_ba", "TDF2", TDF2_q12, cheby2_tdf2_order8_cut1000_f64_ba[0], cheby2_tdf2_order8_cut1000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "cheby2_df1_order8_cut2000_f64_ba", "DF1", DF1_q24, cheby2_df1_order8_cut2000_f64_ba[0], cheby2_df1_order8_cut2000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "cheby2_df1_order8_cut2000_f64_ba", "DF1", DF1_q12, cheby2_df1_order8_cut2000_f64_ba[0], cheby2_df1_order8_cut2000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "cheby2_df2_order8_cut2000_f64_ba", "DF2", DF2_q24, cheby2_df2_order8_cut2000_f64_ba[0], cheby2_df2_order8_cut2000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "cheby2_df2_order8_cut2000_f64_ba", "DF2", DF2_q12, cheby2_df2_order8_cut2000_f64_ba[0], cheby2_df2_order8_cut2000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "cheby2_tdf2_order8_cut2000_f64_ba", "TDF2", TDF2_q24, cheby2_tdf2_order8_cut2000_f64_ba[0], cheby2_tdf2_order8_cut2000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "cheby2_tdf2_order8_cut2000_f64_ba", "TDF2", TDF2_q12, cheby2_tdf2_order8_cut2000_f64_ba[0], cheby2_tdf2_order8_cut2000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "cheby2_df1_order8_cut5000_f64_ba", "DF1", DF1_q24, cheby2_df1_order8_cut5000_f64_ba[0], cheby2_df1_order8_cut5000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "cheby2_df1_order8_cut5000_f64_ba", "DF1", DF1_q12, cheby2_df1_order8_cut5000_f64_ba[0], cheby2_df1_order8_cut5000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "cheby2_df2_order8_cut5000_f64_ba", "DF2", DF2_q24, cheby2_df2_order8_cut5000_f64_ba[0], cheby2_df2_order8_cut5000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "cheby2_df2_order8_cut5000_f64_ba", "DF2", DF2_q12, cheby2_df2_order8_cut5000_f64_ba[0], cheby2_df2_order8_cut5000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "cheby2_tdf2_order8_cut5000_f64_ba", "TDF2", TDF2_q24, cheby2_tdf2_order8_cut5000_f64_ba[0], cheby2_tdf2_order8_cut5000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "cheby2_tdf2_order8_cut5000_f64_ba", "TDF2", TDF2_q12, cheby2_tdf2_order8_cut5000_f64_ba[0], cheby2_tdf2_order8_cut5000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "ellip_df1_order2_cut1000_f64_ba", "DF1", DF1_q24, ellip_df1_order2_cut1000_f64_ba[0], ellip_df1_order2_cut1000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "ellip_df1_order2_cut1000_f64_ba", "DF1", DF1_q12, ellip_df1_order2_cut1000_f64_ba[0], ellip_df1_order2_cut1000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "ellip_df2_order2_cut1000_f64_ba", "DF2", DF2_q24, ellip_df2_order2_cut1000_f64_ba[0], ellip_df2_order2_cut1000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "ellip_df2_order2_cut1000_f64_ba", "DF2", DF2_q12, ellip_df2_order2_cut1000_f64_ba[0], ellip_df2_order2_cut1000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "ellip_tdf2_order2_cut1000_f64_ba", "TDF2", TDF2_q24, ellip_tdf2_order2_cut1000_f64_ba[0], ellip_tdf2_order2_cut1000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "ellip_tdf2_order2_cut1000_f64_ba", "TDF2", TDF2_q12, ellip_tdf2_order2_cut1000_f64_ba[0], ellip_tdf2_order2_cut1000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "ellip_df1_order2_cut2000_f64_ba", "DF1", DF1_q24, ellip_df1_order2_cut2000_f64_ba[0], ellip_df1_order2_cut2000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "ellip_df1_order2_cut2000_f64_ba", "DF1", DF1_q12, ellip_df1_order2_cut2000_f64_ba[0], ellip_df1_order2_cut2000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "ellip_df2_order2_cut2000_f64_ba", "DF2", DF2_q24, ellip_df2_order2_cut2000_f64_ba[0], ellip_df2_order2_cut2000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "ellip_df2_order2_cut2000_f64_ba", "DF2", DF2_q12, ellip_df2_order2_cut2000_f64_ba[0], ellip_df2_order2_cut2000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "ellip_tdf2_order2_cut2000_f64_ba", "TDF2", TDF2_q24, ellip_tdf2_order2_cut2000_f64_ba[0], ellip_tdf2_order2_cut2000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "ellip_tdf2_order2_cut2000_f64_ba", "TDF2", TDF2_q12, ellip_tdf2_order2_cut2000_f64_ba[0], ellip_tdf2_order2_cut2000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "ellip_df1_order2_cut5000_f64_ba", "DF1", DF1_q24, ellip_df1_order2_cut5000_f64_ba[0], ellip_df1_order2_cut5000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "ellip_df1_order2_cut5000_f64_ba", "DF1", DF1_q12, ellip_df1_order2_cut5000_f64_ba[0], ellip_df1_order2_cut5000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "ellip_df2_order2_cut5000_f64_ba", "DF2", DF2_q24, ellip_df2_order2_cut5000_f64_ba[0], ellip_df2_order2_cut5000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "ellip_df2_order2_cut5000_f64_ba", "DF2", DF2_q12, ellip_df2_order2_cut5000_f64_ba[0], ellip_df2_order2_cut5000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "ellip_tdf2_order2_cut5000_f64_ba", "TDF2", TDF2_q24, ellip_tdf2_order2_cut5000_f64_ba[0], ellip_tdf2_order2_cut5000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "ellip_tdf2_order2_cut5000_f64_ba", "TDF2", TDF2_q12, ellip_tdf2_order2_cut5000_f64_ba[0], ellip_tdf2_order2_cut5000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "ellip_df1_order4_cut1000_f64_ba", "DF1", DF1_q24, ellip_df1_order4_cut1000_f64_ba[0], ellip_df1_order4_cut1000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "ellip_df1_order4_cut1000_f64_ba", "DF1", DF1_q12, ellip_df1_order4_cut1000_f64_ba[0], ellip_df1_order4_cut1000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "ellip_df2_order4_cut1000_f64_ba", "DF2", DF2_q24, ellip_df2_order4_cut1000_f64_ba[0], ellip_df2_order4_cut1000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "ellip_df2_order4_cut1000_f64_ba", "DF2", DF2_q12, ellip_df2_order4_cut1000_f64_ba[0], ellip_df2_order4_cut1000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "ellip_tdf2_order4_cut1000_f64_ba", "TDF2", TDF2_q24, ellip_tdf2_order4_cut1000_f64_ba[0], ellip_tdf2_order4_cut1000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "ellip_tdf2_order4_cut1000_f64_ba", "TDF2", TDF2_q12, ellip_tdf2_order4_cut1000_f64_ba[0], ellip_tdf2_order4_cut1000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "ellip_df1_order4_cut2000_f64_ba", "DF1", DF1_q24, ellip_df1_order4_cut2000_f64_ba[0], ellip_df1_order4_cut2000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "ellip_df1_order4_cut2000_f64_ba", "DF1", DF1_q12, ellip_df1_order4_cut2000_f64_ba[0], ellip_df1_order4_cut2000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "ellip_df2_order4_cut2000_f64_ba", "DF2", DF2_q24, ellip_df2_order4_cut2000_f64_ba[0], ellip_df2_order4_cut2000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "ellip_df2_order4_cut2000_f64_ba", "DF2", DF2_q12, ellip_df2_order4_cut2000_f64_ba[0], ellip_df2_order4_cut2000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "ellip_tdf2_order4_cut2000_f64_ba", "TDF2", TDF2_q24, ellip_tdf2_order4_cut2000_f64_ba[0], ellip_tdf2_order4_cut2000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "ellip_tdf2_order4_cut2000_f64_ba", "TDF2", TDF2_q12, ellip_tdf2_order4_cut2000_f64_ba[0], ellip_tdf2_order4_cut2000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "ellip_df1_order4_cut5000_f64_ba", "DF1", DF1_q24, ellip_df1_order4_cut5000_f64_ba[0], ellip_df1_order4_cut5000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "ellip_df1_order4_cut5000_f64_ba", "DF1", DF1_q12, ellip_df1_order4_cut5000_f64_ba[0], ellip_df1_order4_cut5000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "ellip_df2_order4_cut5000_f64_ba", "DF2", DF2_q24, ellip_df2_order4_cut5000_f64_ba[0], ellip_df2_order4_cut5000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "ellip_df2_order4_cut5000_f64_ba", "DF2", DF2_q12, ellip_df2_order4_cut5000_f64_ba[0], ellip_df2_order4_cut5000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "ellip_tdf2_order4_cut5000_f64_ba", "TDF2", TDF2_q24, ellip_tdf2_order4_cut5000_f64_ba[0], ellip_tdf2_order4_cut5000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "ellip_tdf2_order4_cut5000_f64_ba", "TDF2", TDF2_q12, ellip_tdf2_order4_cut5000_f64_ba[0], ellip_tdf2_order4_cut5000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "ellip_df1_order6_cut1000_f64_ba", "DF1", DF1_q24, ellip_df1_order6_cut1000_f64_ba[0], ellip_df1_order6_cut1000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "ellip_df1_order6_cut1000_f64_ba", "DF1", DF1_q12, ellip_df1_order6_cut1000_f64_ba[0], ellip_df1_order6_cut1000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "ellip_df2_order6_cut1000_f64_ba", "DF2", DF2_q24, ellip_df2_order6_cut1000_f64_ba[0], ellip_df2_order6_cut1000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "ellip_df2_order6_cut1000_f64_ba", "DF2", DF2_q12, ellip_df2_order6_cut1000_f64_ba[0], ellip_df2_order6_cut1000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "ellip_tdf2_order6_cut1000_f64_ba", "TDF2", TDF2_q24, ellip_tdf2_order6_cut1000_f64_ba[0], ellip_tdf2_order6_cut1000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "ellip_tdf2_order6_cut1000_f64_ba", "TDF2", TDF2_q12, ellip_tdf2_order6_cut1000_f64_ba[0], ellip_tdf2_order6_cut1000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "ellip_df1_order6_cut2000_f64_ba", "DF1", DF1_q24, ellip_df1_order6_cut2000_f64_ba[0], ellip_df1_order6_cut2000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "ellip_df1_order6_cut2000_f64_ba", "DF1", DF1_q12, ellip_df1_order6_cut2000_f64_ba[0], ellip_df1_order6_cut2000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "ellip_df2_order6_cut2000_f64_ba", "DF2", DF2_q24, ellip_df2_order6_cut2000_f64_ba[0], ellip_df2_order6_cut2000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "ellip_df2_order6_cut2000_f64_ba", "DF2", DF2_q12, ellip_df2_order6_cut2000_f64_ba[0], ellip_df2_order6_cut2000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "ellip_tdf2_order6_cut2000_f64_ba", "TDF2", TDF2_q24, ellip_tdf2_order6_cut2000_f64_ba[0], ellip_tdf2_order6_cut2000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "ellip_tdf2_order6_cut2000_f64_ba", "TDF2", TDF2_q12, ellip_tdf2_order6_cut2000_f64_ba[0], ellip_tdf2_order6_cut2000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "ellip_df1_order6_cut5000_f64_ba", "DF1", DF1_q24, ellip_df1_order6_cut5000_f64_ba[0], ellip_df1_order6_cut5000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "ellip_df1_order6_cut5000_f64_ba", "DF1", DF1_q12, ellip_df1_order6_cut5000_f64_ba[0], ellip_df1_order6_cut5000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "ellip_df2_order6_cut5000_f64_ba", "DF2", DF2_q24, ellip_df2_order6_cut5000_f64_ba[0], ellip_df2_order6_cut5000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "ellip_df2_order6_cut5000_f64_ba", "DF2", DF2_q12, ellip_df2_order6_cut5000_f64_ba[0], ellip_df2_order6_cut5000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "ellip_tdf2_order6_cut5000_f64_ba", "TDF2", TDF2_q24, ellip_tdf2_order6_cut5000_f64_ba[0], ellip_tdf2_order6_cut5000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "ellip_tdf2_order6_cut5000_f64_ba", "TDF2", TDF2_q12, ellip_tdf2_order6_cut5000_f64_ba[0], ellip_tdf2_order6_cut5000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "ellip_df1_order8_cut1000_f64_ba", "DF1", DF1_q24, ellip_df1_order8_cut1000_f64_ba[0], ellip_df1_order8_cut1000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "ellip_df1_order8_cut1000_f64_ba", "DF1", DF1_q12, ellip_df1_order8_cut1000_f64_ba[0], ellip_df1_order8_cut1000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "ellip_df2_order8_cut1000_f64_ba", "DF2", DF2_q24, ellip_df2_order8_cut1000_f64_ba[0], ellip_df2_order8_cut1000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "ellip_df2_order8_cut1000_f64_ba", "DF2", DF2_q12, ellip_df2_order8_cut1000_f64_ba[0], ellip_df2_order8_cut1000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "ellip_tdf2_order8_cut1000_f64_ba", "TDF2", TDF2_q24, ellip_tdf2_order8_cut1000_f64_ba[0], ellip_tdf2_order8_cut1000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "ellip_tdf2_order8_cut1000_f64_ba", "TDF2", TDF2_q12, ellip_tdf2_order8_cut1000_f64_ba[0], ellip_tdf2_order8_cut1000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "ellip_df1_order8_cut2000_f64_ba", "DF1", DF1_q24, ellip_df1_order8_cut2000_f64_ba[0], ellip_df1_order8_cut2000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "ellip_df1_order8_cut2000_f64_ba", "DF1", DF1_q12, ellip_df1_order8_cut2000_f64_ba[0], ellip_df1_order8_cut2000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "ellip_df2_order8_cut2000_f64_ba", "DF2", DF2_q24, ellip_df2_order8_cut2000_f64_ba[0], ellip_df2_order8_cut2000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "ellip_df2_order8_cut2000_f64_ba", "DF2", DF2_q12, ellip_df2_order8_cut2000_f64_ba[0], ellip_df2_order8_cut2000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "ellip_tdf2_order8_cut2000_f64_ba", "TDF2", TDF2_q24, ellip_tdf2_order8_cut2000_f64_ba[0], ellip_tdf2_order8_cut2000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "ellip_tdf2_order8_cut2000_f64_ba", "TDF2", TDF2_q12, ellip_tdf2_order8_cut2000_f64_ba[0], ellip_tdf2_order8_cut2000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "ellip_df1_order8_cut5000_f64_ba", "DF1", DF1_q24, ellip_df1_order8_cut5000_f64_ba[0], ellip_df1_order8_cut5000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "ellip_df1_order8_cut5000_f64_ba", "DF1", DF1_q12, ellip_df1_order8_cut5000_f64_ba[0], ellip_df1_order8_cut5000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "ellip_df2_order8_cut5000_f64_ba", "DF2", DF2_q24, ellip_df2_order8_cut5000_f64_ba[0], ellip_df2_order8_cut5000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "ellip_df2_order8_cut5000_f64_ba", "DF2", DF2_q12, ellip_df2_order8_cut5000_f64_ba[0], ellip_df2_order8_cut5000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "ellip_tdf2_order8_cut5000_f64_ba", "TDF2", TDF2_q24, ellip_tdf2_order8_cut5000_f64_ba[0], ellip_tdf2_order8_cut5000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "ellip_tdf2_order8_cut5000_f64_ba", "TDF2", TDF2_q12, ellip_tdf2_order8_cut5000_f64_ba[0], ellip_tdf2_order8_cut5000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "bessel_df1_order2_cut1000_f64_ba", "DF1", DF1_q24, bessel_df1_order2_cut1000_f64_ba[0], bessel_df1_order2_cut1000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "bessel_df1_order2_cut1000_f64_ba", "DF1", DF1_q12, bessel_df1_order2_cut1000_f64_ba[0], bessel_df1_order2_cut1000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "bessel_df2_order2_cut1000_f64_ba", "DF2", DF2_q24, bessel_df2_order2_cut1000_f64_ba[0], bessel_df2_order2_cut1000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "bessel_df2_order2_cut1000_f64_ba", "DF2", DF2_q12, bessel_df2_order2_cut1000_f64_ba[0], bessel_df2_order2_cut1000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "bessel_tdf2_order2_cut1000_f64_ba", "TDF2", TDF2_q24, bessel_tdf2_order2_cut1000_f64_ba[0], bessel_tdf2_order2_cut1000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "bessel_tdf2_order2_cut1000_f64_ba", "TDF2", TDF2_q12, bessel_tdf2_order2_cut1000_f64_ba[0], bessel_tdf2_order2_cut1000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "bessel_df1_order2_cut2000_f64_ba", "DF1", DF1_q24, bessel_df1_order2_cut2000_f64_ba[0], bessel_df1_order2_cut2000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "bessel_df1_order2_cut2000_f64_ba", "DF1", DF1_q12, bessel_df1_order2_cut2000_f64_ba[0], bessel_df1_order2_cut2000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "bessel_df2_order2_cut2000_f64_ba", "DF2", DF2_q24, bessel_df2_order2_cut2000_f64_ba[0], bessel_df2_order2_cut2000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "bessel_df2_order2_cut2000_f64_ba", "DF2", DF2_q12, bessel_df2_order2_cut2000_f64_ba[0], bessel_df2_order2_cut2000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "bessel_tdf2_order2_cut2000_f64_ba", "TDF2", TDF2_q24, bessel_tdf2_order2_cut2000_f64_ba[0], bessel_tdf2_order2_cut2000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "bessel_tdf2_order2_cut2000_f64_ba", "TDF2", TDF2_q12, bessel_tdf2_order2_cut2000_f64_ba[0], bessel_tdf2_order2_cut2000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "bessel_df1_order2_cut5000_f64_ba", "DF1", DF1_q24, bessel_df1_order2_cut5000_f64_ba[0], bessel_df1_order2_cut5000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "bessel_df1_order2_cut5000_f64_ba", "DF1", DF1_q12, bessel_df1_order2_cut5000_f64_ba[0], bessel_df1_order2_cut5000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "bessel_df2_order2_cut5000_f64_ba", "DF2", DF2_q24, bessel_df2_order2_cut5000_f64_ba[0], bessel_df2_order2_cut5000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "bessel_df2_order2_cut5000_f64_ba", "DF2", DF2_q12, bessel_df2_order2_cut5000_f64_ba[0], bessel_df2_order2_cut5000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "bessel_tdf2_order2_cut5000_f64_ba", "TDF2", TDF2_q24, bessel_tdf2_order2_cut5000_f64_ba[0], bessel_tdf2_order2_cut5000_f64_ba[1], 3);
    benchmark_fixed_q12(fp, "bessel_tdf2_order2_cut5000_f64_ba", "TDF2", TDF2_q12, bessel_tdf2_order2_cut5000_f64_ba[0], bessel_tdf2_order2_cut5000_f64_ba[1], 3);

    benchmark_fixed_q24(fp, "bessel_df1_order4_cut1000_f64_ba", "DF1", DF1_q24, bessel_df1_order4_cut1000_f64_ba[0], bessel_df1_order4_cut1000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "bessel_df1_order4_cut1000_f64_ba", "DF1", DF1_q12, bessel_df1_order4_cut1000_f64_ba[0], bessel_df1_order4_cut1000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "bessel_df2_order4_cut1000_f64_ba", "DF2", DF2_q24, bessel_df2_order4_cut1000_f64_ba[0], bessel_df2_order4_cut1000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "bessel_df2_order4_cut1000_f64_ba", "DF2", DF2_q12, bessel_df2_order4_cut1000_f64_ba[0], bessel_df2_order4_cut1000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "bessel_tdf2_order4_cut1000_f64_ba", "TDF2", TDF2_q24, bessel_tdf2_order4_cut1000_f64_ba[0], bessel_tdf2_order4_cut1000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "bessel_tdf2_order4_cut1000_f64_ba", "TDF2", TDF2_q12, bessel_tdf2_order4_cut1000_f64_ba[0], bessel_tdf2_order4_cut1000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "bessel_df1_order4_cut2000_f64_ba", "DF1", DF1_q24, bessel_df1_order4_cut2000_f64_ba[0], bessel_df1_order4_cut2000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "bessel_df1_order4_cut2000_f64_ba", "DF1", DF1_q12, bessel_df1_order4_cut2000_f64_ba[0], bessel_df1_order4_cut2000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "bessel_df2_order4_cut2000_f64_ba", "DF2", DF2_q24, bessel_df2_order4_cut2000_f64_ba[0], bessel_df2_order4_cut2000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "bessel_df2_order4_cut2000_f64_ba", "DF2", DF2_q12, bessel_df2_order4_cut2000_f64_ba[0], bessel_df2_order4_cut2000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "bessel_tdf2_order4_cut2000_f64_ba", "TDF2", TDF2_q24, bessel_tdf2_order4_cut2000_f64_ba[0], bessel_tdf2_order4_cut2000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "bessel_tdf2_order4_cut2000_f64_ba", "TDF2", TDF2_q12, bessel_tdf2_order4_cut2000_f64_ba[0], bessel_tdf2_order4_cut2000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "bessel_df1_order4_cut5000_f64_ba", "DF1", DF1_q24, bessel_df1_order4_cut5000_f64_ba[0], bessel_df1_order4_cut5000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "bessel_df1_order4_cut5000_f64_ba", "DF1", DF1_q12, bessel_df1_order4_cut5000_f64_ba[0], bessel_df1_order4_cut5000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "bessel_df2_order4_cut5000_f64_ba", "DF2", DF2_q24, bessel_df2_order4_cut5000_f64_ba[0], bessel_df2_order4_cut5000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "bessel_df2_order4_cut5000_f64_ba", "DF2", DF2_q12, bessel_df2_order4_cut5000_f64_ba[0], bessel_df2_order4_cut5000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "bessel_tdf2_order4_cut5000_f64_ba", "TDF2", TDF2_q24, bessel_tdf2_order4_cut5000_f64_ba[0], bessel_tdf2_order4_cut5000_f64_ba[1], 5);
    benchmark_fixed_q12(fp, "bessel_tdf2_order4_cut5000_f64_ba", "TDF2", TDF2_q12, bessel_tdf2_order4_cut5000_f64_ba[0], bessel_tdf2_order4_cut5000_f64_ba[1], 5);

    benchmark_fixed_q24(fp, "bessel_df1_order6_cut1000_f64_ba", "DF1", DF1_q24, bessel_df1_order6_cut1000_f64_ba[0], bessel_df1_order6_cut1000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "bessel_df1_order6_cut1000_f64_ba", "DF1", DF1_q12, bessel_df1_order6_cut1000_f64_ba[0], bessel_df1_order6_cut1000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "bessel_df2_order6_cut1000_f64_ba", "DF2", DF2_q24, bessel_df2_order6_cut1000_f64_ba[0], bessel_df2_order6_cut1000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "bessel_df2_order6_cut1000_f64_ba", "DF2", DF2_q12, bessel_df2_order6_cut1000_f64_ba[0], bessel_df2_order6_cut1000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "bessel_tdf2_order6_cut1000_f64_ba", "TDF2", TDF2_q24, bessel_tdf2_order6_cut1000_f64_ba[0], bessel_tdf2_order6_cut1000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "bessel_tdf2_order6_cut1000_f64_ba", "TDF2", TDF2_q12, bessel_tdf2_order6_cut1000_f64_ba[0], bessel_tdf2_order6_cut1000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "bessel_df1_order6_cut2000_f64_ba", "DF1", DF1_q24, bessel_df1_order6_cut2000_f64_ba[0], bessel_df1_order6_cut2000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "bessel_df1_order6_cut2000_f64_ba", "DF1", DF1_q12, bessel_df1_order6_cut2000_f64_ba[0], bessel_df1_order6_cut2000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "bessel_df2_order6_cut2000_f64_ba", "DF2", DF2_q24, bessel_df2_order6_cut2000_f64_ba[0], bessel_df2_order6_cut2000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "bessel_df2_order6_cut2000_f64_ba", "DF2", DF2_q12, bessel_df2_order6_cut2000_f64_ba[0], bessel_df2_order6_cut2000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "bessel_tdf2_order6_cut2000_f64_ba", "TDF2", TDF2_q24, bessel_tdf2_order6_cut2000_f64_ba[0], bessel_tdf2_order6_cut2000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "bessel_tdf2_order6_cut2000_f64_ba", "TDF2", TDF2_q12, bessel_tdf2_order6_cut2000_f64_ba[0], bessel_tdf2_order6_cut2000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "bessel_df1_order6_cut5000_f64_ba", "DF1", DF1_q24, bessel_df1_order6_cut5000_f64_ba[0], bessel_df1_order6_cut5000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "bessel_df1_order6_cut5000_f64_ba", "DF1", DF1_q12, bessel_df1_order6_cut5000_f64_ba[0], bessel_df1_order6_cut5000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "bessel_df2_order6_cut5000_f64_ba", "DF2", DF2_q24, bessel_df2_order6_cut5000_f64_ba[0], bessel_df2_order6_cut5000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "bessel_df2_order6_cut5000_f64_ba", "DF2", DF2_q12, bessel_df2_order6_cut5000_f64_ba[0], bessel_df2_order6_cut5000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "bessel_tdf2_order6_cut5000_f64_ba", "TDF2", TDF2_q24, bessel_tdf2_order6_cut5000_f64_ba[0], bessel_tdf2_order6_cut5000_f64_ba[1], 7);
    benchmark_fixed_q12(fp, "bessel_tdf2_order6_cut5000_f64_ba", "TDF2", TDF2_q12, bessel_tdf2_order6_cut5000_f64_ba[0], bessel_tdf2_order6_cut5000_f64_ba[1], 7);

    benchmark_fixed_q24(fp, "bessel_df1_order8_cut1000_f64_ba", "DF1", DF1_q24, bessel_df1_order8_cut1000_f64_ba[0], bessel_df1_order8_cut1000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "bessel_df1_order8_cut1000_f64_ba", "DF1", DF1_q12, bessel_df1_order8_cut1000_f64_ba[0], bessel_df1_order8_cut1000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "bessel_df2_order8_cut1000_f64_ba", "DF2", DF2_q24, bessel_df2_order8_cut1000_f64_ba[0], bessel_df2_order8_cut1000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "bessel_df2_order8_cut1000_f64_ba", "DF2", DF2_q12, bessel_df2_order8_cut1000_f64_ba[0], bessel_df2_order8_cut1000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "bessel_tdf2_order8_cut1000_f64_ba", "TDF2", TDF2_q24, bessel_tdf2_order8_cut1000_f64_ba[0], bessel_tdf2_order8_cut1000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "bessel_tdf2_order8_cut1000_f64_ba", "TDF2", TDF2_q12, bessel_tdf2_order8_cut1000_f64_ba[0], bessel_tdf2_order8_cut1000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "bessel_df1_order8_cut2000_f64_ba", "DF1", DF1_q24, bessel_df1_order8_cut2000_f64_ba[0], bessel_df1_order8_cut2000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "bessel_df1_order8_cut2000_f64_ba", "DF1", DF1_q12, bessel_df1_order8_cut2000_f64_ba[0], bessel_df1_order8_cut2000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "bessel_df2_order8_cut2000_f64_ba", "DF2", DF2_q24, bessel_df2_order8_cut2000_f64_ba[0], bessel_df2_order8_cut2000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "bessel_df2_order8_cut2000_f64_ba", "DF2", DF2_q12, bessel_df2_order8_cut2000_f64_ba[0], bessel_df2_order8_cut2000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "bessel_tdf2_order8_cut2000_f64_ba", "TDF2", TDF2_q24, bessel_tdf2_order8_cut2000_f64_ba[0], bessel_tdf2_order8_cut2000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "bessel_tdf2_order8_cut2000_f64_ba", "TDF2", TDF2_q12, bessel_tdf2_order8_cut2000_f64_ba[0], bessel_tdf2_order8_cut2000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "bessel_df1_order8_cut5000_f64_ba", "DF1", DF1_q24, bessel_df1_order8_cut5000_f64_ba[0], bessel_df1_order8_cut5000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "bessel_df1_order8_cut5000_f64_ba", "DF1", DF1_q12, bessel_df1_order8_cut5000_f64_ba[0], bessel_df1_order8_cut5000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "bessel_df2_order8_cut5000_f64_ba", "DF2", DF2_q24, bessel_df2_order8_cut5000_f64_ba[0], bessel_df2_order8_cut5000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "bessel_df2_order8_cut5000_f64_ba", "DF2", DF2_q12, bessel_df2_order8_cut5000_f64_ba[0], bessel_df2_order8_cut5000_f64_ba[1], 9);

    benchmark_fixed_q24(fp, "bessel_tdf2_order8_cut5000_f64_ba", "TDF2", TDF2_q24, bessel_tdf2_order8_cut5000_f64_ba[0], bessel_tdf2_order8_cut5000_f64_ba[1], 9);
    benchmark_fixed_q12(fp, "bessel_tdf2_order8_cut5000_f64_ba", "TDF2", TDF2_q12, bessel_tdf2_order8_cut5000_f64_ba[0], bessel_tdf2_order8_cut5000_f64_ba[1], 9);

    // --- CASCADE SOS ---
    benchmark_cascade_q24(fp, "butter_cascade_order2_cut1000_f64_sos", CASCADE_q24, butter_cascade_order2_cut1000_f64_sos, 1);
    benchmark_cascade_q12(fp, "butter_cascade_order2_cut1000_f64_sos", CASCADE_q12, butter_cascade_order2_cut1000_f64_sos, 1);

    benchmark_cascade_q24(fp, "butter_cascade_order2_cut2000_f64_sos", CASCADE_q24, butter_cascade_order2_cut2000_f64_sos, 1);
    benchmark_cascade_q12(fp, "butter_cascade_order2_cut2000_f64_sos", CASCADE_q12, butter_cascade_order2_cut2000_f64_sos, 1);

    benchmark_cascade_q24(fp, "butter_cascade_order2_cut5000_f64_sos", CASCADE_q24, butter_cascade_order2_cut5000_f64_sos, 1);
    benchmark_cascade_q12(fp, "butter_cascade_order2_cut5000_f64_sos", CASCADE_q12, butter_cascade_order2_cut5000_f64_sos, 1);

    benchmark_cascade_q24(fp, "butter_cascade_order4_cut1000_f64_sos", CASCADE_q24, butter_cascade_order4_cut1000_f64_sos, 2);
    benchmark_cascade_q12(fp, "butter_cascade_order4_cut1000_f64_sos", CASCADE_q12, butter_cascade_order4_cut1000_f64_sos, 2);

    benchmark_cascade_q24(fp, "butter_cascade_order4_cut2000_f64_sos", CASCADE_q24, butter_cascade_order4_cut2000_f64_sos, 2);
    benchmark_cascade_q12(fp, "butter_cascade_order4_cut2000_f64_sos", CASCADE_q12, butter_cascade_order4_cut2000_f64_sos, 2);

    benchmark_cascade_q24(fp, "butter_cascade_order4_cut5000_f64_sos", CASCADE_q24, butter_cascade_order4_cut5000_f64_sos, 2);
    benchmark_cascade_q12(fp, "butter_cascade_order4_cut5000_f64_sos", CASCADE_q12, butter_cascade_order4_cut5000_f64_sos, 2);

    benchmark_cascade_q24(fp, "butter_cascade_order6_cut1000_f64_sos", CASCADE_q24, butter_cascade_order6_cut1000_f64_sos, 3);
    benchmark_cascade_q12(fp, "butter_cascade_order6_cut1000_f64_sos", CASCADE_q12, butter_cascade_order6_cut1000_f64_sos, 3);

    benchmark_cascade_q24(fp, "butter_cascade_order6_cut2000_f64_sos", CASCADE_q24, butter_cascade_order6_cut2000_f64_sos, 3);
    benchmark_cascade_q12(fp, "butter_cascade_order6_cut2000_f64_sos", CASCADE_q12, butter_cascade_order6_cut2000_f64_sos, 3);

    benchmark_cascade_q24(fp, "butter_cascade_order6_cut5000_f64_sos", CASCADE_q24, butter_cascade_order6_cut5000_f64_sos, 3);
    benchmark_cascade_q12(fp, "butter_cascade_order6_cut5000_f64_sos", CASCADE_q12, butter_cascade_order6_cut5000_f64_sos, 3);

    benchmark_cascade_q24(fp, "butter_cascade_order8_cut1000_f64_sos", CASCADE_q24, butter_cascade_order8_cut1000_f64_sos, 4);
    benchmark_cascade_q12(fp, "butter_cascade_order8_cut1000_f64_sos", CASCADE_q12, butter_cascade_order8_cut1000_f64_sos, 4);

    benchmark_cascade_q24(fp, "butter_cascade_order8_cut2000_f64_sos", CASCADE_q24, butter_cascade_order8_cut2000_f64_sos, 4);
    benchmark_cascade_q12(fp, "butter_cascade_order8_cut2000_f64_sos", CASCADE_q12, butter_cascade_order8_cut2000_f64_sos, 4);

    benchmark_cascade_q24(fp, "butter_cascade_order8_cut5000_f64_sos", CASCADE_q24, butter_cascade_order8_cut5000_f64_sos, 4);
    benchmark_cascade_q12(fp, "butter_cascade_order8_cut5000_f64_sos", CASCADE_q12, butter_cascade_order8_cut5000_f64_sos, 4);

    benchmark_cascade_q24(fp, "cheby1_cascade_order2_cut1000_f64_sos", CASCADE_q24, cheby1_cascade_order2_cut1000_f64_sos, 1);
    benchmark_cascade_q12(fp, "cheby1_cascade_order2_cut1000_f64_sos", CASCADE_q12, cheby1_cascade_order2_cut1000_f64_sos, 1);

    benchmark_cascade_q24(fp, "cheby1_cascade_order2_cut2000_f64_sos", CASCADE_q24, cheby1_cascade_order2_cut2000_f64_sos, 1);
    benchmark_cascade_q12(fp, "cheby1_cascade_order2_cut2000_f64_sos", CASCADE_q12, cheby1_cascade_order2_cut2000_f64_sos, 1);

    benchmark_cascade_q24(fp, "cheby1_cascade_order2_cut5000_f64_sos", CASCADE_q24, cheby1_cascade_order2_cut5000_f64_sos, 1);
    benchmark_cascade_q12(fp, "cheby1_cascade_order2_cut5000_f64_sos", CASCADE_q12, cheby1_cascade_order2_cut5000_f64_sos, 1);

    benchmark_cascade_q24(fp, "cheby1_cascade_order4_cut1000_f64_sos", CASCADE_q24, cheby1_cascade_order4_cut1000_f64_sos, 2);
    benchmark_cascade_q12(fp, "cheby1_cascade_order4_cut1000_f64_sos", CASCADE_q12, cheby1_cascade_order4_cut1000_f64_sos, 2);

    benchmark_cascade_q24(fp, "cheby1_cascade_order4_cut2000_f64_sos", CASCADE_q24, cheby1_cascade_order4_cut2000_f64_sos, 2);
    benchmark_cascade_q12(fp, "cheby1_cascade_order4_cut2000_f64_sos", CASCADE_q12, cheby1_cascade_order4_cut2000_f64_sos, 2);

    benchmark_cascade_q24(fp, "cheby1_cascade_order4_cut5000_f64_sos", CASCADE_q24, cheby1_cascade_order4_cut5000_f64_sos, 2);
    benchmark_cascade_q12(fp, "cheby1_cascade_order4_cut5000_f64_sos", CASCADE_q12, cheby1_cascade_order4_cut5000_f64_sos, 2);

    benchmark_cascade_q24(fp, "cheby1_cascade_order6_cut1000_f64_sos", CASCADE_q24, cheby1_cascade_order6_cut1000_f64_sos, 3);
    benchmark_cascade_q12(fp, "cheby1_cascade_order6_cut1000_f64_sos", CASCADE_q12, cheby1_cascade_order6_cut1000_f64_sos, 3);

    benchmark_cascade_q24(fp, "cheby1_cascade_order6_cut2000_f64_sos", CASCADE_q24, cheby1_cascade_order6_cut2000_f64_sos, 3);
    benchmark_cascade_q12(fp, "cheby1_cascade_order6_cut2000_f64_sos", CASCADE_q12, cheby1_cascade_order6_cut2000_f64_sos, 3);

    benchmark_cascade_q24(fp, "cheby1_cascade_order6_cut5000_f64_sos", CASCADE_q24, cheby1_cascade_order6_cut5000_f64_sos, 3);
    benchmark_cascade_q12(fp, "cheby1_cascade_order6_cut5000_f64_sos", CASCADE_q12, cheby1_cascade_order6_cut5000_f64_sos, 3);

    benchmark_cascade_q24(fp, "cheby1_cascade_order8_cut1000_f64_sos", CASCADE_q24, cheby1_cascade_order8_cut1000_f64_sos, 4);
    benchmark_cascade_q12(fp, "cheby1_cascade_order8_cut1000_f64_sos", CASCADE_q12, cheby1_cascade_order8_cut1000_f64_sos, 4);

    benchmark_cascade_q24(fp, "cheby1_cascade_order8_cut2000_f64_sos", CASCADE_q24, cheby1_cascade_order8_cut2000_f64_sos, 4);
    benchmark_cascade_q12(fp, "cheby1_cascade_order8_cut2000_f64_sos", CASCADE_q12, cheby1_cascade_order8_cut2000_f64_sos, 4);

    benchmark_cascade_q24(fp, "cheby1_cascade_order8_cut5000_f64_sos", CASCADE_q24, cheby1_cascade_order8_cut5000_f64_sos, 4);
    benchmark_cascade_q12(fp, "cheby1_cascade_order8_cut5000_f64_sos", CASCADE_q12, cheby1_cascade_order8_cut5000_f64_sos, 4);

    benchmark_cascade_q24(fp, "cheby2_cascade_order2_cut1000_f64_sos", CASCADE_q24, cheby2_cascade_order2_cut1000_f64_sos, 1);
    benchmark_cascade_q12(fp, "cheby2_cascade_order2_cut1000_f64_sos", CASCADE_q12, cheby2_cascade_order2_cut1000_f64_sos, 1);

    benchmark_cascade_q24(fp, "cheby2_cascade_order2_cut2000_f64_sos", CASCADE_q24, cheby2_cascade_order2_cut2000_f64_sos, 1);
    benchmark_cascade_q12(fp, "cheby2_cascade_order2_cut2000_f64_sos", CASCADE_q12, cheby2_cascade_order2_cut2000_f64_sos, 1);

    benchmark_cascade_q24(fp, "cheby2_cascade_order2_cut5000_f64_sos", CASCADE_q24, cheby2_cascade_order2_cut5000_f64_sos, 1);
    benchmark_cascade_q12(fp, "cheby2_cascade_order2_cut5000_f64_sos", CASCADE_q12, cheby2_cascade_order2_cut5000_f64_sos, 1);

    benchmark_cascade_q24(fp, "cheby2_cascade_order4_cut1000_f64_sos", CASCADE_q24, cheby2_cascade_order4_cut1000_f64_sos, 2);
    benchmark_cascade_q12(fp, "cheby2_cascade_order4_cut1000_f64_sos", CASCADE_q12, cheby2_cascade_order4_cut1000_f64_sos, 2);

    benchmark_cascade_q24(fp, "cheby2_cascade_order4_cut2000_f64_sos", CASCADE_q24, cheby2_cascade_order4_cut2000_f64_sos, 2);
    benchmark_cascade_q12(fp, "cheby2_cascade_order4_cut2000_f64_sos", CASCADE_q12, cheby2_cascade_order4_cut2000_f64_sos, 2);

    benchmark_cascade_q24(fp, "cheby2_cascade_order4_cut5000_f64_sos", CASCADE_q24, cheby2_cascade_order4_cut5000_f64_sos, 2);
    benchmark_cascade_q12(fp, "cheby2_cascade_order4_cut5000_f64_sos", CASCADE_q12, cheby2_cascade_order4_cut5000_f64_sos, 2);

    benchmark_cascade_q24(fp, "cheby2_cascade_order6_cut1000_f64_sos", CASCADE_q24, cheby2_cascade_order6_cut1000_f64_sos, 3);
    benchmark_cascade_q12(fp, "cheby2_cascade_order6_cut1000_f64_sos", CASCADE_q12, cheby2_cascade_order6_cut1000_f64_sos, 3);

    benchmark_cascade_q24(fp, "cheby2_cascade_order6_cut2000_f64_sos", CASCADE_q24, cheby2_cascade_order6_cut2000_f64_sos, 3);
    benchmark_cascade_q12(fp, "cheby2_cascade_order6_cut2000_f64_sos", CASCADE_q12, cheby2_cascade_order6_cut2000_f64_sos, 3);

    benchmark_cascade_q24(fp, "cheby2_cascade_order6_cut5000_f64_sos", CASCADE_q24, cheby2_cascade_order6_cut5000_f64_sos, 3);
    benchmark_cascade_q12(fp, "cheby2_cascade_order6_cut5000_f64_sos", CASCADE_q12, cheby2_cascade_order6_cut5000_f64_sos, 3);

    benchmark_cascade_q24(fp, "cheby2_cascade_order8_cut1000_f64_sos", CASCADE_q24, cheby2_cascade_order8_cut1000_f64_sos, 4);
    benchmark_cascade_q12(fp, "cheby2_cascade_order8_cut1000_f64_sos", CASCADE_q12, cheby2_cascade_order8_cut1000_f64_sos, 4);

    benchmark_cascade_q24(fp, "cheby2_cascade_order8_cut2000_f64_sos", CASCADE_q24, cheby2_cascade_order8_cut2000_f64_sos, 4);
    benchmark_cascade_q12(fp, "cheby2_cascade_order8_cut2000_f64_sos", CASCADE_q12, cheby2_cascade_order8_cut2000_f64_sos, 4);

    benchmark_cascade_q24(fp, "cheby2_cascade_order8_cut5000_f64_sos", CASCADE_q24, cheby2_cascade_order8_cut5000_f64_sos, 4);
    benchmark_cascade_q12(fp, "cheby2_cascade_order8_cut5000_f64_sos", CASCADE_q12, cheby2_cascade_order8_cut5000_f64_sos, 4);

    benchmark_cascade_q24(fp, "ellip_cascade_order2_cut1000_f64_sos", CASCADE_q24, ellip_cascade_order2_cut1000_f64_sos, 1);
    benchmark_cascade_q12(fp, "ellip_cascade_order2_cut1000_f64_sos", CASCADE_q12, ellip_cascade_order2_cut1000_f64_sos, 1);

    benchmark_cascade_q24(fp, "ellip_cascade_order2_cut2000_f64_sos", CASCADE_q24, ellip_cascade_order2_cut2000_f64_sos, 1);
    benchmark_cascade_q12(fp, "ellip_cascade_order2_cut2000_f64_sos", CASCADE_q12, ellip_cascade_order2_cut2000_f64_sos, 1);

    benchmark_cascade_q24(fp, "ellip_cascade_order2_cut5000_f64_sos", CASCADE_q24, ellip_cascade_order2_cut5000_f64_sos, 1);
    benchmark_cascade_q12(fp, "ellip_cascade_order2_cut5000_f64_sos", CASCADE_q12, ellip_cascade_order2_cut5000_f64_sos, 1);

    benchmark_cascade_q24(fp, "ellip_cascade_order4_cut1000_f64_sos", CASCADE_q24, ellip_cascade_order4_cut1000_f64_sos, 2);
    benchmark_cascade_q12(fp, "ellip_cascade_order4_cut1000_f64_sos", CASCADE_q12, ellip_cascade_order4_cut1000_f64_sos, 2);

    benchmark_cascade_q24(fp, "ellip_cascade_order4_cut2000_f64_sos", CASCADE_q24, ellip_cascade_order4_cut2000_f64_sos, 2);
    benchmark_cascade_q12(fp, "ellip_cascade_order4_cut2000_f64_sos", CASCADE_q12, ellip_cascade_order4_cut2000_f64_sos, 2);

    benchmark_cascade_q24(fp, "ellip_cascade_order4_cut5000_f64_sos", CASCADE_q24, ellip_cascade_order4_cut5000_f64_sos, 2);
    benchmark_cascade_q12(fp, "ellip_cascade_order4_cut5000_f64_sos", CASCADE_q12, ellip_cascade_order4_cut5000_f64_sos, 2);

    benchmark_cascade_q24(fp, "ellip_cascade_order6_cut1000_f64_sos", CASCADE_q24, ellip_cascade_order6_cut1000_f64_sos, 3);
    benchmark_cascade_q12(fp, "ellip_cascade_order6_cut1000_f64_sos", CASCADE_q12, ellip_cascade_order6_cut1000_f64_sos, 3);

    benchmark_cascade_q24(fp, "ellip_cascade_order6_cut2000_f64_sos", CASCADE_q24, ellip_cascade_order6_cut2000_f64_sos, 3);
    benchmark_cascade_q12(fp, "ellip_cascade_order6_cut2000_f64_sos", CASCADE_q12, ellip_cascade_order6_cut2000_f64_sos, 3);

    benchmark_cascade_q24(fp, "ellip_cascade_order6_cut5000_f64_sos", CASCADE_q24, ellip_cascade_order6_cut5000_f64_sos, 3);
    benchmark_cascade_q12(fp, "ellip_cascade_order6_cut5000_f64_sos", CASCADE_q12, ellip_cascade_order6_cut5000_f64_sos, 3);

    benchmark_cascade_q24(fp, "ellip_cascade_order8_cut1000_f64_sos", CASCADE_q24, ellip_cascade_order8_cut1000_f64_sos, 4);
    benchmark_cascade_q12(fp, "ellip_cascade_order8_cut1000_f64_sos", CASCADE_q12, ellip_cascade_order8_cut1000_f64_sos, 4);

    benchmark_cascade_q24(fp, "ellip_cascade_order8_cut2000_f64_sos", CASCADE_q24, ellip_cascade_order8_cut2000_f64_sos, 4);
    benchmark_cascade_q12(fp, "ellip_cascade_order8_cut2000_f64_sos", CASCADE_q12, ellip_cascade_order8_cut2000_f64_sos, 4);

    benchmark_cascade_q24(fp, "ellip_cascade_order8_cut5000_f64_sos", CASCADE_q24, ellip_cascade_order8_cut5000_f64_sos, 4);
    benchmark_cascade_q12(fp, "ellip_cascade_order8_cut5000_f64_sos", CASCADE_q12, ellip_cascade_order8_cut5000_f64_sos, 4);

    benchmark_cascade_q24(fp, "bessel_cascade_order2_cut1000_f64_sos", CASCADE_q24, bessel_cascade_order2_cut1000_f64_sos, 1);
    benchmark_cascade_q12(fp, "bessel_cascade_order2_cut1000_f64_sos", CASCADE_q12, bessel_cascade_order2_cut1000_f64_sos, 1);

    benchmark_cascade_q24(fp, "bessel_cascade_order2_cut2000_f64_sos", CASCADE_q24, bessel_cascade_order2_cut2000_f64_sos, 1);
    benchmark_cascade_q12(fp, "bessel_cascade_order2_cut2000_f64_sos", CASCADE_q12, bessel_cascade_order2_cut2000_f64_sos, 1);

    benchmark_cascade_q24(fp, "bessel_cascade_order2_cut5000_f64_sos", CASCADE_q24, bessel_cascade_order2_cut5000_f64_sos, 1);
    benchmark_cascade_q12(fp, "bessel_cascade_order2_cut5000_f64_sos", CASCADE_q12, bessel_cascade_order2_cut5000_f64_sos, 1);

    benchmark_cascade_q24(fp, "bessel_cascade_order4_cut1000_f64_sos", CASCADE_q24, bessel_cascade_order4_cut1000_f64_sos, 2);
    benchmark_cascade_q12(fp, "bessel_cascade_order4_cut1000_f64_sos", CASCADE_q12, bessel_cascade_order4_cut1000_f64_sos, 2);

    benchmark_cascade_q24(fp, "bessel_cascade_order4_cut2000_f64_sos", CASCADE_q24, bessel_cascade_order4_cut2000_f64_sos, 2);
    benchmark_cascade_q12(fp, "bessel_cascade_order4_cut2000_f64_sos", CASCADE_q12, bessel_cascade_order4_cut2000_f64_sos, 2);

    benchmark_cascade_q24(fp, "bessel_cascade_order4_cut5000_f64_sos", CASCADE_q24, bessel_cascade_order4_cut5000_f64_sos, 2);
    benchmark_cascade_q12(fp, "bessel_cascade_order4_cut5000_f64_sos", CASCADE_q12, bessel_cascade_order4_cut5000_f64_sos, 2);

    benchmark_cascade_q24(fp, "bessel_cascade_order6_cut1000_f64_sos", CASCADE_q24, bessel_cascade_order6_cut1000_f64_sos, 3);
    benchmark_cascade_q12(fp, "bessel_cascade_order6_cut1000_f64_sos", CASCADE_q12, bessel_cascade_order6_cut1000_f64_sos, 3);

    benchmark_cascade_q24(fp, "bessel_cascade_order6_cut2000_f64_sos", CASCADE_q24, bessel_cascade_order6_cut2000_f64_sos, 3);
    benchmark_cascade_q12(fp, "bessel_cascade_order6_cut2000_f64_sos", CASCADE_q12, bessel_cascade_order6_cut2000_f64_sos, 3);

    benchmark_cascade_q24(fp, "bessel_cascade_order6_cut5000_f64_sos", CASCADE_q24, bessel_cascade_order6_cut5000_f64_sos, 3);
    benchmark_cascade_q12(fp, "bessel_cascade_order6_cut5000_f64_sos", CASCADE_q12, bessel_cascade_order6_cut5000_f64_sos, 3);

    benchmark_cascade_q24(fp, "bessel_cascade_order8_cut1000_f64_sos", CASCADE_q24, bessel_cascade_order8_cut1000_f64_sos, 4);
    benchmark_cascade_q12(fp, "bessel_cascade_order8_cut1000_f64_sos", CASCADE_q12, bessel_cascade_order8_cut1000_f64_sos, 4);

    benchmark_cascade_q24(fp, "bessel_cascade_order8_cut2000_f64_sos", CASCADE_q24, bessel_cascade_order8_cut2000_f64_sos, 4);
    benchmark_cascade_q12(fp, "bessel_cascade_order8_cut2000_f64_sos", CASCADE_q12, bessel_cascade_order8_cut2000_f64_sos, 4);

    benchmark_cascade_q24(fp, "bessel_cascade_order8_cut5000_f64_sos", CASCADE_q24, bessel_cascade_order8_cut5000_f64_sos, 4);
    benchmark_cascade_q12(fp, "bessel_cascade_order8_cut5000_f64_sos", CASCADE_q12, bessel_cascade_order8_cut5000_f64_sos, 4);

    fclose(fp);
    return 0;
}
