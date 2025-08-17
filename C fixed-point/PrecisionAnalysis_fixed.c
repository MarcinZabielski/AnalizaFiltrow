#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdint.h>
#include "../C/lib/structures.h"
#include "./lib/structuresQ24s.h"
#include "./lib/structuresQ12s.h"
#include "./lib/fixedpointQ24.h"
#include "./lib/fixedpointQ12.h"
#include "../_filtercoeffs/filtercoeffs.h"

#define N 4096

// Analiza precyzji filtrowania w języku C (reprezentacja stałopozycyjna)

//Kompilacja: gcc -o PrecisionAnalysis_fixed PrecisionAnalysis_fixed.c ./lib/fixedpointQ24.c ./lib/fixedpointQ12.c ./lib/structuresQ12s.c ./lib/structuresQ24s.c ../_filtercoeffs/filtercoeffs.c ../C/lib/structures.c -lm

// === Globalne liczniki (nieuzywane tu) ===
int q24_overflow_count = 0;
int q24_underflow_count = 0;

int q12_overflow_count = 0;
int q12_underflow_count = 0;

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

// === MAE ===
double compute_mae_q(const double *ref, const void *test, int len, int q) {
    double sum = 0.0;
    for (int i = 0; i < len; i++) {
        double fx = (q == 24) ? q24_to_double(((q24 *)test)[i]) : q12_to_double(((q12 *)test)[i]);
        sum += fabs(ref[i] - fx);
    }
    return sum / len;
}

// === Analiza precyzji ===
void precision_analysis_q(FILE *fp, const char *filter_name, const char *structure, int qtype,
                          void (*func_q)(void*, void*, void*, void*, int, int),
                          void (*func_d)(double*, double*, double*, double*, int, int),
                          double *b_d, double *a_d, int order) {
    int cutoff = -1;
    char filter_type[32];
    char qstr[8];
    snprintf(qstr, sizeof(qstr), "q%d", qtype);

    sscanf(filter_name, "%[^_]", filter_type);

    char *cut_ptr = strstr(filter_name, "_cut");
    if (cut_ptr != NULL)
        sscanf(cut_ptr, "_cut%d", &cutoff);

    void *x_q, *y_q;
    double *x_d = calloc(N, sizeof(double));
    double *y_d = calloc(N, sizeof(double));

    void *b_q, *a_q;
    if (qtype == 24) {
        x_q = calloc(N, sizeof(q24));
        y_q = calloc(N, sizeof(q24));
        b_q = calloc(order, sizeof(q24));
        a_q = calloc(order, sizeof(q24));
        convert_ba_to_q24(b_d, a_d, (q24*)b_q, (q24*)a_q, order);
        ((q24*)x_q)[0] = double_to_q24(1.0);
    } else {
        x_q = calloc(N, sizeof(q12));
        y_q = calloc(N, sizeof(q12));
        b_q = calloc(order, sizeof(q12));
        a_q = calloc(order, sizeof(q12));
        convert_ba_to_q12(b_d, a_d, (q12*)b_q, (q12*)a_q, order);
        ((q12*)x_q)[0] = double_to_q12(1.0);
    }

    x_d[0] = 1.0;

    func_q(x_q, y_q, b_q, a_q, N, order);
    func_d(x_d, y_d, b_d, a_d, N, order);

    double mae_impulse = compute_mae_q(y_d, y_q, N, qtype);
    fprintf(fp, "%s,%s,%s,%d,%d,impulse,%.8e\n", filter_type, qstr, structure, cutoff, order - 1, mae_impulse);

    // Rand test
    for (int i = 0; i < N; i++) {
        double r = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
        x_d[i] = r;
        if (qtype == 24) {
            ((q24*)x_q)[i] = double_to_q24(r);
        } else {
            ((q12*)x_q)[i] = double_to_q12(r);
        }
    }

    memset(y_q, 0, N * (qtype == 24 ? sizeof(q24) : sizeof(q12)));
    memset(y_d, 0, sizeof(double) * N);

    func_q(x_q, y_q, b_q, a_q, N, order);
    func_d(x_d, y_d, b_d, a_d, N, order);

    double mae_rand = compute_mae_q(y_d, y_q, 128, qtype);
    fprintf(fp, "%s,%s,%s,%d,%d,rand,%.8e\n", filter_type, qstr, structure, cutoff, order - 1, mae_rand);

    free(x_q); free(y_q); free(b_q); free(a_q); free(x_d); free(y_d);
}

void precision_analysis_sos(FILE *fp, const char *filter_name, const char *structure, int qtype,
                            void (*func_q)(void*, void*, void*, int, int),
                            void (*func_d)(double*, double*, const double[][6], int, int),
                            const double sos_d[][6], int sections) {
    int cutoff = -1;
    char filter_type[32];
    char qstr[8];
    snprintf(qstr, sizeof(qstr), "q%d", qtype);

    sscanf(filter_name, "%[^_]", filter_type);

    char *cut_ptr = strstr(filter_name, "_cut");
    if (cut_ptr != NULL)
        sscanf(cut_ptr, "_cut%d", &cutoff);

    void *x_q, *y_q, *sos_q;
    double *x_d = calloc(N, sizeof(double));
    double *y_d = calloc(N, sizeof(double));

    if (qtype == 24) {
        x_q = calloc(N, sizeof(q24));
        y_q = calloc(N, sizeof(q24));
        sos_q = calloc(sections, sizeof(q24[6]));
        convert_sos_to_q24(sos_d, (q24 (*)[6])sos_q, sections);
        ((q24*)x_q)[0] = double_to_q24(1.0);
    } else {
        x_q = calloc(N, sizeof(q12));
        y_q = calloc(N, sizeof(q12));
        sos_q = calloc(sections, sizeof(q12[6]));
        convert_sos_to_q12(sos_d, (q12 (*)[6])sos_q, sections);
        ((q12*)x_q)[0] = double_to_q12(1.0);
    }

    x_d[0] = 1.0;

    func_q(x_q, y_q, sos_q, N, sections);
    func_d(x_d, y_d, sos_d, N, sections);

    double mae_impulse = compute_mae_q(y_d, y_q, N, qtype);
    fprintf(fp, "%s,%s,%s,%d,%d,impulse,%.8e\n", filter_type, qstr, structure, cutoff, 2 * sections, mae_impulse);

    // Rand test
    for (int i = 0; i < N; i++) {
        double r = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
        x_d[i] = r;
        if (qtype == 24) {
            ((q24*)x_q)[i] = double_to_q24(r);
        } else {
            ((q12*)x_q)[i] = double_to_q12(r);
        }
    }

    memset(y_q, 0, N * (qtype == 24 ? sizeof(q24) : sizeof(q12)));
    memset(y_d, 0, sizeof(double) * N);

    func_q(x_q, y_q, sos_q, N, sections);
    func_d(x_d, y_d, sos_d, N, sections);

    double mae_rand = compute_mae_q(y_d, y_q, 128, qtype);
    fprintf(fp, "%s,%s,%s,%d,%d,rand,%.8e\n", filter_type, qstr, structure, cutoff, 2 * sections, mae_rand);

    free(x_q); free(y_q); free(sos_q); free(x_d); free(y_d);
}


int main() {
    FILE *fp_precision = fopen("c_fixed_precision_results.csv", "w");
    if (!fp_precision) {
        perror("Can't open CSV file");
        return 1;
    }
    fprintf(fp_precision, "filter_name,type,structure,cutoff,order,signal,MAE\n");

    // Tu wkleić zawartość pliku generated_calls_precision_fixed.txt
    // === AUTO-GENERATED PRECISION ANALYSIS CALLS ===
    // --- DF1 / DF2 / TDF2 ---
    precision_analysis_q(fp_precision, "butter_df1_order2_cut1000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, butter_df1_order2_cut1000_f64_ba[0], butter_df1_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "butter_df1_order2_cut1000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, butter_df1_order2_cut1000_f64_ba[0], butter_df1_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "butter_df2_order2_cut1000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, butter_df2_order2_cut1000_f64_ba[0], butter_df2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "butter_df2_order2_cut1000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, butter_df2_order2_cut1000_f64_ba[0], butter_df2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "butter_tdf2_order2_cut1000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, butter_tdf2_order2_cut1000_f64_ba[0], butter_tdf2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "butter_tdf2_order2_cut1000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, butter_tdf2_order2_cut1000_f64_ba[0], butter_tdf2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "butter_df1_order2_cut2000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, butter_df1_order2_cut2000_f64_ba[0], butter_df1_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "butter_df1_order2_cut2000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, butter_df1_order2_cut2000_f64_ba[0], butter_df1_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "butter_df2_order2_cut2000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, butter_df2_order2_cut2000_f64_ba[0], butter_df2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "butter_df2_order2_cut2000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, butter_df2_order2_cut2000_f64_ba[0], butter_df2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "butter_tdf2_order2_cut2000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, butter_tdf2_order2_cut2000_f64_ba[0], butter_tdf2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "butter_tdf2_order2_cut2000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, butter_tdf2_order2_cut2000_f64_ba[0], butter_tdf2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "butter_df1_order2_cut5000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, butter_df1_order2_cut5000_f64_ba[0], butter_df1_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "butter_df1_order2_cut5000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, butter_df1_order2_cut5000_f64_ba[0], butter_df1_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "butter_df2_order2_cut5000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, butter_df2_order2_cut5000_f64_ba[0], butter_df2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "butter_df2_order2_cut5000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, butter_df2_order2_cut5000_f64_ba[0], butter_df2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "butter_tdf2_order2_cut5000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, butter_tdf2_order2_cut5000_f64_ba[0], butter_tdf2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "butter_tdf2_order2_cut5000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, butter_tdf2_order2_cut5000_f64_ba[0], butter_tdf2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "butter_df1_order4_cut1000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, butter_df1_order4_cut1000_f64_ba[0], butter_df1_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "butter_df1_order4_cut1000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, butter_df1_order4_cut1000_f64_ba[0], butter_df1_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "butter_df2_order4_cut1000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, butter_df2_order4_cut1000_f64_ba[0], butter_df2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "butter_df2_order4_cut1000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, butter_df2_order4_cut1000_f64_ba[0], butter_df2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "butter_tdf2_order4_cut1000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, butter_tdf2_order4_cut1000_f64_ba[0], butter_tdf2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "butter_tdf2_order4_cut1000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, butter_tdf2_order4_cut1000_f64_ba[0], butter_tdf2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "butter_df1_order4_cut2000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, butter_df1_order4_cut2000_f64_ba[0], butter_df1_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "butter_df1_order4_cut2000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, butter_df1_order4_cut2000_f64_ba[0], butter_df1_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "butter_df2_order4_cut2000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, butter_df2_order4_cut2000_f64_ba[0], butter_df2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "butter_df2_order4_cut2000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, butter_df2_order4_cut2000_f64_ba[0], butter_df2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "butter_tdf2_order4_cut2000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, butter_tdf2_order4_cut2000_f64_ba[0], butter_tdf2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "butter_tdf2_order4_cut2000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, butter_tdf2_order4_cut2000_f64_ba[0], butter_tdf2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "butter_df1_order4_cut5000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, butter_df1_order4_cut5000_f64_ba[0], butter_df1_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "butter_df1_order4_cut5000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, butter_df1_order4_cut5000_f64_ba[0], butter_df1_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "butter_df2_order4_cut5000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, butter_df2_order4_cut5000_f64_ba[0], butter_df2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "butter_df2_order4_cut5000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, butter_df2_order4_cut5000_f64_ba[0], butter_df2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "butter_tdf2_order4_cut5000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, butter_tdf2_order4_cut5000_f64_ba[0], butter_tdf2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "butter_tdf2_order4_cut5000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, butter_tdf2_order4_cut5000_f64_ba[0], butter_tdf2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "butter_df1_order6_cut1000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, butter_df1_order6_cut1000_f64_ba[0], butter_df1_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "butter_df1_order6_cut1000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, butter_df1_order6_cut1000_f64_ba[0], butter_df1_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "butter_df2_order6_cut1000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, butter_df2_order6_cut1000_f64_ba[0], butter_df2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "butter_df2_order6_cut1000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, butter_df2_order6_cut1000_f64_ba[0], butter_df2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "butter_tdf2_order6_cut1000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, butter_tdf2_order6_cut1000_f64_ba[0], butter_tdf2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "butter_tdf2_order6_cut1000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, butter_tdf2_order6_cut1000_f64_ba[0], butter_tdf2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "butter_df1_order6_cut2000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, butter_df1_order6_cut2000_f64_ba[0], butter_df1_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "butter_df1_order6_cut2000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, butter_df1_order6_cut2000_f64_ba[0], butter_df1_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "butter_df2_order6_cut2000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, butter_df2_order6_cut2000_f64_ba[0], butter_df2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "butter_df2_order6_cut2000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, butter_df2_order6_cut2000_f64_ba[0], butter_df2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "butter_tdf2_order6_cut2000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, butter_tdf2_order6_cut2000_f64_ba[0], butter_tdf2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "butter_tdf2_order6_cut2000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, butter_tdf2_order6_cut2000_f64_ba[0], butter_tdf2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "butter_df1_order6_cut5000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, butter_df1_order6_cut5000_f64_ba[0], butter_df1_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "butter_df1_order6_cut5000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, butter_df1_order6_cut5000_f64_ba[0], butter_df1_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "butter_df2_order6_cut5000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, butter_df2_order6_cut5000_f64_ba[0], butter_df2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "butter_df2_order6_cut5000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, butter_df2_order6_cut5000_f64_ba[0], butter_df2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "butter_tdf2_order6_cut5000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, butter_tdf2_order6_cut5000_f64_ba[0], butter_tdf2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "butter_tdf2_order6_cut5000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, butter_tdf2_order6_cut5000_f64_ba[0], butter_tdf2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "butter_df1_order8_cut1000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, butter_df1_order8_cut1000_f64_ba[0], butter_df1_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "butter_df1_order8_cut1000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, butter_df1_order8_cut1000_f64_ba[0], butter_df1_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "butter_df2_order8_cut1000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, butter_df2_order8_cut1000_f64_ba[0], butter_df2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "butter_df2_order8_cut1000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, butter_df2_order8_cut1000_f64_ba[0], butter_df2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "butter_tdf2_order8_cut1000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, butter_tdf2_order8_cut1000_f64_ba[0], butter_tdf2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "butter_tdf2_order8_cut1000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, butter_tdf2_order8_cut1000_f64_ba[0], butter_tdf2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "butter_df1_order8_cut2000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, butter_df1_order8_cut2000_f64_ba[0], butter_df1_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "butter_df1_order8_cut2000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, butter_df1_order8_cut2000_f64_ba[0], butter_df1_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "butter_df2_order8_cut2000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, butter_df2_order8_cut2000_f64_ba[0], butter_df2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "butter_df2_order8_cut2000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, butter_df2_order8_cut2000_f64_ba[0], butter_df2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "butter_tdf2_order8_cut2000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, butter_tdf2_order8_cut2000_f64_ba[0], butter_tdf2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "butter_tdf2_order8_cut2000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, butter_tdf2_order8_cut2000_f64_ba[0], butter_tdf2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "butter_df1_order8_cut5000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, butter_df1_order8_cut5000_f64_ba[0], butter_df1_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "butter_df1_order8_cut5000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, butter_df1_order8_cut5000_f64_ba[0], butter_df1_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "butter_df2_order8_cut5000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, butter_df2_order8_cut5000_f64_ba[0], butter_df2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "butter_df2_order8_cut5000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, butter_df2_order8_cut5000_f64_ba[0], butter_df2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "butter_tdf2_order8_cut5000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, butter_tdf2_order8_cut5000_f64_ba[0], butter_tdf2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "butter_tdf2_order8_cut5000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, butter_tdf2_order8_cut5000_f64_ba[0], butter_tdf2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby1_df1_order2_cut1000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby1_df1_order2_cut1000_f64_ba[0], cheby1_df1_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby1_df1_order2_cut1000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby1_df1_order2_cut1000_f64_ba[0], cheby1_df1_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby1_df2_order2_cut1000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby1_df2_order2_cut1000_f64_ba[0], cheby1_df2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby1_df2_order2_cut1000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby1_df2_order2_cut1000_f64_ba[0], cheby1_df2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order2_cut1000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby1_tdf2_order2_cut1000_f64_ba[0], cheby1_tdf2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order2_cut1000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby1_tdf2_order2_cut1000_f64_ba[0], cheby1_tdf2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby1_df1_order2_cut2000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby1_df1_order2_cut2000_f64_ba[0], cheby1_df1_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby1_df1_order2_cut2000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby1_df1_order2_cut2000_f64_ba[0], cheby1_df1_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby1_df2_order2_cut2000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby1_df2_order2_cut2000_f64_ba[0], cheby1_df2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby1_df2_order2_cut2000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby1_df2_order2_cut2000_f64_ba[0], cheby1_df2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order2_cut2000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby1_tdf2_order2_cut2000_f64_ba[0], cheby1_tdf2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order2_cut2000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby1_tdf2_order2_cut2000_f64_ba[0], cheby1_tdf2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby1_df1_order2_cut5000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby1_df1_order2_cut5000_f64_ba[0], cheby1_df1_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby1_df1_order2_cut5000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby1_df1_order2_cut5000_f64_ba[0], cheby1_df1_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby1_df2_order2_cut5000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby1_df2_order2_cut5000_f64_ba[0], cheby1_df2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby1_df2_order2_cut5000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby1_df2_order2_cut5000_f64_ba[0], cheby1_df2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order2_cut5000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby1_tdf2_order2_cut5000_f64_ba[0], cheby1_tdf2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order2_cut5000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby1_tdf2_order2_cut5000_f64_ba[0], cheby1_tdf2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby1_df1_order4_cut1000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby1_df1_order4_cut1000_f64_ba[0], cheby1_df1_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby1_df1_order4_cut1000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby1_df1_order4_cut1000_f64_ba[0], cheby1_df1_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby1_df2_order4_cut1000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby1_df2_order4_cut1000_f64_ba[0], cheby1_df2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby1_df2_order4_cut1000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby1_df2_order4_cut1000_f64_ba[0], cheby1_df2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order4_cut1000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby1_tdf2_order4_cut1000_f64_ba[0], cheby1_tdf2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order4_cut1000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby1_tdf2_order4_cut1000_f64_ba[0], cheby1_tdf2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby1_df1_order4_cut2000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby1_df1_order4_cut2000_f64_ba[0], cheby1_df1_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby1_df1_order4_cut2000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby1_df1_order4_cut2000_f64_ba[0], cheby1_df1_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby1_df2_order4_cut2000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby1_df2_order4_cut2000_f64_ba[0], cheby1_df2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby1_df2_order4_cut2000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby1_df2_order4_cut2000_f64_ba[0], cheby1_df2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order4_cut2000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby1_tdf2_order4_cut2000_f64_ba[0], cheby1_tdf2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order4_cut2000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby1_tdf2_order4_cut2000_f64_ba[0], cheby1_tdf2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby1_df1_order4_cut5000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby1_df1_order4_cut5000_f64_ba[0], cheby1_df1_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby1_df1_order4_cut5000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby1_df1_order4_cut5000_f64_ba[0], cheby1_df1_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby1_df2_order4_cut5000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby1_df2_order4_cut5000_f64_ba[0], cheby1_df2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby1_df2_order4_cut5000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby1_df2_order4_cut5000_f64_ba[0], cheby1_df2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order4_cut5000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby1_tdf2_order4_cut5000_f64_ba[0], cheby1_tdf2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order4_cut5000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby1_tdf2_order4_cut5000_f64_ba[0], cheby1_tdf2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby1_df1_order6_cut1000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby1_df1_order6_cut1000_f64_ba[0], cheby1_df1_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby1_df1_order6_cut1000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby1_df1_order6_cut1000_f64_ba[0], cheby1_df1_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby1_df2_order6_cut1000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby1_df2_order6_cut1000_f64_ba[0], cheby1_df2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby1_df2_order6_cut1000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby1_df2_order6_cut1000_f64_ba[0], cheby1_df2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order6_cut1000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby1_tdf2_order6_cut1000_f64_ba[0], cheby1_tdf2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order6_cut1000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby1_tdf2_order6_cut1000_f64_ba[0], cheby1_tdf2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby1_df1_order6_cut2000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby1_df1_order6_cut2000_f64_ba[0], cheby1_df1_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby1_df1_order6_cut2000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby1_df1_order6_cut2000_f64_ba[0], cheby1_df1_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby1_df2_order6_cut2000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby1_df2_order6_cut2000_f64_ba[0], cheby1_df2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby1_df2_order6_cut2000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby1_df2_order6_cut2000_f64_ba[0], cheby1_df2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order6_cut2000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby1_tdf2_order6_cut2000_f64_ba[0], cheby1_tdf2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order6_cut2000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby1_tdf2_order6_cut2000_f64_ba[0], cheby1_tdf2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby1_df1_order6_cut5000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby1_df1_order6_cut5000_f64_ba[0], cheby1_df1_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby1_df1_order6_cut5000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby1_df1_order6_cut5000_f64_ba[0], cheby1_df1_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby1_df2_order6_cut5000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby1_df2_order6_cut5000_f64_ba[0], cheby1_df2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby1_df2_order6_cut5000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby1_df2_order6_cut5000_f64_ba[0], cheby1_df2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order6_cut5000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby1_tdf2_order6_cut5000_f64_ba[0], cheby1_tdf2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order6_cut5000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby1_tdf2_order6_cut5000_f64_ba[0], cheby1_tdf2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby1_df1_order8_cut1000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby1_df1_order8_cut1000_f64_ba[0], cheby1_df1_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby1_df1_order8_cut1000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby1_df1_order8_cut1000_f64_ba[0], cheby1_df1_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby1_df2_order8_cut1000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby1_df2_order8_cut1000_f64_ba[0], cheby1_df2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby1_df2_order8_cut1000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby1_df2_order8_cut1000_f64_ba[0], cheby1_df2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order8_cut1000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby1_tdf2_order8_cut1000_f64_ba[0], cheby1_tdf2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order8_cut1000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby1_tdf2_order8_cut1000_f64_ba[0], cheby1_tdf2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby1_df1_order8_cut2000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby1_df1_order8_cut2000_f64_ba[0], cheby1_df1_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby1_df1_order8_cut2000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby1_df1_order8_cut2000_f64_ba[0], cheby1_df1_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby1_df2_order8_cut2000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby1_df2_order8_cut2000_f64_ba[0], cheby1_df2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby1_df2_order8_cut2000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby1_df2_order8_cut2000_f64_ba[0], cheby1_df2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order8_cut2000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby1_tdf2_order8_cut2000_f64_ba[0], cheby1_tdf2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order8_cut2000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby1_tdf2_order8_cut2000_f64_ba[0], cheby1_tdf2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby1_df1_order8_cut5000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby1_df1_order8_cut5000_f64_ba[0], cheby1_df1_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby1_df1_order8_cut5000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby1_df1_order8_cut5000_f64_ba[0], cheby1_df1_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby1_df2_order8_cut5000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby1_df2_order8_cut5000_f64_ba[0], cheby1_df2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby1_df2_order8_cut5000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby1_df2_order8_cut5000_f64_ba[0], cheby1_df2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order8_cut5000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby1_tdf2_order8_cut5000_f64_ba[0], cheby1_tdf2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby1_tdf2_order8_cut5000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby1_tdf2_order8_cut5000_f64_ba[0], cheby1_tdf2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby2_df1_order2_cut1000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby2_df1_order2_cut1000_f64_ba[0], cheby2_df1_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby2_df1_order2_cut1000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby2_df1_order2_cut1000_f64_ba[0], cheby2_df1_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby2_df2_order2_cut1000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby2_df2_order2_cut1000_f64_ba[0], cheby2_df2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby2_df2_order2_cut1000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby2_df2_order2_cut1000_f64_ba[0], cheby2_df2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order2_cut1000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby2_tdf2_order2_cut1000_f64_ba[0], cheby2_tdf2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order2_cut1000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby2_tdf2_order2_cut1000_f64_ba[0], cheby2_tdf2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby2_df1_order2_cut2000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby2_df1_order2_cut2000_f64_ba[0], cheby2_df1_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby2_df1_order2_cut2000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby2_df1_order2_cut2000_f64_ba[0], cheby2_df1_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby2_df2_order2_cut2000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby2_df2_order2_cut2000_f64_ba[0], cheby2_df2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby2_df2_order2_cut2000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby2_df2_order2_cut2000_f64_ba[0], cheby2_df2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order2_cut2000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby2_tdf2_order2_cut2000_f64_ba[0], cheby2_tdf2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order2_cut2000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby2_tdf2_order2_cut2000_f64_ba[0], cheby2_tdf2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby2_df1_order2_cut5000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby2_df1_order2_cut5000_f64_ba[0], cheby2_df1_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby2_df1_order2_cut5000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby2_df1_order2_cut5000_f64_ba[0], cheby2_df1_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby2_df2_order2_cut5000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby2_df2_order2_cut5000_f64_ba[0], cheby2_df2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby2_df2_order2_cut5000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby2_df2_order2_cut5000_f64_ba[0], cheby2_df2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order2_cut5000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby2_tdf2_order2_cut5000_f64_ba[0], cheby2_tdf2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order2_cut5000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby2_tdf2_order2_cut5000_f64_ba[0], cheby2_tdf2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "cheby2_df1_order4_cut1000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby2_df1_order4_cut1000_f64_ba[0], cheby2_df1_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby2_df1_order4_cut1000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby2_df1_order4_cut1000_f64_ba[0], cheby2_df1_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby2_df2_order4_cut1000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby2_df2_order4_cut1000_f64_ba[0], cheby2_df2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby2_df2_order4_cut1000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby2_df2_order4_cut1000_f64_ba[0], cheby2_df2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order4_cut1000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby2_tdf2_order4_cut1000_f64_ba[0], cheby2_tdf2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order4_cut1000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby2_tdf2_order4_cut1000_f64_ba[0], cheby2_tdf2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby2_df1_order4_cut2000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby2_df1_order4_cut2000_f64_ba[0], cheby2_df1_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby2_df1_order4_cut2000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby2_df1_order4_cut2000_f64_ba[0], cheby2_df1_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby2_df2_order4_cut2000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby2_df2_order4_cut2000_f64_ba[0], cheby2_df2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby2_df2_order4_cut2000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby2_df2_order4_cut2000_f64_ba[0], cheby2_df2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order4_cut2000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby2_tdf2_order4_cut2000_f64_ba[0], cheby2_tdf2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order4_cut2000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby2_tdf2_order4_cut2000_f64_ba[0], cheby2_tdf2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby2_df1_order4_cut5000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby2_df1_order4_cut5000_f64_ba[0], cheby2_df1_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby2_df1_order4_cut5000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby2_df1_order4_cut5000_f64_ba[0], cheby2_df1_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby2_df2_order4_cut5000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby2_df2_order4_cut5000_f64_ba[0], cheby2_df2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby2_df2_order4_cut5000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby2_df2_order4_cut5000_f64_ba[0], cheby2_df2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order4_cut5000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby2_tdf2_order4_cut5000_f64_ba[0], cheby2_tdf2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order4_cut5000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby2_tdf2_order4_cut5000_f64_ba[0], cheby2_tdf2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "cheby2_df1_order6_cut1000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby2_df1_order6_cut1000_f64_ba[0], cheby2_df1_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby2_df1_order6_cut1000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby2_df1_order6_cut1000_f64_ba[0], cheby2_df1_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby2_df2_order6_cut1000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby2_df2_order6_cut1000_f64_ba[0], cheby2_df2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby2_df2_order6_cut1000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby2_df2_order6_cut1000_f64_ba[0], cheby2_df2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order6_cut1000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby2_tdf2_order6_cut1000_f64_ba[0], cheby2_tdf2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order6_cut1000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby2_tdf2_order6_cut1000_f64_ba[0], cheby2_tdf2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby2_df1_order6_cut2000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby2_df1_order6_cut2000_f64_ba[0], cheby2_df1_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby2_df1_order6_cut2000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby2_df1_order6_cut2000_f64_ba[0], cheby2_df1_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby2_df2_order6_cut2000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby2_df2_order6_cut2000_f64_ba[0], cheby2_df2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby2_df2_order6_cut2000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby2_df2_order6_cut2000_f64_ba[0], cheby2_df2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order6_cut2000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby2_tdf2_order6_cut2000_f64_ba[0], cheby2_tdf2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order6_cut2000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby2_tdf2_order6_cut2000_f64_ba[0], cheby2_tdf2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby2_df1_order6_cut5000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby2_df1_order6_cut5000_f64_ba[0], cheby2_df1_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby2_df1_order6_cut5000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby2_df1_order6_cut5000_f64_ba[0], cheby2_df1_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby2_df2_order6_cut5000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby2_df2_order6_cut5000_f64_ba[0], cheby2_df2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby2_df2_order6_cut5000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby2_df2_order6_cut5000_f64_ba[0], cheby2_df2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order6_cut5000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby2_tdf2_order6_cut5000_f64_ba[0], cheby2_tdf2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order6_cut5000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby2_tdf2_order6_cut5000_f64_ba[0], cheby2_tdf2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "cheby2_df1_order8_cut1000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby2_df1_order8_cut1000_f64_ba[0], cheby2_df1_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby2_df1_order8_cut1000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby2_df1_order8_cut1000_f64_ba[0], cheby2_df1_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby2_df2_order8_cut1000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby2_df2_order8_cut1000_f64_ba[0], cheby2_df2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby2_df2_order8_cut1000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby2_df2_order8_cut1000_f64_ba[0], cheby2_df2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order8_cut1000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby2_tdf2_order8_cut1000_f64_ba[0], cheby2_tdf2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order8_cut1000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby2_tdf2_order8_cut1000_f64_ba[0], cheby2_tdf2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby2_df1_order8_cut2000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby2_df1_order8_cut2000_f64_ba[0], cheby2_df1_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby2_df1_order8_cut2000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby2_df1_order8_cut2000_f64_ba[0], cheby2_df1_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby2_df2_order8_cut2000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby2_df2_order8_cut2000_f64_ba[0], cheby2_df2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby2_df2_order8_cut2000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby2_df2_order8_cut2000_f64_ba[0], cheby2_df2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order8_cut2000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby2_tdf2_order8_cut2000_f64_ba[0], cheby2_tdf2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order8_cut2000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby2_tdf2_order8_cut2000_f64_ba[0], cheby2_tdf2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby2_df1_order8_cut5000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, cheby2_df1_order8_cut5000_f64_ba[0], cheby2_df1_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby2_df1_order8_cut5000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, cheby2_df1_order8_cut5000_f64_ba[0], cheby2_df1_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby2_df2_order8_cut5000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, cheby2_df2_order8_cut5000_f64_ba[0], cheby2_df2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby2_df2_order8_cut5000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, cheby2_df2_order8_cut5000_f64_ba[0], cheby2_df2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order8_cut5000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, cheby2_tdf2_order8_cut5000_f64_ba[0], cheby2_tdf2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "cheby2_tdf2_order8_cut5000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, cheby2_tdf2_order8_cut5000_f64_ba[0], cheby2_tdf2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "ellip_df1_order2_cut1000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, ellip_df1_order2_cut1000_f64_ba[0], ellip_df1_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "ellip_df1_order2_cut1000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, ellip_df1_order2_cut1000_f64_ba[0], ellip_df1_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "ellip_df2_order2_cut1000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, ellip_df2_order2_cut1000_f64_ba[0], ellip_df2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "ellip_df2_order2_cut1000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, ellip_df2_order2_cut1000_f64_ba[0], ellip_df2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "ellip_tdf2_order2_cut1000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, ellip_tdf2_order2_cut1000_f64_ba[0], ellip_tdf2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "ellip_tdf2_order2_cut1000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, ellip_tdf2_order2_cut1000_f64_ba[0], ellip_tdf2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "ellip_df1_order2_cut2000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, ellip_df1_order2_cut2000_f64_ba[0], ellip_df1_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "ellip_df1_order2_cut2000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, ellip_df1_order2_cut2000_f64_ba[0], ellip_df1_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "ellip_df2_order2_cut2000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, ellip_df2_order2_cut2000_f64_ba[0], ellip_df2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "ellip_df2_order2_cut2000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, ellip_df2_order2_cut2000_f64_ba[0], ellip_df2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "ellip_tdf2_order2_cut2000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, ellip_tdf2_order2_cut2000_f64_ba[0], ellip_tdf2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "ellip_tdf2_order2_cut2000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, ellip_tdf2_order2_cut2000_f64_ba[0], ellip_tdf2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "ellip_df1_order2_cut5000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, ellip_df1_order2_cut5000_f64_ba[0], ellip_df1_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "ellip_df1_order2_cut5000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, ellip_df1_order2_cut5000_f64_ba[0], ellip_df1_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "ellip_df2_order2_cut5000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, ellip_df2_order2_cut5000_f64_ba[0], ellip_df2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "ellip_df2_order2_cut5000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, ellip_df2_order2_cut5000_f64_ba[0], ellip_df2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "ellip_tdf2_order2_cut5000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, ellip_tdf2_order2_cut5000_f64_ba[0], ellip_tdf2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "ellip_tdf2_order2_cut5000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, ellip_tdf2_order2_cut5000_f64_ba[0], ellip_tdf2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "ellip_df1_order4_cut1000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, ellip_df1_order4_cut1000_f64_ba[0], ellip_df1_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "ellip_df1_order4_cut1000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, ellip_df1_order4_cut1000_f64_ba[0], ellip_df1_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "ellip_df2_order4_cut1000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, ellip_df2_order4_cut1000_f64_ba[0], ellip_df2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "ellip_df2_order4_cut1000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, ellip_df2_order4_cut1000_f64_ba[0], ellip_df2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "ellip_tdf2_order4_cut1000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, ellip_tdf2_order4_cut1000_f64_ba[0], ellip_tdf2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "ellip_tdf2_order4_cut1000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, ellip_tdf2_order4_cut1000_f64_ba[0], ellip_tdf2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "ellip_df1_order4_cut2000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, ellip_df1_order4_cut2000_f64_ba[0], ellip_df1_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "ellip_df1_order4_cut2000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, ellip_df1_order4_cut2000_f64_ba[0], ellip_df1_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "ellip_df2_order4_cut2000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, ellip_df2_order4_cut2000_f64_ba[0], ellip_df2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "ellip_df2_order4_cut2000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, ellip_df2_order4_cut2000_f64_ba[0], ellip_df2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "ellip_tdf2_order4_cut2000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, ellip_tdf2_order4_cut2000_f64_ba[0], ellip_tdf2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "ellip_tdf2_order4_cut2000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, ellip_tdf2_order4_cut2000_f64_ba[0], ellip_tdf2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "ellip_df1_order4_cut5000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, ellip_df1_order4_cut5000_f64_ba[0], ellip_df1_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "ellip_df1_order4_cut5000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, ellip_df1_order4_cut5000_f64_ba[0], ellip_df1_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "ellip_df2_order4_cut5000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, ellip_df2_order4_cut5000_f64_ba[0], ellip_df2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "ellip_df2_order4_cut5000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, ellip_df2_order4_cut5000_f64_ba[0], ellip_df2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "ellip_tdf2_order4_cut5000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, ellip_tdf2_order4_cut5000_f64_ba[0], ellip_tdf2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "ellip_tdf2_order4_cut5000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, ellip_tdf2_order4_cut5000_f64_ba[0], ellip_tdf2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "ellip_df1_order6_cut1000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, ellip_df1_order6_cut1000_f64_ba[0], ellip_df1_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "ellip_df1_order6_cut1000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, ellip_df1_order6_cut1000_f64_ba[0], ellip_df1_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "ellip_df2_order6_cut1000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, ellip_df2_order6_cut1000_f64_ba[0], ellip_df2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "ellip_df2_order6_cut1000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, ellip_df2_order6_cut1000_f64_ba[0], ellip_df2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "ellip_tdf2_order6_cut1000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, ellip_tdf2_order6_cut1000_f64_ba[0], ellip_tdf2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "ellip_tdf2_order6_cut1000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, ellip_tdf2_order6_cut1000_f64_ba[0], ellip_tdf2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "ellip_df1_order6_cut2000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, ellip_df1_order6_cut2000_f64_ba[0], ellip_df1_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "ellip_df1_order6_cut2000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, ellip_df1_order6_cut2000_f64_ba[0], ellip_df1_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "ellip_df2_order6_cut2000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, ellip_df2_order6_cut2000_f64_ba[0], ellip_df2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "ellip_df2_order6_cut2000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, ellip_df2_order6_cut2000_f64_ba[0], ellip_df2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "ellip_tdf2_order6_cut2000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, ellip_tdf2_order6_cut2000_f64_ba[0], ellip_tdf2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "ellip_tdf2_order6_cut2000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, ellip_tdf2_order6_cut2000_f64_ba[0], ellip_tdf2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "ellip_df1_order6_cut5000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, ellip_df1_order6_cut5000_f64_ba[0], ellip_df1_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "ellip_df1_order6_cut5000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, ellip_df1_order6_cut5000_f64_ba[0], ellip_df1_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "ellip_df2_order6_cut5000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, ellip_df2_order6_cut5000_f64_ba[0], ellip_df2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "ellip_df2_order6_cut5000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, ellip_df2_order6_cut5000_f64_ba[0], ellip_df2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "ellip_tdf2_order6_cut5000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, ellip_tdf2_order6_cut5000_f64_ba[0], ellip_tdf2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "ellip_tdf2_order6_cut5000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, ellip_tdf2_order6_cut5000_f64_ba[0], ellip_tdf2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "ellip_df1_order8_cut1000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, ellip_df1_order8_cut1000_f64_ba[0], ellip_df1_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "ellip_df1_order8_cut1000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, ellip_df1_order8_cut1000_f64_ba[0], ellip_df1_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "ellip_df2_order8_cut1000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, ellip_df2_order8_cut1000_f64_ba[0], ellip_df2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "ellip_df2_order8_cut1000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, ellip_df2_order8_cut1000_f64_ba[0], ellip_df2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "ellip_tdf2_order8_cut1000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, ellip_tdf2_order8_cut1000_f64_ba[0], ellip_tdf2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "ellip_tdf2_order8_cut1000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, ellip_tdf2_order8_cut1000_f64_ba[0], ellip_tdf2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "ellip_df1_order8_cut2000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, ellip_df1_order8_cut2000_f64_ba[0], ellip_df1_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "ellip_df1_order8_cut2000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, ellip_df1_order8_cut2000_f64_ba[0], ellip_df1_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "ellip_df2_order8_cut2000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, ellip_df2_order8_cut2000_f64_ba[0], ellip_df2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "ellip_df2_order8_cut2000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, ellip_df2_order8_cut2000_f64_ba[0], ellip_df2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "ellip_tdf2_order8_cut2000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, ellip_tdf2_order8_cut2000_f64_ba[0], ellip_tdf2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "ellip_tdf2_order8_cut2000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, ellip_tdf2_order8_cut2000_f64_ba[0], ellip_tdf2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "ellip_df1_order8_cut5000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, ellip_df1_order8_cut5000_f64_ba[0], ellip_df1_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "ellip_df1_order8_cut5000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, ellip_df1_order8_cut5000_f64_ba[0], ellip_df1_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "ellip_df2_order8_cut5000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, ellip_df2_order8_cut5000_f64_ba[0], ellip_df2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "ellip_df2_order8_cut5000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, ellip_df2_order8_cut5000_f64_ba[0], ellip_df2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "ellip_tdf2_order8_cut5000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, ellip_tdf2_order8_cut5000_f64_ba[0], ellip_tdf2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "ellip_tdf2_order8_cut5000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, ellip_tdf2_order8_cut5000_f64_ba[0], ellip_tdf2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "bessel_df1_order2_cut1000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, bessel_df1_order2_cut1000_f64_ba[0], bessel_df1_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "bessel_df1_order2_cut1000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, bessel_df1_order2_cut1000_f64_ba[0], bessel_df1_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "bessel_df2_order2_cut1000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, bessel_df2_order2_cut1000_f64_ba[0], bessel_df2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "bessel_df2_order2_cut1000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, bessel_df2_order2_cut1000_f64_ba[0], bessel_df2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "bessel_tdf2_order2_cut1000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, bessel_tdf2_order2_cut1000_f64_ba[0], bessel_tdf2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "bessel_tdf2_order2_cut1000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, bessel_tdf2_order2_cut1000_f64_ba[0], bessel_tdf2_order2_cut1000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "bessel_df1_order2_cut2000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, bessel_df1_order2_cut2000_f64_ba[0], bessel_df1_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "bessel_df1_order2_cut2000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, bessel_df1_order2_cut2000_f64_ba[0], bessel_df1_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "bessel_df2_order2_cut2000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, bessel_df2_order2_cut2000_f64_ba[0], bessel_df2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "bessel_df2_order2_cut2000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, bessel_df2_order2_cut2000_f64_ba[0], bessel_df2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "bessel_tdf2_order2_cut2000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, bessel_tdf2_order2_cut2000_f64_ba[0], bessel_tdf2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "bessel_tdf2_order2_cut2000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, bessel_tdf2_order2_cut2000_f64_ba[0], bessel_tdf2_order2_cut2000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "bessel_df1_order2_cut5000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, bessel_df1_order2_cut5000_f64_ba[0], bessel_df1_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "bessel_df1_order2_cut5000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, bessel_df1_order2_cut5000_f64_ba[0], bessel_df1_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "bessel_df2_order2_cut5000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, bessel_df2_order2_cut5000_f64_ba[0], bessel_df2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "bessel_df2_order2_cut5000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, bessel_df2_order2_cut5000_f64_ba[0], bessel_df2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "bessel_tdf2_order2_cut5000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, bessel_tdf2_order2_cut5000_f64_ba[0], bessel_tdf2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "bessel_tdf2_order2_cut5000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, bessel_tdf2_order2_cut5000_f64_ba[0], bessel_tdf2_order2_cut5000_f64_ba[1], 3);
    precision_analysis_q(fp_precision, "bessel_df1_order4_cut1000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, bessel_df1_order4_cut1000_f64_ba[0], bessel_df1_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "bessel_df1_order4_cut1000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, bessel_df1_order4_cut1000_f64_ba[0], bessel_df1_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "bessel_df2_order4_cut1000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, bessel_df2_order4_cut1000_f64_ba[0], bessel_df2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "bessel_df2_order4_cut1000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, bessel_df2_order4_cut1000_f64_ba[0], bessel_df2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "bessel_tdf2_order4_cut1000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, bessel_tdf2_order4_cut1000_f64_ba[0], bessel_tdf2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "bessel_tdf2_order4_cut1000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, bessel_tdf2_order4_cut1000_f64_ba[0], bessel_tdf2_order4_cut1000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "bessel_df1_order4_cut2000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, bessel_df1_order4_cut2000_f64_ba[0], bessel_df1_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "bessel_df1_order4_cut2000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, bessel_df1_order4_cut2000_f64_ba[0], bessel_df1_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "bessel_df2_order4_cut2000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, bessel_df2_order4_cut2000_f64_ba[0], bessel_df2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "bessel_df2_order4_cut2000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, bessel_df2_order4_cut2000_f64_ba[0], bessel_df2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "bessel_tdf2_order4_cut2000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, bessel_tdf2_order4_cut2000_f64_ba[0], bessel_tdf2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "bessel_tdf2_order4_cut2000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, bessel_tdf2_order4_cut2000_f64_ba[0], bessel_tdf2_order4_cut2000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "bessel_df1_order4_cut5000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, bessel_df1_order4_cut5000_f64_ba[0], bessel_df1_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "bessel_df1_order4_cut5000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, bessel_df1_order4_cut5000_f64_ba[0], bessel_df1_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "bessel_df2_order4_cut5000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, bessel_df2_order4_cut5000_f64_ba[0], bessel_df2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "bessel_df2_order4_cut5000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, bessel_df2_order4_cut5000_f64_ba[0], bessel_df2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "bessel_tdf2_order4_cut5000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, bessel_tdf2_order4_cut5000_f64_ba[0], bessel_tdf2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "bessel_tdf2_order4_cut5000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, bessel_tdf2_order4_cut5000_f64_ba[0], bessel_tdf2_order4_cut5000_f64_ba[1], 5);
    precision_analysis_q(fp_precision, "bessel_df1_order6_cut1000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, bessel_df1_order6_cut1000_f64_ba[0], bessel_df1_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "bessel_df1_order6_cut1000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, bessel_df1_order6_cut1000_f64_ba[0], bessel_df1_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "bessel_df2_order6_cut1000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, bessel_df2_order6_cut1000_f64_ba[0], bessel_df2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "bessel_df2_order6_cut1000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, bessel_df2_order6_cut1000_f64_ba[0], bessel_df2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "bessel_tdf2_order6_cut1000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, bessel_tdf2_order6_cut1000_f64_ba[0], bessel_tdf2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "bessel_tdf2_order6_cut1000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, bessel_tdf2_order6_cut1000_f64_ba[0], bessel_tdf2_order6_cut1000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "bessel_df1_order6_cut2000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, bessel_df1_order6_cut2000_f64_ba[0], bessel_df1_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "bessel_df1_order6_cut2000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, bessel_df1_order6_cut2000_f64_ba[0], bessel_df1_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "bessel_df2_order6_cut2000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, bessel_df2_order6_cut2000_f64_ba[0], bessel_df2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "bessel_df2_order6_cut2000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, bessel_df2_order6_cut2000_f64_ba[0], bessel_df2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "bessel_tdf2_order6_cut2000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, bessel_tdf2_order6_cut2000_f64_ba[0], bessel_tdf2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "bessel_tdf2_order6_cut2000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, bessel_tdf2_order6_cut2000_f64_ba[0], bessel_tdf2_order6_cut2000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "bessel_df1_order6_cut5000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, bessel_df1_order6_cut5000_f64_ba[0], bessel_df1_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "bessel_df1_order6_cut5000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, bessel_df1_order6_cut5000_f64_ba[0], bessel_df1_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "bessel_df2_order6_cut5000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, bessel_df2_order6_cut5000_f64_ba[0], bessel_df2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "bessel_df2_order6_cut5000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, bessel_df2_order6_cut5000_f64_ba[0], bessel_df2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "bessel_tdf2_order6_cut5000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, bessel_tdf2_order6_cut5000_f64_ba[0], bessel_tdf2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "bessel_tdf2_order6_cut5000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, bessel_tdf2_order6_cut5000_f64_ba[0], bessel_tdf2_order6_cut5000_f64_ba[1], 7);
    precision_analysis_q(fp_precision, "bessel_df1_order8_cut1000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, bessel_df1_order8_cut1000_f64_ba[0], bessel_df1_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "bessel_df1_order8_cut1000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, bessel_df1_order8_cut1000_f64_ba[0], bessel_df1_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "bessel_df2_order8_cut1000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, bessel_df2_order8_cut1000_f64_ba[0], bessel_df2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "bessel_df2_order8_cut1000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, bessel_df2_order8_cut1000_f64_ba[0], bessel_df2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "bessel_tdf2_order8_cut1000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, bessel_tdf2_order8_cut1000_f64_ba[0], bessel_tdf2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "bessel_tdf2_order8_cut1000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, bessel_tdf2_order8_cut1000_f64_ba[0], bessel_tdf2_order8_cut1000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "bessel_df1_order8_cut2000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, bessel_df1_order8_cut2000_f64_ba[0], bessel_df1_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "bessel_df1_order8_cut2000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, bessel_df1_order8_cut2000_f64_ba[0], bessel_df1_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "bessel_df2_order8_cut2000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, bessel_df2_order8_cut2000_f64_ba[0], bessel_df2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "bessel_df2_order8_cut2000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, bessel_df2_order8_cut2000_f64_ba[0], bessel_df2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "bessel_tdf2_order8_cut2000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, bessel_tdf2_order8_cut2000_f64_ba[0], bessel_tdf2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "bessel_tdf2_order8_cut2000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, bessel_tdf2_order8_cut2000_f64_ba[0], bessel_tdf2_order8_cut2000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "bessel_df1_order8_cut5000", "DF1", 24, (void (*)(void*, void*, void*, void*, int, int))DF1_q24, DF1_d, bessel_df1_order8_cut5000_f64_ba[0], bessel_df1_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "bessel_df1_order8_cut5000", "DF1", 12, (void (*)(void*, void*, void*, void*, int, int))DF1_q12, DF1_d, bessel_df1_order8_cut5000_f64_ba[0], bessel_df1_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "bessel_df2_order8_cut5000", "DF2", 24, (void (*)(void*, void*, void*, void*, int, int))DF2_q24, DF2_d, bessel_df2_order8_cut5000_f64_ba[0], bessel_df2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "bessel_df2_order8_cut5000", "DF2", 12, (void (*)(void*, void*, void*, void*, int, int))DF2_q12, DF2_d, bessel_df2_order8_cut5000_f64_ba[0], bessel_df2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "bessel_tdf2_order8_cut5000", "TDF2", 24, (void (*)(void*, void*, void*, void*, int, int))TDF2_q24, TDF2_d, bessel_tdf2_order8_cut5000_f64_ba[0], bessel_tdf2_order8_cut5000_f64_ba[1], 9);
    precision_analysis_q(fp_precision, "bessel_tdf2_order8_cut5000", "TDF2", 12, (void (*)(void*, void*, void*, void*, int, int))TDF2_q12, TDF2_d, bessel_tdf2_order8_cut5000_f64_ba[0], bessel_tdf2_order8_cut5000_f64_ba[1], 9);

    // --- CASCADE (SOS) ---

    precision_analysis_sos(fp_precision, "butter_cascade_order2_cut1000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order2_cut1000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "butter_cascade_order2_cut1000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order2_cut1000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "butter_cascade_order2_cut2000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order2_cut2000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "butter_cascade_order2_cut2000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order2_cut2000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "butter_cascade_order2_cut5000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order2_cut5000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "butter_cascade_order2_cut5000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order2_cut5000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "butter_cascade_order4_cut1000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order4_cut1000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "butter_cascade_order4_cut1000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order4_cut1000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "butter_cascade_order4_cut2000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order4_cut2000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "butter_cascade_order4_cut2000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order4_cut2000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "butter_cascade_order4_cut5000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order4_cut5000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "butter_cascade_order4_cut5000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order4_cut5000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "butter_cascade_order6_cut1000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order6_cut1000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "butter_cascade_order6_cut1000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order6_cut1000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "butter_cascade_order6_cut2000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order6_cut2000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "butter_cascade_order6_cut2000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order6_cut2000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "butter_cascade_order6_cut5000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order6_cut5000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "butter_cascade_order6_cut5000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order6_cut5000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "butter_cascade_order8_cut1000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order8_cut1000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "butter_cascade_order8_cut1000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order8_cut1000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "butter_cascade_order8_cut2000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order8_cut2000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "butter_cascade_order8_cut2000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order8_cut2000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "butter_cascade_order8_cut5000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order8_cut5000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "butter_cascade_order8_cut5000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, butter_cascade_order8_cut5000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order2_cut1000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order2_cut1000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order2_cut1000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order2_cut1000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order2_cut2000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order2_cut2000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order2_cut2000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order2_cut2000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order2_cut5000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order2_cut5000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order2_cut5000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order2_cut5000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order4_cut1000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order4_cut1000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order4_cut1000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order4_cut1000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order4_cut2000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order4_cut2000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order4_cut2000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order4_cut2000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order4_cut5000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order4_cut5000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order4_cut5000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order4_cut5000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order6_cut1000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order6_cut1000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order6_cut1000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order6_cut1000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order6_cut2000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order6_cut2000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order6_cut2000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order6_cut2000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order6_cut5000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order6_cut5000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order6_cut5000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order6_cut5000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order8_cut1000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order8_cut1000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order8_cut1000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order8_cut1000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order8_cut2000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order8_cut2000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order8_cut2000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order8_cut2000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order8_cut5000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order8_cut5000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "cheby1_cascade_order8_cut5000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby1_cascade_order8_cut5000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order2_cut1000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order2_cut1000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order2_cut1000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order2_cut1000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order2_cut2000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order2_cut2000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order2_cut2000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order2_cut2000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order2_cut5000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order2_cut5000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order2_cut5000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order2_cut5000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order4_cut1000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order4_cut1000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order4_cut1000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order4_cut1000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order4_cut2000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order4_cut2000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order4_cut2000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order4_cut2000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order4_cut5000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order4_cut5000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order4_cut5000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order4_cut5000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order6_cut1000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order6_cut1000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order6_cut1000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order6_cut1000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order6_cut2000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order6_cut2000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order6_cut2000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order6_cut2000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order6_cut5000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order6_cut5000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order6_cut5000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order6_cut5000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order8_cut1000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order8_cut1000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order8_cut1000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order8_cut1000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order8_cut2000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order8_cut2000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order8_cut2000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order8_cut2000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order8_cut5000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order8_cut5000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "cheby2_cascade_order8_cut5000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, cheby2_cascade_order8_cut5000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "ellip_cascade_order2_cut1000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order2_cut1000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "ellip_cascade_order2_cut1000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order2_cut1000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "ellip_cascade_order2_cut2000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order2_cut2000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "ellip_cascade_order2_cut2000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order2_cut2000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "ellip_cascade_order2_cut5000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order2_cut5000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "ellip_cascade_order2_cut5000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order2_cut5000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "ellip_cascade_order4_cut1000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order4_cut1000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "ellip_cascade_order4_cut1000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order4_cut1000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "ellip_cascade_order4_cut2000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order4_cut2000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "ellip_cascade_order4_cut2000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order4_cut2000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "ellip_cascade_order4_cut5000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order4_cut5000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "ellip_cascade_order4_cut5000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order4_cut5000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "ellip_cascade_order6_cut1000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order6_cut1000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "ellip_cascade_order6_cut1000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order6_cut1000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "ellip_cascade_order6_cut2000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order6_cut2000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "ellip_cascade_order6_cut2000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order6_cut2000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "ellip_cascade_order6_cut5000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order6_cut5000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "ellip_cascade_order6_cut5000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order6_cut5000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "ellip_cascade_order8_cut1000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order8_cut1000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "ellip_cascade_order8_cut1000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order8_cut1000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "ellip_cascade_order8_cut2000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order8_cut2000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "ellip_cascade_order8_cut2000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order8_cut2000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "ellip_cascade_order8_cut5000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order8_cut5000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "ellip_cascade_order8_cut5000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, ellip_cascade_order8_cut5000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "bessel_cascade_order2_cut1000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order2_cut1000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "bessel_cascade_order2_cut1000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order2_cut1000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "bessel_cascade_order2_cut2000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order2_cut2000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "bessel_cascade_order2_cut2000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order2_cut2000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "bessel_cascade_order2_cut5000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order2_cut5000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "bessel_cascade_order2_cut5000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order2_cut5000_f64_sos, 1);
    precision_analysis_sos(fp_precision, "bessel_cascade_order4_cut1000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order4_cut1000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "bessel_cascade_order4_cut1000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order4_cut1000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "bessel_cascade_order4_cut2000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order4_cut2000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "bessel_cascade_order4_cut2000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order4_cut2000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "bessel_cascade_order4_cut5000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order4_cut5000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "bessel_cascade_order4_cut5000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order4_cut5000_f64_sos, 2);
    precision_analysis_sos(fp_precision, "bessel_cascade_order6_cut1000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order6_cut1000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "bessel_cascade_order6_cut1000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order6_cut1000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "bessel_cascade_order6_cut2000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order6_cut2000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "bessel_cascade_order6_cut2000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order6_cut2000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "bessel_cascade_order6_cut5000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order6_cut5000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "bessel_cascade_order6_cut5000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order6_cut5000_f64_sos, 3);
    precision_analysis_sos(fp_precision, "bessel_cascade_order8_cut1000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order8_cut1000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "bessel_cascade_order8_cut1000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order8_cut1000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "bessel_cascade_order8_cut2000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order8_cut2000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "bessel_cascade_order8_cut2000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order8_cut2000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "bessel_cascade_order8_cut5000", "CASCADE", 24, (void (*)(void*, void*, void*, int, int))CASCADE_q24, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order8_cut5000_f64_sos, 4);
    precision_analysis_sos(fp_precision, "bessel_cascade_order8_cut5000", "CASCADE", 12, (void (*)(void*, void*, void*, int, int))CASCADE_q12, (void (*)(double*, double*, const double[][6], int, int))CASCADE_d, bessel_cascade_order8_cut5000_f64_sos, 4);

    fclose(fp_precision);
    return 0;
}