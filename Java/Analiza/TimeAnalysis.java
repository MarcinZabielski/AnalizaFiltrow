package Analiza;

import filters.DF1;
import filters.DF2;
import filters.TDF2;
import filters.CASCADE;
import filters.FilterFunction;

import java.io.FileWriter;
import java.io.IOException;
import java.util.Locale;
import java.util.Random;

import coeffs.FilterCoeffs;

// Analiza szybkości filtrowania w języku java

public class TimeAnalysis {

    static final int FS = 48000;
    static final int DURATION_MIN = 1;
    static final int N = FS * 60 * DURATION_MIN;
    static final Random rand = new Random();

    // === Szum ===
    static void generateWhiteNoise(float[] x) {
        for (int i = 0; i < x.length; i++) {
            x[i] = 2.0f * rand.nextFloat() - 1.0f;
        }
    }

    static void generateWhiteNoise(double[] x) {
        for (int i = 0; i < x.length; i++) {
            x[i] = 2.0 * rand.nextDouble() - 1.0;
        }
    }

    public static float[] flattenSOS(float[][] sosMatrix) {
        int sections = sosMatrix.length;
        float[] flat = new float[sections * 6];
        for (int i = 0; i < sections; i++) {
            System.arraycopy(sosMatrix[i], 0, flat, i * 6, 6);
        }
        return flat;
    }

    public static double[] flattenSOS(double[][] sosMatrix) {
        int sections = sosMatrix.length;
        double[] flat = new double[sections * 6];
        for (int i = 0; i < sections; i++) {
            System.arraycopy(sosMatrix[i], 0, flat, i * 6, 6);
        }
        return flat;
    }

    // === Benchmark ===
    static void benchmarkAndLog(FileWriter fp, String filterName, String type, String structure,
                                FilterFunction.FloatFilter funcF,
                                FilterFunction.DoubleFilter funcD,
                                float[] bF, float[] aF,
                                double[] bD, double[] aD, int order) throws IOException {

        int cutoff = parseCutoff(filterName);
        String filterType = parseFilterType(filterName);

        if ("float".equals(type) && funcF != null) {
            float[] x = new float[N];
            float[] y = new float[N];
            generateWhiteNoise(x);
            long start = System.nanoTime();
            funcF.apply(x, y, bF, aF, N, order);
            long end = System.nanoTime();
            double seconds = (end - start) / 1e9;
            fp.write(String.format(Locale.US,"%s,float,%s,%d,%d,%.6f\n", filterType, structure, cutoff, order - 1, seconds));
            System.out.printf("[LOG] %s (float, %s, cut %d, order %d): %.6f sec\n", filterType, structure, cutoff, order - 1, seconds);
        }

        if ("double".equals(type) && funcD != null) {
            double[] x = new double[N];
            double[] y = new double[N];
            generateWhiteNoise(x);
            long start = System.nanoTime();
            funcD.apply(x, y, bD, aD, N, order);
            long end = System.nanoTime();
            double seconds = (end - start) / 1e9;
            fp.write(String.format(Locale.US,"%s,double,%s,%d,%d,%.6f\n", filterType, structure, cutoff, order - 1, seconds));
            System.out.printf("[LOG] %s (double, %s, cut %d, order %d): %.6f sec\n", filterType, structure, cutoff, order - 1, seconds);
        }
    }

    static void benchmarkCascadeAndLog(FileWriter fp, String filterName, String type,
                                       FilterFunction.FloatCascade funcF,
                                       FilterFunction.DoubleCascade funcD,
                                       float[][] sosF, double[][] sosD, int sections) throws IOException {

        int cutoff = parseCutoff(filterName);
        String filterType = parseFilterType(filterName);

        float[] sosFlatF = (sosF != null) ? flattenSOS(sosF) : null;
        double[] sosFlatD = (sosD != null) ? flattenSOS(sosD) : null;

        if ("float".equals(type) && funcF != null) {
            float[] x = new float[N];
            float[] y = new float[N];
            generateWhiteNoise(x);
            long start = System.nanoTime();
            funcF.apply(x, y, sosFlatF, N, sections);
            long end = System.nanoTime();
            double seconds = (end - start) / 1e9;
            fp.write(String.format(Locale.US,"%s,float,CASCADE,%d,%d,%.6f\n", filterType, cutoff, 2 * sections, seconds));
            System.out.printf("[LOG] %s (float, CASCADE, cut %d, order %d): %.6f sec\n", filterType, cutoff, 2 * sections, seconds);
        }

        if ("double".equals(type) && funcD != null) {
            double[] x = new double[N];
            double[] y = new double[N];
            generateWhiteNoise(x);
            long start = System.nanoTime();
            funcD.apply(x, y, sosFlatD, N, sections);
            long end = System.nanoTime();
            double seconds = (end - start) / 1e9;
            fp.write(String.format(Locale.US,"%s,double,CASCADE,%d,%d,%.6f\n", filterType, cutoff, 2 * sections, seconds));
            System.out.printf("[LOG] %s (double, CASCADE, cut %d, order %d): %.6f sec\n", filterType, cutoff, 2 * sections, seconds);
        }
    }

    private static int parseCutoff(String name) {
        int idx = name.indexOf("_cut");
        if (idx != -1) {
            String substr = name.substring(idx + 4);
            return Integer.parseInt(substr.split("_")[0]);
        }
        return -1;
    }

    private static String parseFilterType(String name) {
        return name.split("_")[0];
    }

    // === MAIN ===
    public static void main(String[] args) {
        try (FileWriter fp = new FileWriter("java_time_results.csv")) {
            fp.write("filter_name,type,structure,cutoff,order,time_seconds\n");

            for (int i = 0; i < 20; i++) {
                // Tu wkleić zawartość pliku generated_calls_time_java.txt
                // === AUTO-GENERATED TIME ANALYSIS CALLS ===
                // --- DF1 / DF2 / TDF2 ---
                benchmarkAndLog(fp, "butter_df1_order2_cut1000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BUTTER_DF1_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER2_CUT1000_F64_BA[1], 3);
                benchmarkAndLog(fp, "butter_df2_order2_cut1000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BUTTER_DF2_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER2_CUT1000_F64_BA[1], 3);
                benchmarkAndLog(fp, "butter_tdf2_order2_cut1000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BUTTER_TDF2_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER2_CUT1000_F64_BA[1], 3);
                benchmarkAndLog(fp, "butter_df1_order2_cut1000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BUTTER_DF1_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER2_CUT1000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "butter_df2_order2_cut1000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BUTTER_DF2_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER2_CUT1000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "butter_tdf2_order2_cut1000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BUTTER_TDF2_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER2_CUT1000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "butter_df1_order2_cut2000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BUTTER_DF1_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER2_CUT2000_F64_BA[1], 3);
                benchmarkAndLog(fp, "butter_df2_order2_cut2000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BUTTER_DF2_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER2_CUT2000_F64_BA[1], 3);
                benchmarkAndLog(fp, "butter_tdf2_order2_cut2000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BUTTER_TDF2_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER2_CUT2000_F64_BA[1], 3);
                benchmarkAndLog(fp, "butter_df1_order2_cut2000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BUTTER_DF1_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER2_CUT2000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "butter_df2_order2_cut2000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BUTTER_DF2_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER2_CUT2000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "butter_tdf2_order2_cut2000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BUTTER_TDF2_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER2_CUT2000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "butter_df1_order2_cut5000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BUTTER_DF1_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER2_CUT5000_F64_BA[1], 3);
                benchmarkAndLog(fp, "butter_df2_order2_cut5000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BUTTER_DF2_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER2_CUT5000_F64_BA[1], 3);
                benchmarkAndLog(fp, "butter_tdf2_order2_cut5000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BUTTER_TDF2_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER2_CUT5000_F64_BA[1], 3);
                benchmarkAndLog(fp, "butter_df1_order2_cut5000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BUTTER_DF1_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER2_CUT5000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "butter_df2_order2_cut5000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BUTTER_DF2_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER2_CUT5000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "butter_tdf2_order2_cut5000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BUTTER_TDF2_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER2_CUT5000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "butter_df1_order4_cut1000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BUTTER_DF1_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER4_CUT1000_F64_BA[1], 5);
                benchmarkAndLog(fp, "butter_df2_order4_cut1000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BUTTER_DF2_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER4_CUT1000_F64_BA[1], 5);
                benchmarkAndLog(fp, "butter_tdf2_order4_cut1000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BUTTER_TDF2_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER4_CUT1000_F64_BA[1], 5);
                benchmarkAndLog(fp, "butter_df1_order4_cut1000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BUTTER_DF1_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER4_CUT1000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "butter_df2_order4_cut1000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BUTTER_DF2_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER4_CUT1000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "butter_tdf2_order4_cut1000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BUTTER_TDF2_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER4_CUT1000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "butter_df1_order4_cut2000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BUTTER_DF1_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER4_CUT2000_F64_BA[1], 5);
                benchmarkAndLog(fp, "butter_df2_order4_cut2000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BUTTER_DF2_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER4_CUT2000_F64_BA[1], 5);
                benchmarkAndLog(fp, "butter_tdf2_order4_cut2000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BUTTER_TDF2_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER4_CUT2000_F64_BA[1], 5);
                benchmarkAndLog(fp, "butter_df1_order4_cut2000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BUTTER_DF1_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER4_CUT2000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "butter_df2_order4_cut2000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BUTTER_DF2_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER4_CUT2000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "butter_tdf2_order4_cut2000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BUTTER_TDF2_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER4_CUT2000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "butter_df1_order4_cut5000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BUTTER_DF1_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER4_CUT5000_F64_BA[1], 5);
                benchmarkAndLog(fp, "butter_df2_order4_cut5000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BUTTER_DF2_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER4_CUT5000_F64_BA[1], 5);
                benchmarkAndLog(fp, "butter_tdf2_order4_cut5000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BUTTER_TDF2_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER4_CUT5000_F64_BA[1], 5);
                benchmarkAndLog(fp, "butter_df1_order4_cut5000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BUTTER_DF1_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER4_CUT5000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "butter_df2_order4_cut5000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BUTTER_DF2_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER4_CUT5000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "butter_tdf2_order4_cut5000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BUTTER_TDF2_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER4_CUT5000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "butter_df1_order6_cut1000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BUTTER_DF1_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER6_CUT1000_F64_BA[1], 7);
                benchmarkAndLog(fp, "butter_df2_order6_cut1000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BUTTER_DF2_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER6_CUT1000_F64_BA[1], 7);
                benchmarkAndLog(fp, "butter_tdf2_order6_cut1000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BUTTER_TDF2_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER6_CUT1000_F64_BA[1], 7);
                benchmarkAndLog(fp, "butter_df1_order6_cut1000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BUTTER_DF1_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER6_CUT1000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "butter_df2_order6_cut1000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BUTTER_DF2_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER6_CUT1000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "butter_tdf2_order6_cut1000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BUTTER_TDF2_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER6_CUT1000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "butter_df1_order6_cut2000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BUTTER_DF1_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER6_CUT2000_F64_BA[1], 7);
                benchmarkAndLog(fp, "butter_df2_order6_cut2000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BUTTER_DF2_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER6_CUT2000_F64_BA[1], 7);
                benchmarkAndLog(fp, "butter_tdf2_order6_cut2000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BUTTER_TDF2_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER6_CUT2000_F64_BA[1], 7);
                benchmarkAndLog(fp, "butter_df1_order6_cut2000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BUTTER_DF1_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER6_CUT2000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "butter_df2_order6_cut2000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BUTTER_DF2_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER6_CUT2000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "butter_tdf2_order6_cut2000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BUTTER_TDF2_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER6_CUT2000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "butter_df1_order6_cut5000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BUTTER_DF1_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER6_CUT5000_F64_BA[1], 7);
                benchmarkAndLog(fp, "butter_df2_order6_cut5000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BUTTER_DF2_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER6_CUT5000_F64_BA[1], 7);
                benchmarkAndLog(fp, "butter_tdf2_order6_cut5000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BUTTER_TDF2_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER6_CUT5000_F64_BA[1], 7);
                benchmarkAndLog(fp, "butter_df1_order6_cut5000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BUTTER_DF1_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER6_CUT5000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "butter_df2_order6_cut5000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BUTTER_DF2_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER6_CUT5000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "butter_tdf2_order6_cut5000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BUTTER_TDF2_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER6_CUT5000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "butter_df1_order8_cut1000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BUTTER_DF1_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER8_CUT1000_F64_BA[1], 9);
                benchmarkAndLog(fp, "butter_df2_order8_cut1000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BUTTER_DF2_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER8_CUT1000_F64_BA[1], 9);
                benchmarkAndLog(fp, "butter_tdf2_order8_cut1000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BUTTER_TDF2_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER8_CUT1000_F64_BA[1], 9);
                benchmarkAndLog(fp, "butter_df1_order8_cut1000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BUTTER_DF1_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER8_CUT1000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "butter_df2_order8_cut1000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BUTTER_DF2_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER8_CUT1000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "butter_tdf2_order8_cut1000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BUTTER_TDF2_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER8_CUT1000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "butter_df1_order8_cut2000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BUTTER_DF1_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER8_CUT2000_F64_BA[1], 9);
                benchmarkAndLog(fp, "butter_df2_order8_cut2000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BUTTER_DF2_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER8_CUT2000_F64_BA[1], 9);
                benchmarkAndLog(fp, "butter_tdf2_order8_cut2000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BUTTER_TDF2_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER8_CUT2000_F64_BA[1], 9);
                benchmarkAndLog(fp, "butter_df1_order8_cut2000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BUTTER_DF1_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER8_CUT2000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "butter_df2_order8_cut2000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BUTTER_DF2_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER8_CUT2000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "butter_tdf2_order8_cut2000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BUTTER_TDF2_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER8_CUT2000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "butter_df1_order8_cut5000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BUTTER_DF1_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER8_CUT5000_F64_BA[1], 9);
                benchmarkAndLog(fp, "butter_df2_order8_cut5000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BUTTER_DF2_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER8_CUT5000_F64_BA[1], 9);
                benchmarkAndLog(fp, "butter_tdf2_order8_cut5000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BUTTER_TDF2_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER8_CUT5000_F64_BA[1], 9);
                benchmarkAndLog(fp, "butter_df1_order8_cut5000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BUTTER_DF1_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER8_CUT5000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "butter_df2_order8_cut5000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BUTTER_DF2_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER8_CUT5000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "butter_tdf2_order8_cut5000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BUTTER_TDF2_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER8_CUT5000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "cheby1_df1_order2_cut1000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY1_DF1_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER2_CUT1000_F64_BA[1], 3);
                benchmarkAndLog(fp, "cheby1_df2_order2_cut1000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY1_DF2_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER2_CUT1000_F64_BA[1], 3);
                benchmarkAndLog(fp, "cheby1_tdf2_order2_cut1000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT1000_F64_BA[1], 3);
                benchmarkAndLog(fp, "cheby1_df1_order2_cut1000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY1_DF1_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER2_CUT1000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "cheby1_df2_order2_cut1000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY1_DF2_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER2_CUT1000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "cheby1_tdf2_order2_cut1000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT1000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "cheby1_df1_order2_cut2000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY1_DF1_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER2_CUT2000_F64_BA[1], 3);
                benchmarkAndLog(fp, "cheby1_df2_order2_cut2000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY1_DF2_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER2_CUT2000_F64_BA[1], 3);
                benchmarkAndLog(fp, "cheby1_tdf2_order2_cut2000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT2000_F64_BA[1], 3);
                benchmarkAndLog(fp, "cheby1_df1_order2_cut2000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY1_DF1_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER2_CUT2000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "cheby1_df2_order2_cut2000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY1_DF2_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER2_CUT2000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "cheby1_tdf2_order2_cut2000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT2000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "cheby1_df1_order2_cut5000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY1_DF1_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER2_CUT5000_F64_BA[1], 3);
                benchmarkAndLog(fp, "cheby1_df2_order2_cut5000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY1_DF2_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER2_CUT5000_F64_BA[1], 3);
                benchmarkAndLog(fp, "cheby1_tdf2_order2_cut5000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT5000_F64_BA[1], 3);
                benchmarkAndLog(fp, "cheby1_df1_order2_cut5000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY1_DF1_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER2_CUT5000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "cheby1_df2_order2_cut5000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY1_DF2_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER2_CUT5000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "cheby1_tdf2_order2_cut5000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT5000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "cheby1_df1_order4_cut1000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY1_DF1_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER4_CUT1000_F64_BA[1], 5);
                benchmarkAndLog(fp, "cheby1_df2_order4_cut1000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY1_DF2_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER4_CUT1000_F64_BA[1], 5);
                benchmarkAndLog(fp, "cheby1_tdf2_order4_cut1000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT1000_F64_BA[1], 5);
                benchmarkAndLog(fp, "cheby1_df1_order4_cut1000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY1_DF1_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER4_CUT1000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "cheby1_df2_order4_cut1000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY1_DF2_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER4_CUT1000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "cheby1_tdf2_order4_cut1000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT1000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "cheby1_df1_order4_cut2000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY1_DF1_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER4_CUT2000_F64_BA[1], 5);
                benchmarkAndLog(fp, "cheby1_df2_order4_cut2000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY1_DF2_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER4_CUT2000_F64_BA[1], 5);
                benchmarkAndLog(fp, "cheby1_tdf2_order4_cut2000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT2000_F64_BA[1], 5);
                benchmarkAndLog(fp, "cheby1_df1_order4_cut2000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY1_DF1_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER4_CUT2000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "cheby1_df2_order4_cut2000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY1_DF2_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER4_CUT2000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "cheby1_tdf2_order4_cut2000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT2000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "cheby1_df1_order4_cut5000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY1_DF1_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER4_CUT5000_F64_BA[1], 5);
                benchmarkAndLog(fp, "cheby1_df2_order4_cut5000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY1_DF2_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER4_CUT5000_F64_BA[1], 5);
                benchmarkAndLog(fp, "cheby1_tdf2_order4_cut5000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT5000_F64_BA[1], 5);
                benchmarkAndLog(fp, "cheby1_df1_order4_cut5000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY1_DF1_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER4_CUT5000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "cheby1_df2_order4_cut5000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY1_DF2_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER4_CUT5000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "cheby1_tdf2_order4_cut5000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT5000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "cheby1_df1_order6_cut1000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY1_DF1_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER6_CUT1000_F64_BA[1], 7);
                benchmarkAndLog(fp, "cheby1_df2_order6_cut1000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY1_DF2_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER6_CUT1000_F64_BA[1], 7);
                benchmarkAndLog(fp, "cheby1_tdf2_order6_cut1000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT1000_F64_BA[1], 7);
                benchmarkAndLog(fp, "cheby1_df1_order6_cut1000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY1_DF1_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER6_CUT1000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "cheby1_df2_order6_cut1000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY1_DF2_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER6_CUT1000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "cheby1_tdf2_order6_cut1000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT1000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "cheby1_df1_order6_cut2000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY1_DF1_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER6_CUT2000_F64_BA[1], 7);
                benchmarkAndLog(fp, "cheby1_df2_order6_cut2000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY1_DF2_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER6_CUT2000_F64_BA[1], 7);
                benchmarkAndLog(fp, "cheby1_tdf2_order6_cut2000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT2000_F64_BA[1], 7);
                benchmarkAndLog(fp, "cheby1_df1_order6_cut2000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY1_DF1_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER6_CUT2000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "cheby1_df2_order6_cut2000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY1_DF2_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER6_CUT2000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "cheby1_tdf2_order6_cut2000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT2000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "cheby1_df1_order6_cut5000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY1_DF1_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER6_CUT5000_F64_BA[1], 7);
                benchmarkAndLog(fp, "cheby1_df2_order6_cut5000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY1_DF2_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER6_CUT5000_F64_BA[1], 7);
                benchmarkAndLog(fp, "cheby1_tdf2_order6_cut5000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT5000_F64_BA[1], 7);
                benchmarkAndLog(fp, "cheby1_df1_order6_cut5000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY1_DF1_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER6_CUT5000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "cheby1_df2_order6_cut5000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY1_DF2_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER6_CUT5000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "cheby1_tdf2_order6_cut5000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT5000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "cheby1_df1_order8_cut1000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY1_DF1_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER8_CUT1000_F64_BA[1], 9);
                benchmarkAndLog(fp, "cheby1_df2_order8_cut1000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY1_DF2_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER8_CUT1000_F64_BA[1], 9);
                benchmarkAndLog(fp, "cheby1_tdf2_order8_cut1000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT1000_F64_BA[1], 9);
                benchmarkAndLog(fp, "cheby1_df1_order8_cut1000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY1_DF1_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER8_CUT1000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "cheby1_df2_order8_cut1000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY1_DF2_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER8_CUT1000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "cheby1_tdf2_order8_cut1000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT1000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "cheby1_df1_order8_cut2000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY1_DF1_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER8_CUT2000_F64_BA[1], 9);
                benchmarkAndLog(fp, "cheby1_df2_order8_cut2000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY1_DF2_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER8_CUT2000_F64_BA[1], 9);
                benchmarkAndLog(fp, "cheby1_tdf2_order8_cut2000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT2000_F64_BA[1], 9);
                benchmarkAndLog(fp, "cheby1_df1_order8_cut2000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY1_DF1_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER8_CUT2000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "cheby1_df2_order8_cut2000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY1_DF2_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER8_CUT2000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "cheby1_tdf2_order8_cut2000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT2000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "cheby1_df1_order8_cut5000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY1_DF1_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER8_CUT5000_F64_BA[1], 9);
                benchmarkAndLog(fp, "cheby1_df2_order8_cut5000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY1_DF2_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER8_CUT5000_F64_BA[1], 9);
                benchmarkAndLog(fp, "cheby1_tdf2_order8_cut5000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT5000_F64_BA[1], 9);
                benchmarkAndLog(fp, "cheby1_df1_order8_cut5000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY1_DF1_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER8_CUT5000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "cheby1_df2_order8_cut5000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY1_DF2_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER8_CUT5000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "cheby1_tdf2_order8_cut5000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT5000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "cheby2_df1_order2_cut1000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY2_DF1_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER2_CUT1000_F64_BA[1], 3);
                benchmarkAndLog(fp, "cheby2_df2_order2_cut1000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY2_DF2_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER2_CUT1000_F64_BA[1], 3);
                benchmarkAndLog(fp, "cheby2_tdf2_order2_cut1000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT1000_F64_BA[1], 3);
                benchmarkAndLog(fp, "cheby2_df1_order2_cut1000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY2_DF1_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER2_CUT1000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "cheby2_df2_order2_cut1000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY2_DF2_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER2_CUT1000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "cheby2_tdf2_order2_cut1000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT1000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "cheby2_df1_order2_cut2000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY2_DF1_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER2_CUT2000_F64_BA[1], 3);
                benchmarkAndLog(fp, "cheby2_df2_order2_cut2000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY2_DF2_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER2_CUT2000_F64_BA[1], 3);
                benchmarkAndLog(fp, "cheby2_tdf2_order2_cut2000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT2000_F64_BA[1], 3);
                benchmarkAndLog(fp, "cheby2_df1_order2_cut2000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY2_DF1_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER2_CUT2000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "cheby2_df2_order2_cut2000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY2_DF2_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER2_CUT2000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "cheby2_tdf2_order2_cut2000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT2000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "cheby2_df1_order2_cut5000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY2_DF1_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER2_CUT5000_F64_BA[1], 3);
                benchmarkAndLog(fp, "cheby2_df2_order2_cut5000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY2_DF2_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER2_CUT5000_F64_BA[1], 3);
                benchmarkAndLog(fp, "cheby2_tdf2_order2_cut5000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT5000_F64_BA[1], 3);
                benchmarkAndLog(fp, "cheby2_df1_order2_cut5000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY2_DF1_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER2_CUT5000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "cheby2_df2_order2_cut5000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY2_DF2_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER2_CUT5000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "cheby2_tdf2_order2_cut5000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT5000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "cheby2_df1_order4_cut1000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY2_DF1_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER4_CUT1000_F64_BA[1], 5);
                benchmarkAndLog(fp, "cheby2_df2_order4_cut1000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY2_DF2_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER4_CUT1000_F64_BA[1], 5);
                benchmarkAndLog(fp, "cheby2_tdf2_order4_cut1000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT1000_F64_BA[1], 5);
                benchmarkAndLog(fp, "cheby2_df1_order4_cut1000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY2_DF1_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER4_CUT1000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "cheby2_df2_order4_cut1000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY2_DF2_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER4_CUT1000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "cheby2_tdf2_order4_cut1000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT1000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "cheby2_df1_order4_cut2000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY2_DF1_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER4_CUT2000_F64_BA[1], 5);
                benchmarkAndLog(fp, "cheby2_df2_order4_cut2000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY2_DF2_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER4_CUT2000_F64_BA[1], 5);
                benchmarkAndLog(fp, "cheby2_tdf2_order4_cut2000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT2000_F64_BA[1], 5);
                benchmarkAndLog(fp, "cheby2_df1_order4_cut2000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY2_DF1_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER4_CUT2000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "cheby2_df2_order4_cut2000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY2_DF2_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER4_CUT2000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "cheby2_tdf2_order4_cut2000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT2000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "cheby2_df1_order4_cut5000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY2_DF1_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER4_CUT5000_F64_BA[1], 5);
                benchmarkAndLog(fp, "cheby2_df2_order4_cut5000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY2_DF2_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER4_CUT5000_F64_BA[1], 5);
                benchmarkAndLog(fp, "cheby2_tdf2_order4_cut5000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT5000_F64_BA[1], 5);
                benchmarkAndLog(fp, "cheby2_df1_order4_cut5000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY2_DF1_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER4_CUT5000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "cheby2_df2_order4_cut5000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY2_DF2_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER4_CUT5000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "cheby2_tdf2_order4_cut5000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT5000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "cheby2_df1_order6_cut1000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY2_DF1_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER6_CUT1000_F64_BA[1], 7);
                benchmarkAndLog(fp, "cheby2_df2_order6_cut1000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY2_DF2_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER6_CUT1000_F64_BA[1], 7);
                benchmarkAndLog(fp, "cheby2_tdf2_order6_cut1000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT1000_F64_BA[1], 7);
                benchmarkAndLog(fp, "cheby2_df1_order6_cut1000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY2_DF1_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER6_CUT1000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "cheby2_df2_order6_cut1000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY2_DF2_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER6_CUT1000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "cheby2_tdf2_order6_cut1000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT1000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "cheby2_df1_order6_cut2000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY2_DF1_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER6_CUT2000_F64_BA[1], 7);
                benchmarkAndLog(fp, "cheby2_df2_order6_cut2000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY2_DF2_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER6_CUT2000_F64_BA[1], 7);
                benchmarkAndLog(fp, "cheby2_tdf2_order6_cut2000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT2000_F64_BA[1], 7);
                benchmarkAndLog(fp, "cheby2_df1_order6_cut2000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY2_DF1_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER6_CUT2000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "cheby2_df2_order6_cut2000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY2_DF2_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER6_CUT2000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "cheby2_tdf2_order6_cut2000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT2000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "cheby2_df1_order6_cut5000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY2_DF1_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER6_CUT5000_F64_BA[1], 7);
                benchmarkAndLog(fp, "cheby2_df2_order6_cut5000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY2_DF2_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER6_CUT5000_F64_BA[1], 7);
                benchmarkAndLog(fp, "cheby2_tdf2_order6_cut5000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT5000_F64_BA[1], 7);
                benchmarkAndLog(fp, "cheby2_df1_order6_cut5000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY2_DF1_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER6_CUT5000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "cheby2_df2_order6_cut5000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY2_DF2_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER6_CUT5000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "cheby2_tdf2_order6_cut5000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT5000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "cheby2_df1_order8_cut1000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY2_DF1_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER8_CUT1000_F64_BA[1], 9);
                benchmarkAndLog(fp, "cheby2_df2_order8_cut1000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY2_DF2_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER8_CUT1000_F64_BA[1], 9);
                benchmarkAndLog(fp, "cheby2_tdf2_order8_cut1000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT1000_F64_BA[1], 9);
                benchmarkAndLog(fp, "cheby2_df1_order8_cut1000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY2_DF1_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER8_CUT1000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "cheby2_df2_order8_cut1000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY2_DF2_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER8_CUT1000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "cheby2_tdf2_order8_cut1000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT1000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "cheby2_df1_order8_cut2000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY2_DF1_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER8_CUT2000_F64_BA[1], 9);
                benchmarkAndLog(fp, "cheby2_df2_order8_cut2000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY2_DF2_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER8_CUT2000_F64_BA[1], 9);
                benchmarkAndLog(fp, "cheby2_tdf2_order8_cut2000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT2000_F64_BA[1], 9);
                benchmarkAndLog(fp, "cheby2_df1_order8_cut2000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY2_DF1_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER8_CUT2000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "cheby2_df2_order8_cut2000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY2_DF2_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER8_CUT2000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "cheby2_tdf2_order8_cut2000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT2000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "cheby2_df1_order8_cut5000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.CHEBY2_DF1_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER8_CUT5000_F64_BA[1], 9);
                benchmarkAndLog(fp, "cheby2_df2_order8_cut5000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.CHEBY2_DF2_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER8_CUT5000_F64_BA[1], 9);
                benchmarkAndLog(fp, "cheby2_tdf2_order8_cut5000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT5000_F64_BA[1], 9);
                benchmarkAndLog(fp, "cheby2_df1_order8_cut5000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.CHEBY2_DF1_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER8_CUT5000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "cheby2_df2_order8_cut5000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.CHEBY2_DF2_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER8_CUT5000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "cheby2_tdf2_order8_cut5000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT5000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "ellip_df1_order2_cut1000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.ELLIP_DF1_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER2_CUT1000_F64_BA[1], 3);
                benchmarkAndLog(fp, "ellip_df2_order2_cut1000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.ELLIP_DF2_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER2_CUT1000_F64_BA[1], 3);
                benchmarkAndLog(fp, "ellip_tdf2_order2_cut1000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.ELLIP_TDF2_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER2_CUT1000_F64_BA[1], 3);
                benchmarkAndLog(fp, "ellip_df1_order2_cut1000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.ELLIP_DF1_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER2_CUT1000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "ellip_df2_order2_cut1000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.ELLIP_DF2_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER2_CUT1000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "ellip_tdf2_order2_cut1000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.ELLIP_TDF2_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER2_CUT1000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "ellip_df1_order2_cut2000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.ELLIP_DF1_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER2_CUT2000_F64_BA[1], 3);
                benchmarkAndLog(fp, "ellip_df2_order2_cut2000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.ELLIP_DF2_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER2_CUT2000_F64_BA[1], 3);
                benchmarkAndLog(fp, "ellip_tdf2_order2_cut2000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.ELLIP_TDF2_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER2_CUT2000_F64_BA[1], 3);
                benchmarkAndLog(fp, "ellip_df1_order2_cut2000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.ELLIP_DF1_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER2_CUT2000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "ellip_df2_order2_cut2000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.ELLIP_DF2_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER2_CUT2000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "ellip_tdf2_order2_cut2000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.ELLIP_TDF2_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER2_CUT2000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "ellip_df1_order2_cut5000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.ELLIP_DF1_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER2_CUT5000_F64_BA[1], 3);
                benchmarkAndLog(fp, "ellip_df2_order2_cut5000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.ELLIP_DF2_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER2_CUT5000_F64_BA[1], 3);
                benchmarkAndLog(fp, "ellip_tdf2_order2_cut5000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.ELLIP_TDF2_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER2_CUT5000_F64_BA[1], 3);
                benchmarkAndLog(fp, "ellip_df1_order2_cut5000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.ELLIP_DF1_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER2_CUT5000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "ellip_df2_order2_cut5000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.ELLIP_DF2_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER2_CUT5000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "ellip_tdf2_order2_cut5000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.ELLIP_TDF2_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER2_CUT5000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "ellip_df1_order4_cut1000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.ELLIP_DF1_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER4_CUT1000_F64_BA[1], 5);
                benchmarkAndLog(fp, "ellip_df2_order4_cut1000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.ELLIP_DF2_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER4_CUT1000_F64_BA[1], 5);
                benchmarkAndLog(fp, "ellip_tdf2_order4_cut1000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.ELLIP_TDF2_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER4_CUT1000_F64_BA[1], 5);
                benchmarkAndLog(fp, "ellip_df1_order4_cut1000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.ELLIP_DF1_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER4_CUT1000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "ellip_df2_order4_cut1000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.ELLIP_DF2_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER4_CUT1000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "ellip_tdf2_order4_cut1000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.ELLIP_TDF2_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER4_CUT1000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "ellip_df1_order4_cut2000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.ELLIP_DF1_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER4_CUT2000_F64_BA[1], 5);
                benchmarkAndLog(fp, "ellip_df2_order4_cut2000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.ELLIP_DF2_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER4_CUT2000_F64_BA[1], 5);
                benchmarkAndLog(fp, "ellip_tdf2_order4_cut2000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.ELLIP_TDF2_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER4_CUT2000_F64_BA[1], 5);
                benchmarkAndLog(fp, "ellip_df1_order4_cut2000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.ELLIP_DF1_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER4_CUT2000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "ellip_df2_order4_cut2000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.ELLIP_DF2_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER4_CUT2000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "ellip_tdf2_order4_cut2000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.ELLIP_TDF2_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER4_CUT2000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "ellip_df1_order4_cut5000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.ELLIP_DF1_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER4_CUT5000_F64_BA[1], 5);
                benchmarkAndLog(fp, "ellip_df2_order4_cut5000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.ELLIP_DF2_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER4_CUT5000_F64_BA[1], 5);
                benchmarkAndLog(fp, "ellip_tdf2_order4_cut5000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.ELLIP_TDF2_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER4_CUT5000_F64_BA[1], 5);
                benchmarkAndLog(fp, "ellip_df1_order4_cut5000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.ELLIP_DF1_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER4_CUT5000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "ellip_df2_order4_cut5000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.ELLIP_DF2_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER4_CUT5000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "ellip_tdf2_order4_cut5000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.ELLIP_TDF2_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER4_CUT5000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "ellip_df1_order6_cut1000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.ELLIP_DF1_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER6_CUT1000_F64_BA[1], 7);
                benchmarkAndLog(fp, "ellip_df2_order6_cut1000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.ELLIP_DF2_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER6_CUT1000_F64_BA[1], 7);
                benchmarkAndLog(fp, "ellip_tdf2_order6_cut1000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.ELLIP_TDF2_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER6_CUT1000_F64_BA[1], 7);
                benchmarkAndLog(fp, "ellip_df1_order6_cut1000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.ELLIP_DF1_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER6_CUT1000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "ellip_df2_order6_cut1000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.ELLIP_DF2_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER6_CUT1000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "ellip_tdf2_order6_cut1000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.ELLIP_TDF2_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER6_CUT1000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "ellip_df1_order6_cut2000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.ELLIP_DF1_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER6_CUT2000_F64_BA[1], 7);
                benchmarkAndLog(fp, "ellip_df2_order6_cut2000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.ELLIP_DF2_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER6_CUT2000_F64_BA[1], 7);
                benchmarkAndLog(fp, "ellip_tdf2_order6_cut2000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.ELLIP_TDF2_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER6_CUT2000_F64_BA[1], 7);
                benchmarkAndLog(fp, "ellip_df1_order6_cut2000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.ELLIP_DF1_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER6_CUT2000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "ellip_df2_order6_cut2000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.ELLIP_DF2_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER6_CUT2000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "ellip_tdf2_order6_cut2000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.ELLIP_TDF2_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER6_CUT2000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "ellip_df1_order6_cut5000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.ELLIP_DF1_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER6_CUT5000_F64_BA[1], 7);
                benchmarkAndLog(fp, "ellip_df2_order6_cut5000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.ELLIP_DF2_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER6_CUT5000_F64_BA[1], 7);
                benchmarkAndLog(fp, "ellip_tdf2_order6_cut5000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.ELLIP_TDF2_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER6_CUT5000_F64_BA[1], 7);
                benchmarkAndLog(fp, "ellip_df1_order6_cut5000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.ELLIP_DF1_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER6_CUT5000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "ellip_df2_order6_cut5000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.ELLIP_DF2_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER6_CUT5000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "ellip_tdf2_order6_cut5000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.ELLIP_TDF2_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER6_CUT5000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "ellip_df1_order8_cut1000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.ELLIP_DF1_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER8_CUT1000_F64_BA[1], 9);
                benchmarkAndLog(fp, "ellip_df2_order8_cut1000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.ELLIP_DF2_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER8_CUT1000_F64_BA[1], 9);
                benchmarkAndLog(fp, "ellip_tdf2_order8_cut1000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.ELLIP_TDF2_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER8_CUT1000_F64_BA[1], 9);
                benchmarkAndLog(fp, "ellip_df1_order8_cut1000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.ELLIP_DF1_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER8_CUT1000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "ellip_df2_order8_cut1000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.ELLIP_DF2_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER8_CUT1000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "ellip_tdf2_order8_cut1000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.ELLIP_TDF2_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER8_CUT1000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "ellip_df1_order8_cut2000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.ELLIP_DF1_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER8_CUT2000_F64_BA[1], 9);
                benchmarkAndLog(fp, "ellip_df2_order8_cut2000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.ELLIP_DF2_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER8_CUT2000_F64_BA[1], 9);
                benchmarkAndLog(fp, "ellip_tdf2_order8_cut2000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.ELLIP_TDF2_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER8_CUT2000_F64_BA[1], 9);
                benchmarkAndLog(fp, "ellip_df1_order8_cut2000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.ELLIP_DF1_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER8_CUT2000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "ellip_df2_order8_cut2000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.ELLIP_DF2_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER8_CUT2000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "ellip_tdf2_order8_cut2000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.ELLIP_TDF2_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER8_CUT2000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "ellip_df1_order8_cut5000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.ELLIP_DF1_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER8_CUT5000_F64_BA[1], 9);
                benchmarkAndLog(fp, "ellip_df2_order8_cut5000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.ELLIP_DF2_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER8_CUT5000_F64_BA[1], 9);
                benchmarkAndLog(fp, "ellip_tdf2_order8_cut5000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.ELLIP_TDF2_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER8_CUT5000_F64_BA[1], 9);
                benchmarkAndLog(fp, "ellip_df1_order8_cut5000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.ELLIP_DF1_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER8_CUT5000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "ellip_df2_order8_cut5000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.ELLIP_DF2_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER8_CUT5000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "ellip_tdf2_order8_cut5000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.ELLIP_TDF2_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER8_CUT5000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "bessel_df1_order2_cut1000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BESSEL_DF1_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER2_CUT1000_F64_BA[1], 3);
                benchmarkAndLog(fp, "bessel_df2_order2_cut1000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BESSEL_DF2_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER2_CUT1000_F64_BA[1], 3);
                benchmarkAndLog(fp, "bessel_tdf2_order2_cut1000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BESSEL_TDF2_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER2_CUT1000_F64_BA[1], 3);
                benchmarkAndLog(fp, "bessel_df1_order2_cut1000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BESSEL_DF1_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER2_CUT1000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "bessel_df2_order2_cut1000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BESSEL_DF2_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER2_CUT1000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "bessel_tdf2_order2_cut1000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BESSEL_TDF2_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER2_CUT1000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "bessel_df1_order2_cut2000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BESSEL_DF1_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER2_CUT2000_F64_BA[1], 3);
                benchmarkAndLog(fp, "bessel_df2_order2_cut2000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BESSEL_DF2_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER2_CUT2000_F64_BA[1], 3);
                benchmarkAndLog(fp, "bessel_tdf2_order2_cut2000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BESSEL_TDF2_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER2_CUT2000_F64_BA[1], 3);
                benchmarkAndLog(fp, "bessel_df1_order2_cut2000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BESSEL_DF1_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER2_CUT2000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "bessel_df2_order2_cut2000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BESSEL_DF2_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER2_CUT2000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "bessel_tdf2_order2_cut2000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BESSEL_TDF2_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER2_CUT2000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "bessel_df1_order2_cut5000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BESSEL_DF1_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER2_CUT5000_F64_BA[1], 3);
                benchmarkAndLog(fp, "bessel_df2_order2_cut5000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BESSEL_DF2_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER2_CUT5000_F64_BA[1], 3);
                benchmarkAndLog(fp, "bessel_tdf2_order2_cut5000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BESSEL_TDF2_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER2_CUT5000_F64_BA[1], 3);
                benchmarkAndLog(fp, "bessel_df1_order2_cut5000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BESSEL_DF1_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER2_CUT5000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "bessel_df2_order2_cut5000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BESSEL_DF2_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER2_CUT5000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "bessel_tdf2_order2_cut5000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BESSEL_TDF2_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER2_CUT5000_F32_BA[1], null, null, 3);
                benchmarkAndLog(fp, "bessel_df1_order4_cut1000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BESSEL_DF1_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER4_CUT1000_F64_BA[1], 5);
                benchmarkAndLog(fp, "bessel_df2_order4_cut1000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BESSEL_DF2_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER4_CUT1000_F64_BA[1], 5);
                benchmarkAndLog(fp, "bessel_tdf2_order4_cut1000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BESSEL_TDF2_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER4_CUT1000_F64_BA[1], 5);
                benchmarkAndLog(fp, "bessel_df1_order4_cut1000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BESSEL_DF1_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER4_CUT1000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "bessel_df2_order4_cut1000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BESSEL_DF2_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER4_CUT1000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "bessel_tdf2_order4_cut1000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BESSEL_TDF2_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER4_CUT1000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "bessel_df1_order4_cut2000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BESSEL_DF1_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER4_CUT2000_F64_BA[1], 5);
                benchmarkAndLog(fp, "bessel_df2_order4_cut2000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BESSEL_DF2_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER4_CUT2000_F64_BA[1], 5);
                benchmarkAndLog(fp, "bessel_tdf2_order4_cut2000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BESSEL_TDF2_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER4_CUT2000_F64_BA[1], 5);
                benchmarkAndLog(fp, "bessel_df1_order4_cut2000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BESSEL_DF1_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER4_CUT2000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "bessel_df2_order4_cut2000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BESSEL_DF2_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER4_CUT2000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "bessel_tdf2_order4_cut2000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BESSEL_TDF2_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER4_CUT2000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "bessel_df1_order4_cut5000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BESSEL_DF1_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER4_CUT5000_F64_BA[1], 5);
                benchmarkAndLog(fp, "bessel_df2_order4_cut5000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BESSEL_DF2_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER4_CUT5000_F64_BA[1], 5);
                benchmarkAndLog(fp, "bessel_tdf2_order4_cut5000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BESSEL_TDF2_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER4_CUT5000_F64_BA[1], 5);
                benchmarkAndLog(fp, "bessel_df1_order4_cut5000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BESSEL_DF1_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER4_CUT5000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "bessel_df2_order4_cut5000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BESSEL_DF2_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER4_CUT5000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "bessel_tdf2_order4_cut5000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BESSEL_TDF2_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER4_CUT5000_F32_BA[1], null, null, 5);
                benchmarkAndLog(fp, "bessel_df1_order6_cut1000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BESSEL_DF1_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER6_CUT1000_F64_BA[1], 7);
                benchmarkAndLog(fp, "bessel_df2_order6_cut1000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BESSEL_DF2_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER6_CUT1000_F64_BA[1], 7);
                benchmarkAndLog(fp, "bessel_tdf2_order6_cut1000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BESSEL_TDF2_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER6_CUT1000_F64_BA[1], 7);
                benchmarkAndLog(fp, "bessel_df1_order6_cut1000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BESSEL_DF1_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER6_CUT1000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "bessel_df2_order6_cut1000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BESSEL_DF2_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER6_CUT1000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "bessel_tdf2_order6_cut1000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BESSEL_TDF2_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER6_CUT1000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "bessel_df1_order6_cut2000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BESSEL_DF1_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER6_CUT2000_F64_BA[1], 7);
                benchmarkAndLog(fp, "bessel_df2_order6_cut2000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BESSEL_DF2_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER6_CUT2000_F64_BA[1], 7);
                benchmarkAndLog(fp, "bessel_tdf2_order6_cut2000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BESSEL_TDF2_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER6_CUT2000_F64_BA[1], 7);
                benchmarkAndLog(fp, "bessel_df1_order6_cut2000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BESSEL_DF1_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER6_CUT2000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "bessel_df2_order6_cut2000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BESSEL_DF2_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER6_CUT2000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "bessel_tdf2_order6_cut2000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BESSEL_TDF2_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER6_CUT2000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "bessel_df1_order6_cut5000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BESSEL_DF1_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER6_CUT5000_F64_BA[1], 7);
                benchmarkAndLog(fp, "bessel_df2_order6_cut5000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BESSEL_DF2_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER6_CUT5000_F64_BA[1], 7);
                benchmarkAndLog(fp, "bessel_tdf2_order6_cut5000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BESSEL_TDF2_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER6_CUT5000_F64_BA[1], 7);
                benchmarkAndLog(fp, "bessel_df1_order6_cut5000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BESSEL_DF1_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER6_CUT5000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "bessel_df2_order6_cut5000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BESSEL_DF2_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER6_CUT5000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "bessel_tdf2_order6_cut5000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BESSEL_TDF2_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER6_CUT5000_F32_BA[1], null, null, 7);
                benchmarkAndLog(fp, "bessel_df1_order8_cut1000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BESSEL_DF1_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER8_CUT1000_F64_BA[1], 9);
                benchmarkAndLog(fp, "bessel_df2_order8_cut1000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BESSEL_DF2_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER8_CUT1000_F64_BA[1], 9);
                benchmarkAndLog(fp, "bessel_tdf2_order8_cut1000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BESSEL_TDF2_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER8_CUT1000_F64_BA[1], 9);
                benchmarkAndLog(fp, "bessel_df1_order8_cut1000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BESSEL_DF1_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER8_CUT1000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "bessel_df2_order8_cut1000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BESSEL_DF2_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER8_CUT1000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "bessel_tdf2_order8_cut1000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BESSEL_TDF2_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER8_CUT1000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "bessel_df1_order8_cut2000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BESSEL_DF1_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER8_CUT2000_F64_BA[1], 9);
                benchmarkAndLog(fp, "bessel_df2_order8_cut2000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BESSEL_DF2_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER8_CUT2000_F64_BA[1], 9);
                benchmarkAndLog(fp, "bessel_tdf2_order8_cut2000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BESSEL_TDF2_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER8_CUT2000_F64_BA[1], 9);
                benchmarkAndLog(fp, "bessel_df1_order8_cut2000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BESSEL_DF1_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER8_CUT2000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "bessel_df2_order8_cut2000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BESSEL_DF2_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER8_CUT2000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "bessel_tdf2_order8_cut2000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BESSEL_TDF2_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER8_CUT2000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "bessel_df1_order8_cut5000_f64_ba", "double", "DF1", null, DF1::filterDouble, null, null, FilterCoeffs.BESSEL_DF1_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER8_CUT5000_F64_BA[1], 9);
                benchmarkAndLog(fp, "bessel_df2_order8_cut5000_f64_ba", "double", "DF2", null, DF2::filterDouble, null, null, FilterCoeffs.BESSEL_DF2_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER8_CUT5000_F64_BA[1], 9);
                benchmarkAndLog(fp, "bessel_tdf2_order8_cut5000_f64_ba", "double", "TDF2", null, TDF2::filterDouble, null, null, FilterCoeffs.BESSEL_TDF2_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER8_CUT5000_F64_BA[1], 9);
                benchmarkAndLog(fp, "bessel_df1_order8_cut5000_f32_ba", "float", "DF1", DF1::filterFloat, null, FilterCoeffs.BESSEL_DF1_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER8_CUT5000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "bessel_df2_order8_cut5000_f32_ba", "float", "DF2", DF2::filterFloat, null, FilterCoeffs.BESSEL_DF2_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER8_CUT5000_F32_BA[1], null, null, 9);
                benchmarkAndLog(fp, "bessel_tdf2_order8_cut5000_f32_ba", "float", "TDF2", TDF2::filterFloat, null, FilterCoeffs.BESSEL_TDF2_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER8_CUT5000_F32_BA[1], null, null, 9);

                // --- CASCADE ---
                benchmarkCascadeAndLog(fp, "butter_cascade_order2_cut1000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BUTTER_CASCADE_ORDER2_CUT1000_F64_SOS, 1);
                benchmarkCascadeAndLog(fp, "butter_cascade_order2_cut1000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BUTTER_CASCADE_ORDER2_CUT1000_F32_SOS, null, 1);
                benchmarkCascadeAndLog(fp, "butter_cascade_order2_cut2000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BUTTER_CASCADE_ORDER2_CUT2000_F64_SOS, 1);
                benchmarkCascadeAndLog(fp, "butter_cascade_order2_cut2000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BUTTER_CASCADE_ORDER2_CUT2000_F32_SOS, null, 1);
                benchmarkCascadeAndLog(fp, "butter_cascade_order2_cut5000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BUTTER_CASCADE_ORDER2_CUT5000_F64_SOS, 1);
                benchmarkCascadeAndLog(fp, "butter_cascade_order2_cut5000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BUTTER_CASCADE_ORDER2_CUT5000_F32_SOS, null, 1);
                benchmarkCascadeAndLog(fp, "butter_cascade_order4_cut1000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BUTTER_CASCADE_ORDER4_CUT1000_F64_SOS, 2);
                benchmarkCascadeAndLog(fp, "butter_cascade_order4_cut1000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BUTTER_CASCADE_ORDER4_CUT1000_F32_SOS, null, 2);
                benchmarkCascadeAndLog(fp, "butter_cascade_order4_cut2000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BUTTER_CASCADE_ORDER4_CUT2000_F64_SOS, 2);
                benchmarkCascadeAndLog(fp, "butter_cascade_order4_cut2000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BUTTER_CASCADE_ORDER4_CUT2000_F32_SOS, null, 2);
                benchmarkCascadeAndLog(fp, "butter_cascade_order4_cut5000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BUTTER_CASCADE_ORDER4_CUT5000_F64_SOS, 2);
                benchmarkCascadeAndLog(fp, "butter_cascade_order4_cut5000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BUTTER_CASCADE_ORDER4_CUT5000_F32_SOS, null, 2);
                benchmarkCascadeAndLog(fp, "butter_cascade_order6_cut1000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BUTTER_CASCADE_ORDER6_CUT1000_F64_SOS, 3);
                benchmarkCascadeAndLog(fp, "butter_cascade_order6_cut1000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BUTTER_CASCADE_ORDER6_CUT1000_F32_SOS, null, 3);
                benchmarkCascadeAndLog(fp, "butter_cascade_order6_cut2000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BUTTER_CASCADE_ORDER6_CUT2000_F64_SOS, 3);
                benchmarkCascadeAndLog(fp, "butter_cascade_order6_cut2000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BUTTER_CASCADE_ORDER6_CUT2000_F32_SOS, null, 3);
                benchmarkCascadeAndLog(fp, "butter_cascade_order6_cut5000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BUTTER_CASCADE_ORDER6_CUT5000_F64_SOS, 3);
                benchmarkCascadeAndLog(fp, "butter_cascade_order6_cut5000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BUTTER_CASCADE_ORDER6_CUT5000_F32_SOS, null, 3);
                benchmarkCascadeAndLog(fp, "butter_cascade_order8_cut1000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BUTTER_CASCADE_ORDER8_CUT1000_F64_SOS, 4);
                benchmarkCascadeAndLog(fp, "butter_cascade_order8_cut1000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BUTTER_CASCADE_ORDER8_CUT1000_F32_SOS, null, 4);
                benchmarkCascadeAndLog(fp, "butter_cascade_order8_cut2000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BUTTER_CASCADE_ORDER8_CUT2000_F64_SOS, 4);
                benchmarkCascadeAndLog(fp, "butter_cascade_order8_cut2000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BUTTER_CASCADE_ORDER8_CUT2000_F32_SOS, null, 4);
                benchmarkCascadeAndLog(fp, "butter_cascade_order8_cut5000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BUTTER_CASCADE_ORDER8_CUT5000_F64_SOS, 4);
                benchmarkCascadeAndLog(fp, "butter_cascade_order8_cut5000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BUTTER_CASCADE_ORDER8_CUT5000_F32_SOS, null, 4);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order2_cut1000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY1_CASCADE_ORDER2_CUT1000_F64_SOS, 1);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order2_cut1000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY1_CASCADE_ORDER2_CUT1000_F32_SOS, null, 1);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order2_cut2000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY1_CASCADE_ORDER2_CUT2000_F64_SOS, 1);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order2_cut2000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY1_CASCADE_ORDER2_CUT2000_F32_SOS, null, 1);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order2_cut5000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY1_CASCADE_ORDER2_CUT5000_F64_SOS, 1);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order2_cut5000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY1_CASCADE_ORDER2_CUT5000_F32_SOS, null, 1);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order4_cut1000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY1_CASCADE_ORDER4_CUT1000_F64_SOS, 2);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order4_cut1000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY1_CASCADE_ORDER4_CUT1000_F32_SOS, null, 2);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order4_cut2000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY1_CASCADE_ORDER4_CUT2000_F64_SOS, 2);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order4_cut2000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY1_CASCADE_ORDER4_CUT2000_F32_SOS, null, 2);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order4_cut5000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY1_CASCADE_ORDER4_CUT5000_F64_SOS, 2);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order4_cut5000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY1_CASCADE_ORDER4_CUT5000_F32_SOS, null, 2);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order6_cut1000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY1_CASCADE_ORDER6_CUT1000_F64_SOS, 3);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order6_cut1000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY1_CASCADE_ORDER6_CUT1000_F32_SOS, null, 3);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order6_cut2000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY1_CASCADE_ORDER6_CUT2000_F64_SOS, 3);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order6_cut2000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY1_CASCADE_ORDER6_CUT2000_F32_SOS, null, 3);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order6_cut5000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY1_CASCADE_ORDER6_CUT5000_F64_SOS, 3);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order6_cut5000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY1_CASCADE_ORDER6_CUT5000_F32_SOS, null, 3);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order8_cut1000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY1_CASCADE_ORDER8_CUT1000_F64_SOS, 4);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order8_cut1000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY1_CASCADE_ORDER8_CUT1000_F32_SOS, null, 4);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order8_cut2000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY1_CASCADE_ORDER8_CUT2000_F64_SOS, 4);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order8_cut2000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY1_CASCADE_ORDER8_CUT2000_F32_SOS, null, 4);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order8_cut5000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY1_CASCADE_ORDER8_CUT5000_F64_SOS, 4);
                benchmarkCascadeAndLog(fp, "cheby1_cascade_order8_cut5000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY1_CASCADE_ORDER8_CUT5000_F32_SOS, null, 4);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order2_cut1000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY2_CASCADE_ORDER2_CUT1000_F64_SOS, 1);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order2_cut1000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY2_CASCADE_ORDER2_CUT1000_F32_SOS, null, 1);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order2_cut2000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY2_CASCADE_ORDER2_CUT2000_F64_SOS, 1);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order2_cut2000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY2_CASCADE_ORDER2_CUT2000_F32_SOS, null, 1);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order2_cut5000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY2_CASCADE_ORDER2_CUT5000_F64_SOS, 1);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order2_cut5000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY2_CASCADE_ORDER2_CUT5000_F32_SOS, null, 1);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order4_cut1000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY2_CASCADE_ORDER4_CUT1000_F64_SOS, 2);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order4_cut1000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY2_CASCADE_ORDER4_CUT1000_F32_SOS, null, 2);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order4_cut2000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY2_CASCADE_ORDER4_CUT2000_F64_SOS, 2);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order4_cut2000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY2_CASCADE_ORDER4_CUT2000_F32_SOS, null, 2);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order4_cut5000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY2_CASCADE_ORDER4_CUT5000_F64_SOS, 2);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order4_cut5000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY2_CASCADE_ORDER4_CUT5000_F32_SOS, null, 2);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order6_cut1000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY2_CASCADE_ORDER6_CUT1000_F64_SOS, 3);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order6_cut1000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY2_CASCADE_ORDER6_CUT1000_F32_SOS, null, 3);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order6_cut2000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY2_CASCADE_ORDER6_CUT2000_F64_SOS, 3);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order6_cut2000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY2_CASCADE_ORDER6_CUT2000_F32_SOS, null, 3);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order6_cut5000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY2_CASCADE_ORDER6_CUT5000_F64_SOS, 3);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order6_cut5000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY2_CASCADE_ORDER6_CUT5000_F32_SOS, null, 3);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order8_cut1000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY2_CASCADE_ORDER8_CUT1000_F64_SOS, 4);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order8_cut1000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY2_CASCADE_ORDER8_CUT1000_F32_SOS, null, 4);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order8_cut2000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY2_CASCADE_ORDER8_CUT2000_F64_SOS, 4);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order8_cut2000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY2_CASCADE_ORDER8_CUT2000_F32_SOS, null, 4);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order8_cut5000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.CHEBY2_CASCADE_ORDER8_CUT5000_F64_SOS, 4);
                benchmarkCascadeAndLog(fp, "cheby2_cascade_order8_cut5000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.CHEBY2_CASCADE_ORDER8_CUT5000_F32_SOS, null, 4);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order2_cut1000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.ELLIP_CASCADE_ORDER2_CUT1000_F64_SOS, 1);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order2_cut1000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.ELLIP_CASCADE_ORDER2_CUT1000_F32_SOS, null, 1);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order2_cut2000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.ELLIP_CASCADE_ORDER2_CUT2000_F64_SOS, 1);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order2_cut2000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.ELLIP_CASCADE_ORDER2_CUT2000_F32_SOS, null, 1);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order2_cut5000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.ELLIP_CASCADE_ORDER2_CUT5000_F64_SOS, 1);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order2_cut5000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.ELLIP_CASCADE_ORDER2_CUT5000_F32_SOS, null, 1);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order4_cut1000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.ELLIP_CASCADE_ORDER4_CUT1000_F64_SOS, 2);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order4_cut1000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.ELLIP_CASCADE_ORDER4_CUT1000_F32_SOS, null, 2);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order4_cut2000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.ELLIP_CASCADE_ORDER4_CUT2000_F64_SOS, 2);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order4_cut2000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.ELLIP_CASCADE_ORDER4_CUT2000_F32_SOS, null, 2);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order4_cut5000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.ELLIP_CASCADE_ORDER4_CUT5000_F64_SOS, 2);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order4_cut5000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.ELLIP_CASCADE_ORDER4_CUT5000_F32_SOS, null, 2);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order6_cut1000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.ELLIP_CASCADE_ORDER6_CUT1000_F64_SOS, 3);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order6_cut1000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.ELLIP_CASCADE_ORDER6_CUT1000_F32_SOS, null, 3);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order6_cut2000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.ELLIP_CASCADE_ORDER6_CUT2000_F64_SOS, 3);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order6_cut2000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.ELLIP_CASCADE_ORDER6_CUT2000_F32_SOS, null, 3);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order6_cut5000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.ELLIP_CASCADE_ORDER6_CUT5000_F64_SOS, 3);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order6_cut5000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.ELLIP_CASCADE_ORDER6_CUT5000_F32_SOS, null, 3);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order8_cut1000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.ELLIP_CASCADE_ORDER8_CUT1000_F64_SOS, 4);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order8_cut1000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.ELLIP_CASCADE_ORDER8_CUT1000_F32_SOS, null, 4);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order8_cut2000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.ELLIP_CASCADE_ORDER8_CUT2000_F64_SOS, 4);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order8_cut2000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.ELLIP_CASCADE_ORDER8_CUT2000_F32_SOS, null, 4);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order8_cut5000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.ELLIP_CASCADE_ORDER8_CUT5000_F64_SOS, 4);
                benchmarkCascadeAndLog(fp, "ellip_cascade_order8_cut5000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.ELLIP_CASCADE_ORDER8_CUT5000_F32_SOS, null, 4);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order2_cut1000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BESSEL_CASCADE_ORDER2_CUT1000_F64_SOS, 1);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order2_cut1000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BESSEL_CASCADE_ORDER2_CUT1000_F32_SOS, null, 1);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order2_cut2000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BESSEL_CASCADE_ORDER2_CUT2000_F64_SOS, 1);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order2_cut2000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BESSEL_CASCADE_ORDER2_CUT2000_F32_SOS, null, 1);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order2_cut5000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BESSEL_CASCADE_ORDER2_CUT5000_F64_SOS, 1);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order2_cut5000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BESSEL_CASCADE_ORDER2_CUT5000_F32_SOS, null, 1);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order4_cut1000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BESSEL_CASCADE_ORDER4_CUT1000_F64_SOS, 2);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order4_cut1000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BESSEL_CASCADE_ORDER4_CUT1000_F32_SOS, null, 2);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order4_cut2000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BESSEL_CASCADE_ORDER4_CUT2000_F64_SOS, 2);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order4_cut2000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BESSEL_CASCADE_ORDER4_CUT2000_F32_SOS, null, 2);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order4_cut5000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BESSEL_CASCADE_ORDER4_CUT5000_F64_SOS, 2);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order4_cut5000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BESSEL_CASCADE_ORDER4_CUT5000_F32_SOS, null, 2);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order6_cut1000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BESSEL_CASCADE_ORDER6_CUT1000_F64_SOS, 3);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order6_cut1000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BESSEL_CASCADE_ORDER6_CUT1000_F32_SOS, null, 3);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order6_cut2000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BESSEL_CASCADE_ORDER6_CUT2000_F64_SOS, 3);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order6_cut2000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BESSEL_CASCADE_ORDER6_CUT2000_F32_SOS, null, 3);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order6_cut5000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BESSEL_CASCADE_ORDER6_CUT5000_F64_SOS, 3);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order6_cut5000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BESSEL_CASCADE_ORDER6_CUT5000_F32_SOS, null, 3);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order8_cut1000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BESSEL_CASCADE_ORDER8_CUT1000_F64_SOS, 4);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order8_cut1000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BESSEL_CASCADE_ORDER8_CUT1000_F32_SOS, null, 4);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order8_cut2000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BESSEL_CASCADE_ORDER8_CUT2000_F64_SOS, 4);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order8_cut2000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BESSEL_CASCADE_ORDER8_CUT2000_F32_SOS, null, 4);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order8_cut5000_f64_sos", "double", null, CASCADE::filterDouble, null, FilterCoeffs.BESSEL_CASCADE_ORDER8_CUT5000_F64_SOS, 4);
                benchmarkCascadeAndLog(fp, "bessel_cascade_order8_cut5000_f32_sos", "float", CASCADE::filterFloat, null, FilterCoeffs.BESSEL_CASCADE_ORDER8_CUT5000_F32_SOS, null, 4);
            }
            

        } catch (IOException e) {
            System.err.println("Error writing CSV: " + e.getMessage());
        }
    }
}
