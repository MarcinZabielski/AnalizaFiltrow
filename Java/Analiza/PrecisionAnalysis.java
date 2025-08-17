package Analiza;

import filters.*;
import coeffs.FilterCoeffs;

import java.io.FileWriter;
import java.io.IOException;
import java.util.Locale;
import java.util.Random;

// Analiza precyzji filtrowania w języku java

public class PrecisionAnalysis {

    static final int N = 4096;
    static final Random rand = new Random();

    // === Impuls ===
    static void generateImpulse(float[] x) {
        x[0] = 1.0f;
    }

    static void generateImpulse(double[] x) {
        x[0] = 1.0;
    }

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

    // === Metryki ===
    static double computeMAE(double[] ref, float[] test, int len) {
        double sum = 0.0;
        for (int i = 0; i < len; i++) {
            sum += Math.abs(ref[i] - test[i]);
        }
        return sum / len;
    }

    // === Analiza BA ===
    static void Precision(FileWriter fp, String filterName, String structure,
                              FilterFunction.FloatFilter funcF,
                              FilterFunction.DoubleFilter funcD,
                              float[] bF, float[] aF, double[] bD, double[] aD, int order) throws IOException {

        int cutoff = parseCutoff(filterName);
        String filterType = parseFilterType(filterName);

        float[] xF = new float[N];
        float[] yF = new float[N];
        double[] xD = new double[N];
        double[] yD = new double[N];

        // Impuls
        generateImpulse(xF);
        generateImpulse(xD);
        funcF.apply(xF, yF, bF, aF, N, order);
        funcD.apply(xD, yD, bD, aD, N, order);
        double maeImpulse = computeMAE(yD, yF, N);
        fp.write(String.format(Locale.US, "%s,float,%s,%d,%d,impulse,%.8e\n", filterType, structure, cutoff, order - 1, maeImpulse));

        // Szum
        generateWhiteNoise(xF);
        generateWhiteNoise(xD);
        funcF.apply(xF, yF, bF, aF, N, order);
        funcD.apply(xD, yD, bD, aD, N, order);
        double maeRand = computeMAE(yD, yF, 128); // krótszy fragment jak w C
        fp.write(String.format(Locale.US, "%s,float,%s,%d,%d,rand,%.8e\n", filterType, structure, cutoff, order - 1, maeRand));
    }

    // === Analiza CASCADE ===
    static void PrecisionCascade(FileWriter fp, String filterName,
                                     FilterFunction.FloatCascade funcF,
                                     FilterFunction.DoubleCascade funcD,
                                     float[][] sosF, double[][] sosD, int sections) throws IOException {

        int cutoff = parseCutoff(filterName);
        String filterType = parseFilterType(filterName);

        float[] xF = new float[N];
        float[] yF = new float[N];
        double[] xD = new double[N];
        double[] yD = new double[N];

        float[] flatF = flattenSOS(sosF);
        double[] flatD = flattenSOS(sosD);

        // Impuls
        generateImpulse(xF);
        generateImpulse(xD);
        funcF.apply(xF, yF, flatF, N, sections);
        funcD.apply(xD, yD, flatD, N, sections);
        double maeImpulse = computeMAE(yD, yF, N);
        fp.write(String.format(Locale.US, "%s,float,CASCADE,%d,%d,impulse,%.8e\n", filterType, cutoff, 2 * sections, maeImpulse));

        // Szum
        generateWhiteNoise(xF);
        generateWhiteNoise(xD);
        funcF.apply(xF, yF, flatF, N, sections);
        funcD.apply(xD, yD, flatD, N, sections);
        double maeRand = computeMAE(yD, yF, 128);
        fp.write(String.format(Locale.US, "%s,float,CASCADE,%d,%d,rand,%.8e\n", filterType, cutoff, 2 * sections, maeRand));
    }

    // === Helpery ===
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

    // === MAIN ===
    public static void main(String[] args) {
        try (FileWriter fp = new FileWriter("java_precision_results.csv")) {
            fp.write("filter_name,type,structure,cutoff,order,signal,MAE\n");

            // Tu wkleić zawartość pliku generated_calls_precision_java.txt
            // === AUTO-GENERATED PRECISION ANALYSIS CALLS ===
            // --- DF1 / DF2 / TDF2 ---
            Precision(fp, "butter_df1_order2_cut1000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BUTTER_DF1_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER2_CUT1000_F32_BA[1],FilterCoeffs.BUTTER_DF1_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER2_CUT1000_F64_BA[1],3);
            Precision(fp, "butter_df2_order2_cut1000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BUTTER_DF2_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER2_CUT1000_F32_BA[1],FilterCoeffs.BUTTER_DF2_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER2_CUT1000_F64_BA[1],3);
            Precision(fp, "butter_tdf2_order2_cut1000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BUTTER_TDF2_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER2_CUT1000_F32_BA[1],FilterCoeffs.BUTTER_TDF2_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER2_CUT1000_F64_BA[1],3);
            Precision(fp, "butter_df1_order2_cut2000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BUTTER_DF1_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER2_CUT2000_F32_BA[1],FilterCoeffs.BUTTER_DF1_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER2_CUT2000_F64_BA[1],3);
            Precision(fp, "butter_df2_order2_cut2000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BUTTER_DF2_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER2_CUT2000_F32_BA[1],FilterCoeffs.BUTTER_DF2_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER2_CUT2000_F64_BA[1],3);
            Precision(fp, "butter_tdf2_order2_cut2000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BUTTER_TDF2_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER2_CUT2000_F32_BA[1],FilterCoeffs.BUTTER_TDF2_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER2_CUT2000_F64_BA[1],3);
            Precision(fp, "butter_df1_order2_cut5000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BUTTER_DF1_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER2_CUT5000_F32_BA[1],FilterCoeffs.BUTTER_DF1_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER2_CUT5000_F64_BA[1],3);
            Precision(fp, "butter_df2_order2_cut5000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BUTTER_DF2_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER2_CUT5000_F32_BA[1],FilterCoeffs.BUTTER_DF2_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER2_CUT5000_F64_BA[1],3);
            Precision(fp, "butter_tdf2_order2_cut5000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BUTTER_TDF2_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER2_CUT5000_F32_BA[1],FilterCoeffs.BUTTER_TDF2_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER2_CUT5000_F64_BA[1],3);
            Precision(fp, "butter_df1_order4_cut1000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BUTTER_DF1_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER4_CUT1000_F32_BA[1],FilterCoeffs.BUTTER_DF1_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER4_CUT1000_F64_BA[1],5);
            Precision(fp, "butter_df2_order4_cut1000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BUTTER_DF2_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER4_CUT1000_F32_BA[1],FilterCoeffs.BUTTER_DF2_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER4_CUT1000_F64_BA[1],5);
            Precision(fp, "butter_tdf2_order4_cut1000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BUTTER_TDF2_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER4_CUT1000_F32_BA[1],FilterCoeffs.BUTTER_TDF2_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER4_CUT1000_F64_BA[1],5);
            Precision(fp, "butter_df1_order4_cut2000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BUTTER_DF1_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER4_CUT2000_F32_BA[1],FilterCoeffs.BUTTER_DF1_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER4_CUT2000_F64_BA[1],5);
            Precision(fp, "butter_df2_order4_cut2000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BUTTER_DF2_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER4_CUT2000_F32_BA[1],FilterCoeffs.BUTTER_DF2_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER4_CUT2000_F64_BA[1],5);
            Precision(fp, "butter_tdf2_order4_cut2000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BUTTER_TDF2_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER4_CUT2000_F32_BA[1],FilterCoeffs.BUTTER_TDF2_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER4_CUT2000_F64_BA[1],5);
            Precision(fp, "butter_df1_order4_cut5000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BUTTER_DF1_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER4_CUT5000_F32_BA[1],FilterCoeffs.BUTTER_DF1_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER4_CUT5000_F64_BA[1],5);
            Precision(fp, "butter_df2_order4_cut5000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BUTTER_DF2_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER4_CUT5000_F32_BA[1],FilterCoeffs.BUTTER_DF2_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER4_CUT5000_F64_BA[1],5);
            Precision(fp, "butter_tdf2_order4_cut5000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BUTTER_TDF2_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER4_CUT5000_F32_BA[1],FilterCoeffs.BUTTER_TDF2_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER4_CUT5000_F64_BA[1],5);
            Precision(fp, "butter_df1_order6_cut1000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BUTTER_DF1_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER6_CUT1000_F32_BA[1],FilterCoeffs.BUTTER_DF1_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER6_CUT1000_F64_BA[1],7);
            Precision(fp, "butter_df2_order6_cut1000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BUTTER_DF2_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER6_CUT1000_F32_BA[1],FilterCoeffs.BUTTER_DF2_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER6_CUT1000_F64_BA[1],7);
            Precision(fp, "butter_tdf2_order6_cut1000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BUTTER_TDF2_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER6_CUT1000_F32_BA[1],FilterCoeffs.BUTTER_TDF2_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER6_CUT1000_F64_BA[1],7);
            Precision(fp, "butter_df1_order6_cut2000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BUTTER_DF1_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER6_CUT2000_F32_BA[1],FilterCoeffs.BUTTER_DF1_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER6_CUT2000_F64_BA[1],7);
            Precision(fp, "butter_df2_order6_cut2000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BUTTER_DF2_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER6_CUT2000_F32_BA[1],FilterCoeffs.BUTTER_DF2_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER6_CUT2000_F64_BA[1],7);
            Precision(fp, "butter_tdf2_order6_cut2000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BUTTER_TDF2_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER6_CUT2000_F32_BA[1],FilterCoeffs.BUTTER_TDF2_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER6_CUT2000_F64_BA[1],7);
            Precision(fp, "butter_df1_order6_cut5000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BUTTER_DF1_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER6_CUT5000_F32_BA[1],FilterCoeffs.BUTTER_DF1_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER6_CUT5000_F64_BA[1],7);
            Precision(fp, "butter_df2_order6_cut5000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BUTTER_DF2_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER6_CUT5000_F32_BA[1],FilterCoeffs.BUTTER_DF2_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER6_CUT5000_F64_BA[1],7);
            Precision(fp, "butter_tdf2_order6_cut5000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BUTTER_TDF2_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER6_CUT5000_F32_BA[1],FilterCoeffs.BUTTER_TDF2_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER6_CUT5000_F64_BA[1],7);
            Precision(fp, "butter_df1_order8_cut1000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BUTTER_DF1_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER8_CUT1000_F32_BA[1],FilterCoeffs.BUTTER_DF1_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER8_CUT1000_F64_BA[1],9);
            Precision(fp, "butter_df2_order8_cut1000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BUTTER_DF2_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER8_CUT1000_F32_BA[1],FilterCoeffs.BUTTER_DF2_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER8_CUT1000_F64_BA[1],9);
            Precision(fp, "butter_tdf2_order8_cut1000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BUTTER_TDF2_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER8_CUT1000_F32_BA[1],FilterCoeffs.BUTTER_TDF2_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER8_CUT1000_F64_BA[1],9);
            Precision(fp, "butter_df1_order8_cut2000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BUTTER_DF1_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER8_CUT2000_F32_BA[1],FilterCoeffs.BUTTER_DF1_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER8_CUT2000_F64_BA[1],9);
            Precision(fp, "butter_df2_order8_cut2000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BUTTER_DF2_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER8_CUT2000_F32_BA[1],FilterCoeffs.BUTTER_DF2_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER8_CUT2000_F64_BA[1],9);
            Precision(fp, "butter_tdf2_order8_cut2000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BUTTER_TDF2_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER8_CUT2000_F32_BA[1],FilterCoeffs.BUTTER_TDF2_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER8_CUT2000_F64_BA[1],9);
            Precision(fp, "butter_df1_order8_cut5000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BUTTER_DF1_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_DF1_ORDER8_CUT5000_F32_BA[1],FilterCoeffs.BUTTER_DF1_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_DF1_ORDER8_CUT5000_F64_BA[1],9);
            Precision(fp, "butter_df2_order8_cut5000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BUTTER_DF2_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_DF2_ORDER8_CUT5000_F32_BA[1],FilterCoeffs.BUTTER_DF2_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_DF2_ORDER8_CUT5000_F64_BA[1],9);
            Precision(fp, "butter_tdf2_order8_cut5000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BUTTER_TDF2_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER8_CUT5000_F32_BA[1],FilterCoeffs.BUTTER_TDF2_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.BUTTER_TDF2_ORDER8_CUT5000_F64_BA[1],9);
            Precision(fp, "cheby1_df1_order2_cut1000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY1_DF1_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER2_CUT1000_F32_BA[1],FilterCoeffs.CHEBY1_DF1_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER2_CUT1000_F64_BA[1],3);
            Precision(fp, "cheby1_df2_order2_cut1000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY1_DF2_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER2_CUT1000_F32_BA[1],FilterCoeffs.CHEBY1_DF2_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER2_CUT1000_F64_BA[1],3);
            Precision(fp, "cheby1_tdf2_order2_cut1000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT1000_F32_BA[1],FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT1000_F64_BA[1],3);
            Precision(fp, "cheby1_df1_order2_cut2000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY1_DF1_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER2_CUT2000_F32_BA[1],FilterCoeffs.CHEBY1_DF1_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER2_CUT2000_F64_BA[1],3);
            Precision(fp, "cheby1_df2_order2_cut2000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY1_DF2_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER2_CUT2000_F32_BA[1],FilterCoeffs.CHEBY1_DF2_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER2_CUT2000_F64_BA[1],3);
            Precision(fp, "cheby1_tdf2_order2_cut2000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT2000_F32_BA[1],FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT2000_F64_BA[1],3);
            Precision(fp, "cheby1_df1_order2_cut5000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY1_DF1_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER2_CUT5000_F32_BA[1],FilterCoeffs.CHEBY1_DF1_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER2_CUT5000_F64_BA[1],3);
            Precision(fp, "cheby1_df2_order2_cut5000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY1_DF2_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER2_CUT5000_F32_BA[1],FilterCoeffs.CHEBY1_DF2_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER2_CUT5000_F64_BA[1],3);
            Precision(fp, "cheby1_tdf2_order2_cut5000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT5000_F32_BA[1],FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER2_CUT5000_F64_BA[1],3);
            Precision(fp, "cheby1_df1_order4_cut1000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY1_DF1_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER4_CUT1000_F32_BA[1],FilterCoeffs.CHEBY1_DF1_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER4_CUT1000_F64_BA[1],5);
            Precision(fp, "cheby1_df2_order4_cut1000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY1_DF2_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER4_CUT1000_F32_BA[1],FilterCoeffs.CHEBY1_DF2_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER4_CUT1000_F64_BA[1],5);
            Precision(fp, "cheby1_tdf2_order4_cut1000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT1000_F32_BA[1],FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT1000_F64_BA[1],5);
            Precision(fp, "cheby1_df1_order4_cut2000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY1_DF1_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER4_CUT2000_F32_BA[1],FilterCoeffs.CHEBY1_DF1_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER4_CUT2000_F64_BA[1],5);
            Precision(fp, "cheby1_df2_order4_cut2000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY1_DF2_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER4_CUT2000_F32_BA[1],FilterCoeffs.CHEBY1_DF2_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER4_CUT2000_F64_BA[1],5);
            Precision(fp, "cheby1_tdf2_order4_cut2000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT2000_F32_BA[1],FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT2000_F64_BA[1],5);
            Precision(fp, "cheby1_df1_order4_cut5000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY1_DF1_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER4_CUT5000_F32_BA[1],FilterCoeffs.CHEBY1_DF1_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER4_CUT5000_F64_BA[1],5);
            Precision(fp, "cheby1_df2_order4_cut5000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY1_DF2_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER4_CUT5000_F32_BA[1],FilterCoeffs.CHEBY1_DF2_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER4_CUT5000_F64_BA[1],5);
            Precision(fp, "cheby1_tdf2_order4_cut5000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT5000_F32_BA[1],FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER4_CUT5000_F64_BA[1],5);
            Precision(fp, "cheby1_df1_order6_cut1000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY1_DF1_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER6_CUT1000_F32_BA[1],FilterCoeffs.CHEBY1_DF1_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER6_CUT1000_F64_BA[1],7);
            Precision(fp, "cheby1_df2_order6_cut1000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY1_DF2_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER6_CUT1000_F32_BA[1],FilterCoeffs.CHEBY1_DF2_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER6_CUT1000_F64_BA[1],7);
            Precision(fp, "cheby1_tdf2_order6_cut1000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT1000_F32_BA[1],FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT1000_F64_BA[1],7);
            Precision(fp, "cheby1_df1_order6_cut2000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY1_DF1_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER6_CUT2000_F32_BA[1],FilterCoeffs.CHEBY1_DF1_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER6_CUT2000_F64_BA[1],7);
            Precision(fp, "cheby1_df2_order6_cut2000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY1_DF2_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER6_CUT2000_F32_BA[1],FilterCoeffs.CHEBY1_DF2_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER6_CUT2000_F64_BA[1],7);
            Precision(fp, "cheby1_tdf2_order6_cut2000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT2000_F32_BA[1],FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT2000_F64_BA[1],7);
            Precision(fp, "cheby1_df1_order6_cut5000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY1_DF1_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER6_CUT5000_F32_BA[1],FilterCoeffs.CHEBY1_DF1_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER6_CUT5000_F64_BA[1],7);
            Precision(fp, "cheby1_df2_order6_cut5000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY1_DF2_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER6_CUT5000_F32_BA[1],FilterCoeffs.CHEBY1_DF2_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER6_CUT5000_F64_BA[1],7);
            Precision(fp, "cheby1_tdf2_order6_cut5000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT5000_F32_BA[1],FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER6_CUT5000_F64_BA[1],7);
            Precision(fp, "cheby1_df1_order8_cut1000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY1_DF1_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER8_CUT1000_F32_BA[1],FilterCoeffs.CHEBY1_DF1_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER8_CUT1000_F64_BA[1],9);
            Precision(fp, "cheby1_df2_order8_cut1000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY1_DF2_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER8_CUT1000_F32_BA[1],FilterCoeffs.CHEBY1_DF2_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER8_CUT1000_F64_BA[1],9);
            Precision(fp, "cheby1_tdf2_order8_cut1000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT1000_F32_BA[1],FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT1000_F64_BA[1],9);
            Precision(fp, "cheby1_df1_order8_cut2000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY1_DF1_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER8_CUT2000_F32_BA[1],FilterCoeffs.CHEBY1_DF1_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER8_CUT2000_F64_BA[1],9);
            Precision(fp, "cheby1_df2_order8_cut2000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY1_DF2_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER8_CUT2000_F32_BA[1],FilterCoeffs.CHEBY1_DF2_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER8_CUT2000_F64_BA[1],9);
            Precision(fp, "cheby1_tdf2_order8_cut2000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT2000_F32_BA[1],FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT2000_F64_BA[1],9);
            Precision(fp, "cheby1_df1_order8_cut5000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY1_DF1_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER8_CUT5000_F32_BA[1],FilterCoeffs.CHEBY1_DF1_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_DF1_ORDER8_CUT5000_F64_BA[1],9);
            Precision(fp, "cheby1_df2_order8_cut5000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY1_DF2_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER8_CUT5000_F32_BA[1],FilterCoeffs.CHEBY1_DF2_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_DF2_ORDER8_CUT5000_F64_BA[1],9);
            Precision(fp, "cheby1_tdf2_order8_cut5000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT5000_F32_BA[1],FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.CHEBY1_TDF2_ORDER8_CUT5000_F64_BA[1],9);
            Precision(fp, "cheby2_df1_order2_cut1000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY2_DF1_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER2_CUT1000_F32_BA[1],FilterCoeffs.CHEBY2_DF1_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER2_CUT1000_F64_BA[1],3);
            Precision(fp, "cheby2_df2_order2_cut1000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY2_DF2_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER2_CUT1000_F32_BA[1],FilterCoeffs.CHEBY2_DF2_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER2_CUT1000_F64_BA[1],3);
            Precision(fp, "cheby2_tdf2_order2_cut1000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT1000_F32_BA[1],FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT1000_F64_BA[1],3);
            Precision(fp, "cheby2_df1_order2_cut2000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY2_DF1_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER2_CUT2000_F32_BA[1],FilterCoeffs.CHEBY2_DF1_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER2_CUT2000_F64_BA[1],3);
            Precision(fp, "cheby2_df2_order2_cut2000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY2_DF2_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER2_CUT2000_F32_BA[1],FilterCoeffs.CHEBY2_DF2_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER2_CUT2000_F64_BA[1],3);
            Precision(fp, "cheby2_tdf2_order2_cut2000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT2000_F32_BA[1],FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT2000_F64_BA[1],3);
            Precision(fp, "cheby2_df1_order2_cut5000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY2_DF1_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER2_CUT5000_F32_BA[1],FilterCoeffs.CHEBY2_DF1_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER2_CUT5000_F64_BA[1],3);
            Precision(fp, "cheby2_df2_order2_cut5000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY2_DF2_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER2_CUT5000_F32_BA[1],FilterCoeffs.CHEBY2_DF2_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER2_CUT5000_F64_BA[1],3);
            Precision(fp, "cheby2_tdf2_order2_cut5000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT5000_F32_BA[1],FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER2_CUT5000_F64_BA[1],3);
            Precision(fp, "cheby2_df1_order4_cut1000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY2_DF1_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER4_CUT1000_F32_BA[1],FilterCoeffs.CHEBY2_DF1_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER4_CUT1000_F64_BA[1],5);
            Precision(fp, "cheby2_df2_order4_cut1000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY2_DF2_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER4_CUT1000_F32_BA[1],FilterCoeffs.CHEBY2_DF2_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER4_CUT1000_F64_BA[1],5);
            Precision(fp, "cheby2_tdf2_order4_cut1000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT1000_F32_BA[1],FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT1000_F64_BA[1],5);
            Precision(fp, "cheby2_df1_order4_cut2000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY2_DF1_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER4_CUT2000_F32_BA[1],FilterCoeffs.CHEBY2_DF1_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER4_CUT2000_F64_BA[1],5);
            Precision(fp, "cheby2_df2_order4_cut2000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY2_DF2_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER4_CUT2000_F32_BA[1],FilterCoeffs.CHEBY2_DF2_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER4_CUT2000_F64_BA[1],5);
            Precision(fp, "cheby2_tdf2_order4_cut2000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT2000_F32_BA[1],FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT2000_F64_BA[1],5);
            Precision(fp, "cheby2_df1_order4_cut5000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY2_DF1_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER4_CUT5000_F32_BA[1],FilterCoeffs.CHEBY2_DF1_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER4_CUT5000_F64_BA[1],5);
            Precision(fp, "cheby2_df2_order4_cut5000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY2_DF2_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER4_CUT5000_F32_BA[1],FilterCoeffs.CHEBY2_DF2_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER4_CUT5000_F64_BA[1],5);
            Precision(fp, "cheby2_tdf2_order4_cut5000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT5000_F32_BA[1],FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER4_CUT5000_F64_BA[1],5);
            Precision(fp, "cheby2_df1_order6_cut1000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY2_DF1_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER6_CUT1000_F32_BA[1],FilterCoeffs.CHEBY2_DF1_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER6_CUT1000_F64_BA[1],7);
            Precision(fp, "cheby2_df2_order6_cut1000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY2_DF2_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER6_CUT1000_F32_BA[1],FilterCoeffs.CHEBY2_DF2_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER6_CUT1000_F64_BA[1],7);
            Precision(fp, "cheby2_tdf2_order6_cut1000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT1000_F32_BA[1],FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT1000_F64_BA[1],7);
            Precision(fp, "cheby2_df1_order6_cut2000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY2_DF1_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER6_CUT2000_F32_BA[1],FilterCoeffs.CHEBY2_DF1_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER6_CUT2000_F64_BA[1],7);
            Precision(fp, "cheby2_df2_order6_cut2000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY2_DF2_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER6_CUT2000_F32_BA[1],FilterCoeffs.CHEBY2_DF2_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER6_CUT2000_F64_BA[1],7);
            Precision(fp, "cheby2_tdf2_order6_cut2000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT2000_F32_BA[1],FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT2000_F64_BA[1],7);
            Precision(fp, "cheby2_df1_order6_cut5000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY2_DF1_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER6_CUT5000_F32_BA[1],FilterCoeffs.CHEBY2_DF1_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER6_CUT5000_F64_BA[1],7);
            Precision(fp, "cheby2_df2_order6_cut5000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY2_DF2_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER6_CUT5000_F32_BA[1],FilterCoeffs.CHEBY2_DF2_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER6_CUT5000_F64_BA[1],7);
            Precision(fp, "cheby2_tdf2_order6_cut5000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT5000_F32_BA[1],FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER6_CUT5000_F64_BA[1],7);
            Precision(fp, "cheby2_df1_order8_cut1000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY2_DF1_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER8_CUT1000_F32_BA[1],FilterCoeffs.CHEBY2_DF1_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER8_CUT1000_F64_BA[1],9);
            Precision(fp, "cheby2_df2_order8_cut1000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY2_DF2_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER8_CUT1000_F32_BA[1],FilterCoeffs.CHEBY2_DF2_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER8_CUT1000_F64_BA[1],9);
            Precision(fp, "cheby2_tdf2_order8_cut1000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT1000_F32_BA[1],FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT1000_F64_BA[1],9);
            Precision(fp, "cheby2_df1_order8_cut2000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY2_DF1_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER8_CUT2000_F32_BA[1],FilterCoeffs.CHEBY2_DF1_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER8_CUT2000_F64_BA[1],9);
            Precision(fp, "cheby2_df2_order8_cut2000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY2_DF2_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER8_CUT2000_F32_BA[1],FilterCoeffs.CHEBY2_DF2_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER8_CUT2000_F64_BA[1],9);
            Precision(fp, "cheby2_tdf2_order8_cut2000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT2000_F32_BA[1],FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT2000_F64_BA[1],9);
            Precision(fp, "cheby2_df1_order8_cut5000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.CHEBY2_DF1_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER8_CUT5000_F32_BA[1],FilterCoeffs.CHEBY2_DF1_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_DF1_ORDER8_CUT5000_F64_BA[1],9);
            Precision(fp, "cheby2_df2_order8_cut5000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.CHEBY2_DF2_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER8_CUT5000_F32_BA[1],FilterCoeffs.CHEBY2_DF2_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_DF2_ORDER8_CUT5000_F64_BA[1],9);
            Precision(fp, "cheby2_tdf2_order8_cut5000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT5000_F32_BA[1],FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.CHEBY2_TDF2_ORDER8_CUT5000_F64_BA[1],9);
            Precision(fp, "ellip_df1_order2_cut1000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.ELLIP_DF1_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER2_CUT1000_F32_BA[1],FilterCoeffs.ELLIP_DF1_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER2_CUT1000_F64_BA[1],3);
            Precision(fp, "ellip_df2_order2_cut1000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.ELLIP_DF2_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER2_CUT1000_F32_BA[1],FilterCoeffs.ELLIP_DF2_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER2_CUT1000_F64_BA[1],3);
            Precision(fp, "ellip_tdf2_order2_cut1000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.ELLIP_TDF2_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER2_CUT1000_F32_BA[1],FilterCoeffs.ELLIP_TDF2_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER2_CUT1000_F64_BA[1],3);
            Precision(fp, "ellip_df1_order2_cut2000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.ELLIP_DF1_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER2_CUT2000_F32_BA[1],FilterCoeffs.ELLIP_DF1_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER2_CUT2000_F64_BA[1],3);
            Precision(fp, "ellip_df2_order2_cut2000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.ELLIP_DF2_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER2_CUT2000_F32_BA[1],FilterCoeffs.ELLIP_DF2_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER2_CUT2000_F64_BA[1],3);
            Precision(fp, "ellip_tdf2_order2_cut2000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.ELLIP_TDF2_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER2_CUT2000_F32_BA[1],FilterCoeffs.ELLIP_TDF2_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER2_CUT2000_F64_BA[1],3);
            Precision(fp, "ellip_df1_order2_cut5000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.ELLIP_DF1_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER2_CUT5000_F32_BA[1],FilterCoeffs.ELLIP_DF1_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER2_CUT5000_F64_BA[1],3);
            Precision(fp, "ellip_df2_order2_cut5000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.ELLIP_DF2_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER2_CUT5000_F32_BA[1],FilterCoeffs.ELLIP_DF2_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER2_CUT5000_F64_BA[1],3);
            Precision(fp, "ellip_tdf2_order2_cut5000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.ELLIP_TDF2_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER2_CUT5000_F32_BA[1],FilterCoeffs.ELLIP_TDF2_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER2_CUT5000_F64_BA[1],3);
            Precision(fp, "ellip_df1_order4_cut1000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.ELLIP_DF1_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER4_CUT1000_F32_BA[1],FilterCoeffs.ELLIP_DF1_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER4_CUT1000_F64_BA[1],5);
            Precision(fp, "ellip_df2_order4_cut1000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.ELLIP_DF2_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER4_CUT1000_F32_BA[1],FilterCoeffs.ELLIP_DF2_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER4_CUT1000_F64_BA[1],5);
            Precision(fp, "ellip_tdf2_order4_cut1000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.ELLIP_TDF2_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER4_CUT1000_F32_BA[1],FilterCoeffs.ELLIP_TDF2_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER4_CUT1000_F64_BA[1],5);
            Precision(fp, "ellip_df1_order4_cut2000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.ELLIP_DF1_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER4_CUT2000_F32_BA[1],FilterCoeffs.ELLIP_DF1_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER4_CUT2000_F64_BA[1],5);
            Precision(fp, "ellip_df2_order4_cut2000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.ELLIP_DF2_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER4_CUT2000_F32_BA[1],FilterCoeffs.ELLIP_DF2_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER4_CUT2000_F64_BA[1],5);
            Precision(fp, "ellip_tdf2_order4_cut2000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.ELLIP_TDF2_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER4_CUT2000_F32_BA[1],FilterCoeffs.ELLIP_TDF2_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER4_CUT2000_F64_BA[1],5);
            Precision(fp, "ellip_df1_order4_cut5000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.ELLIP_DF1_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER4_CUT5000_F32_BA[1],FilterCoeffs.ELLIP_DF1_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER4_CUT5000_F64_BA[1],5);
            Precision(fp, "ellip_df2_order4_cut5000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.ELLIP_DF2_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER4_CUT5000_F32_BA[1],FilterCoeffs.ELLIP_DF2_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER4_CUT5000_F64_BA[1],5);
            Precision(fp, "ellip_tdf2_order4_cut5000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.ELLIP_TDF2_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER4_CUT5000_F32_BA[1],FilterCoeffs.ELLIP_TDF2_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER4_CUT5000_F64_BA[1],5);
            Precision(fp, "ellip_df1_order6_cut1000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.ELLIP_DF1_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER6_CUT1000_F32_BA[1],FilterCoeffs.ELLIP_DF1_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER6_CUT1000_F64_BA[1],7);
            Precision(fp, "ellip_df2_order6_cut1000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.ELLIP_DF2_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER6_CUT1000_F32_BA[1],FilterCoeffs.ELLIP_DF2_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER6_CUT1000_F64_BA[1],7);
            Precision(fp, "ellip_tdf2_order6_cut1000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.ELLIP_TDF2_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER6_CUT1000_F32_BA[1],FilterCoeffs.ELLIP_TDF2_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER6_CUT1000_F64_BA[1],7);
            Precision(fp, "ellip_df1_order6_cut2000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.ELLIP_DF1_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER6_CUT2000_F32_BA[1],FilterCoeffs.ELLIP_DF1_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER6_CUT2000_F64_BA[1],7);
            Precision(fp, "ellip_df2_order6_cut2000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.ELLIP_DF2_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER6_CUT2000_F32_BA[1],FilterCoeffs.ELLIP_DF2_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER6_CUT2000_F64_BA[1],7);
            Precision(fp, "ellip_tdf2_order6_cut2000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.ELLIP_TDF2_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER6_CUT2000_F32_BA[1],FilterCoeffs.ELLIP_TDF2_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER6_CUT2000_F64_BA[1],7);
            Precision(fp, "ellip_df1_order6_cut5000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.ELLIP_DF1_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER6_CUT5000_F32_BA[1],FilterCoeffs.ELLIP_DF1_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER6_CUT5000_F64_BA[1],7);
            Precision(fp, "ellip_df2_order6_cut5000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.ELLIP_DF2_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER6_CUT5000_F32_BA[1],FilterCoeffs.ELLIP_DF2_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER6_CUT5000_F64_BA[1],7);
            Precision(fp, "ellip_tdf2_order6_cut5000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.ELLIP_TDF2_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER6_CUT5000_F32_BA[1],FilterCoeffs.ELLIP_TDF2_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER6_CUT5000_F64_BA[1],7);
            Precision(fp, "ellip_df1_order8_cut1000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.ELLIP_DF1_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER8_CUT1000_F32_BA[1],FilterCoeffs.ELLIP_DF1_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER8_CUT1000_F64_BA[1],9);
            Precision(fp, "ellip_df2_order8_cut1000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.ELLIP_DF2_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER8_CUT1000_F32_BA[1],FilterCoeffs.ELLIP_DF2_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER8_CUT1000_F64_BA[1],9);
            Precision(fp, "ellip_tdf2_order8_cut1000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.ELLIP_TDF2_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER8_CUT1000_F32_BA[1],FilterCoeffs.ELLIP_TDF2_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER8_CUT1000_F64_BA[1],9);
            Precision(fp, "ellip_df1_order8_cut2000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.ELLIP_DF1_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER8_CUT2000_F32_BA[1],FilterCoeffs.ELLIP_DF1_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER8_CUT2000_F64_BA[1],9);
            Precision(fp, "ellip_df2_order8_cut2000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.ELLIP_DF2_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER8_CUT2000_F32_BA[1],FilterCoeffs.ELLIP_DF2_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER8_CUT2000_F64_BA[1],9);
            Precision(fp, "ellip_tdf2_order8_cut2000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.ELLIP_TDF2_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER8_CUT2000_F32_BA[1],FilterCoeffs.ELLIP_TDF2_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER8_CUT2000_F64_BA[1],9);
            Precision(fp, "ellip_df1_order8_cut5000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.ELLIP_DF1_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_DF1_ORDER8_CUT5000_F32_BA[1],FilterCoeffs.ELLIP_DF1_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_DF1_ORDER8_CUT5000_F64_BA[1],9);
            Precision(fp, "ellip_df2_order8_cut5000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.ELLIP_DF2_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_DF2_ORDER8_CUT5000_F32_BA[1],FilterCoeffs.ELLIP_DF2_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_DF2_ORDER8_CUT5000_F64_BA[1],9);
            Precision(fp, "ellip_tdf2_order8_cut5000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.ELLIP_TDF2_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER8_CUT5000_F32_BA[1],FilterCoeffs.ELLIP_TDF2_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.ELLIP_TDF2_ORDER8_CUT5000_F64_BA[1],9);
            Precision(fp, "bessel_df1_order2_cut1000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BESSEL_DF1_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER2_CUT1000_F32_BA[1],FilterCoeffs.BESSEL_DF1_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER2_CUT1000_F64_BA[1],3);
            Precision(fp, "bessel_df2_order2_cut1000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BESSEL_DF2_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER2_CUT1000_F32_BA[1],FilterCoeffs.BESSEL_DF2_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER2_CUT1000_F64_BA[1],3);
            Precision(fp, "bessel_tdf2_order2_cut1000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BESSEL_TDF2_ORDER2_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER2_CUT1000_F32_BA[1],FilterCoeffs.BESSEL_TDF2_ORDER2_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER2_CUT1000_F64_BA[1],3);
            Precision(fp, "bessel_df1_order2_cut2000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BESSEL_DF1_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER2_CUT2000_F32_BA[1],FilterCoeffs.BESSEL_DF1_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER2_CUT2000_F64_BA[1],3);
            Precision(fp, "bessel_df2_order2_cut2000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BESSEL_DF2_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER2_CUT2000_F32_BA[1],FilterCoeffs.BESSEL_DF2_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER2_CUT2000_F64_BA[1],3);
            Precision(fp, "bessel_tdf2_order2_cut2000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BESSEL_TDF2_ORDER2_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER2_CUT2000_F32_BA[1],FilterCoeffs.BESSEL_TDF2_ORDER2_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER2_CUT2000_F64_BA[1],3);
            Precision(fp, "bessel_df1_order2_cut5000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BESSEL_DF1_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER2_CUT5000_F32_BA[1],FilterCoeffs.BESSEL_DF1_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER2_CUT5000_F64_BA[1],3);
            Precision(fp, "bessel_df2_order2_cut5000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BESSEL_DF2_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER2_CUT5000_F32_BA[1],FilterCoeffs.BESSEL_DF2_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER2_CUT5000_F64_BA[1],3);
            Precision(fp, "bessel_tdf2_order2_cut5000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BESSEL_TDF2_ORDER2_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER2_CUT5000_F32_BA[1],FilterCoeffs.BESSEL_TDF2_ORDER2_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER2_CUT5000_F64_BA[1],3);
            Precision(fp, "bessel_df1_order4_cut1000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BESSEL_DF1_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER4_CUT1000_F32_BA[1],FilterCoeffs.BESSEL_DF1_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER4_CUT1000_F64_BA[1],5);
            Precision(fp, "bessel_df2_order4_cut1000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BESSEL_DF2_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER4_CUT1000_F32_BA[1],FilterCoeffs.BESSEL_DF2_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER4_CUT1000_F64_BA[1],5);
            Precision(fp, "bessel_tdf2_order4_cut1000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BESSEL_TDF2_ORDER4_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER4_CUT1000_F32_BA[1],FilterCoeffs.BESSEL_TDF2_ORDER4_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER4_CUT1000_F64_BA[1],5);
            Precision(fp, "bessel_df1_order4_cut2000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BESSEL_DF1_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER4_CUT2000_F32_BA[1],FilterCoeffs.BESSEL_DF1_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER4_CUT2000_F64_BA[1],5);
            Precision(fp, "bessel_df2_order4_cut2000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BESSEL_DF2_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER4_CUT2000_F32_BA[1],FilterCoeffs.BESSEL_DF2_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER4_CUT2000_F64_BA[1],5);
            Precision(fp, "bessel_tdf2_order4_cut2000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BESSEL_TDF2_ORDER4_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER4_CUT2000_F32_BA[1],FilterCoeffs.BESSEL_TDF2_ORDER4_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER4_CUT2000_F64_BA[1],5);
            Precision(fp, "bessel_df1_order4_cut5000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BESSEL_DF1_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER4_CUT5000_F32_BA[1],FilterCoeffs.BESSEL_DF1_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER4_CUT5000_F64_BA[1],5);
            Precision(fp, "bessel_df2_order4_cut5000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BESSEL_DF2_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER4_CUT5000_F32_BA[1],FilterCoeffs.BESSEL_DF2_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER4_CUT5000_F64_BA[1],5);
            Precision(fp, "bessel_tdf2_order4_cut5000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BESSEL_TDF2_ORDER4_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER4_CUT5000_F32_BA[1],FilterCoeffs.BESSEL_TDF2_ORDER4_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER4_CUT5000_F64_BA[1],5);
            Precision(fp, "bessel_df1_order6_cut1000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BESSEL_DF1_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER6_CUT1000_F32_BA[1],FilterCoeffs.BESSEL_DF1_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER6_CUT1000_F64_BA[1],7);
            Precision(fp, "bessel_df2_order6_cut1000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BESSEL_DF2_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER6_CUT1000_F32_BA[1],FilterCoeffs.BESSEL_DF2_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER6_CUT1000_F64_BA[1],7);
            Precision(fp, "bessel_tdf2_order6_cut1000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BESSEL_TDF2_ORDER6_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER6_CUT1000_F32_BA[1],FilterCoeffs.BESSEL_TDF2_ORDER6_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER6_CUT1000_F64_BA[1],7);
            Precision(fp, "bessel_df1_order6_cut2000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BESSEL_DF1_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER6_CUT2000_F32_BA[1],FilterCoeffs.BESSEL_DF1_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER6_CUT2000_F64_BA[1],7);
            Precision(fp, "bessel_df2_order6_cut2000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BESSEL_DF2_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER6_CUT2000_F32_BA[1],FilterCoeffs.BESSEL_DF2_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER6_CUT2000_F64_BA[1],7);
            Precision(fp, "bessel_tdf2_order6_cut2000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BESSEL_TDF2_ORDER6_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER6_CUT2000_F32_BA[1],FilterCoeffs.BESSEL_TDF2_ORDER6_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER6_CUT2000_F64_BA[1],7);
            Precision(fp, "bessel_df1_order6_cut5000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BESSEL_DF1_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER6_CUT5000_F32_BA[1],FilterCoeffs.BESSEL_DF1_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER6_CUT5000_F64_BA[1],7);
            Precision(fp, "bessel_df2_order6_cut5000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BESSEL_DF2_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER6_CUT5000_F32_BA[1],FilterCoeffs.BESSEL_DF2_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER6_CUT5000_F64_BA[1],7);
            Precision(fp, "bessel_tdf2_order6_cut5000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BESSEL_TDF2_ORDER6_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER6_CUT5000_F32_BA[1],FilterCoeffs.BESSEL_TDF2_ORDER6_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER6_CUT5000_F64_BA[1],7);
            Precision(fp, "bessel_df1_order8_cut1000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BESSEL_DF1_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER8_CUT1000_F32_BA[1],FilterCoeffs.BESSEL_DF1_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER8_CUT1000_F64_BA[1],9);
            Precision(fp, "bessel_df2_order8_cut1000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BESSEL_DF2_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER8_CUT1000_F32_BA[1],FilterCoeffs.BESSEL_DF2_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER8_CUT1000_F64_BA[1],9);
            Precision(fp, "bessel_tdf2_order8_cut1000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BESSEL_TDF2_ORDER8_CUT1000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER8_CUT1000_F32_BA[1],FilterCoeffs.BESSEL_TDF2_ORDER8_CUT1000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER8_CUT1000_F64_BA[1],9);
            Precision(fp, "bessel_df1_order8_cut2000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BESSEL_DF1_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER8_CUT2000_F32_BA[1],FilterCoeffs.BESSEL_DF1_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER8_CUT2000_F64_BA[1],9);
            Precision(fp, "bessel_df2_order8_cut2000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BESSEL_DF2_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER8_CUT2000_F32_BA[1],FilterCoeffs.BESSEL_DF2_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER8_CUT2000_F64_BA[1],9);
            Precision(fp, "bessel_tdf2_order8_cut2000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BESSEL_TDF2_ORDER8_CUT2000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER8_CUT2000_F32_BA[1],FilterCoeffs.BESSEL_TDF2_ORDER8_CUT2000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER8_CUT2000_F64_BA[1],9);
            Precision(fp, "bessel_df1_order8_cut5000_f64_ba", "DF1",DF1::filterFloat, DF1::filterDouble,FilterCoeffs.BESSEL_DF1_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_DF1_ORDER8_CUT5000_F32_BA[1],FilterCoeffs.BESSEL_DF1_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_DF1_ORDER8_CUT5000_F64_BA[1],9);
            Precision(fp, "bessel_df2_order8_cut5000_f64_ba", "DF2",DF2::filterFloat, DF2::filterDouble,FilterCoeffs.BESSEL_DF2_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_DF2_ORDER8_CUT5000_F32_BA[1],FilterCoeffs.BESSEL_DF2_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_DF2_ORDER8_CUT5000_F64_BA[1],9);
            Precision(fp, "bessel_tdf2_order8_cut5000_f64_ba", "TDF2",TDF2::filterFloat, TDF2::filterDouble,FilterCoeffs.BESSEL_TDF2_ORDER8_CUT5000_F32_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER8_CUT5000_F32_BA[1],FilterCoeffs.BESSEL_TDF2_ORDER8_CUT5000_F64_BA[0], FilterCoeffs.BESSEL_TDF2_ORDER8_CUT5000_F64_BA[1],9);

            // --- CASCADE ---
            PrecisionCascade(fp, "butter_cascade_order2_cut1000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BUTTER_CASCADE_ORDER2_CUT1000_F32_SOS,FilterCoeffs.BUTTER_CASCADE_ORDER2_CUT1000_F64_SOS,1);
            PrecisionCascade(fp, "butter_cascade_order2_cut2000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BUTTER_CASCADE_ORDER2_CUT2000_F32_SOS,FilterCoeffs.BUTTER_CASCADE_ORDER2_CUT2000_F64_SOS,1);
            PrecisionCascade(fp, "butter_cascade_order2_cut5000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BUTTER_CASCADE_ORDER2_CUT5000_F32_SOS,FilterCoeffs.BUTTER_CASCADE_ORDER2_CUT5000_F64_SOS,1);
            PrecisionCascade(fp, "butter_cascade_order4_cut1000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BUTTER_CASCADE_ORDER4_CUT1000_F32_SOS,FilterCoeffs.BUTTER_CASCADE_ORDER4_CUT1000_F64_SOS,2);
            PrecisionCascade(fp, "butter_cascade_order4_cut2000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BUTTER_CASCADE_ORDER4_CUT2000_F32_SOS,FilterCoeffs.BUTTER_CASCADE_ORDER4_CUT2000_F64_SOS,2);
            PrecisionCascade(fp, "butter_cascade_order4_cut5000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BUTTER_CASCADE_ORDER4_CUT5000_F32_SOS,FilterCoeffs.BUTTER_CASCADE_ORDER4_CUT5000_F64_SOS,2);
            PrecisionCascade(fp, "butter_cascade_order6_cut1000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BUTTER_CASCADE_ORDER6_CUT1000_F32_SOS,FilterCoeffs.BUTTER_CASCADE_ORDER6_CUT1000_F64_SOS,3);
            PrecisionCascade(fp, "butter_cascade_order6_cut2000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BUTTER_CASCADE_ORDER6_CUT2000_F32_SOS,FilterCoeffs.BUTTER_CASCADE_ORDER6_CUT2000_F64_SOS,3);
            PrecisionCascade(fp, "butter_cascade_order6_cut5000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BUTTER_CASCADE_ORDER6_CUT5000_F32_SOS,FilterCoeffs.BUTTER_CASCADE_ORDER6_CUT5000_F64_SOS,3);
            PrecisionCascade(fp, "butter_cascade_order8_cut1000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BUTTER_CASCADE_ORDER8_CUT1000_F32_SOS,FilterCoeffs.BUTTER_CASCADE_ORDER8_CUT1000_F64_SOS,4);
            PrecisionCascade(fp, "butter_cascade_order8_cut2000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BUTTER_CASCADE_ORDER8_CUT2000_F32_SOS,FilterCoeffs.BUTTER_CASCADE_ORDER8_CUT2000_F64_SOS,4);
            PrecisionCascade(fp, "butter_cascade_order8_cut5000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BUTTER_CASCADE_ORDER8_CUT5000_F32_SOS,FilterCoeffs.BUTTER_CASCADE_ORDER8_CUT5000_F64_SOS,4);
            PrecisionCascade(fp, "cheby1_cascade_order2_cut1000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY1_CASCADE_ORDER2_CUT1000_F32_SOS,FilterCoeffs.CHEBY1_CASCADE_ORDER2_CUT1000_F64_SOS,1);
            PrecisionCascade(fp, "cheby1_cascade_order2_cut2000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY1_CASCADE_ORDER2_CUT2000_F32_SOS,FilterCoeffs.CHEBY1_CASCADE_ORDER2_CUT2000_F64_SOS,1);
            PrecisionCascade(fp, "cheby1_cascade_order2_cut5000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY1_CASCADE_ORDER2_CUT5000_F32_SOS,FilterCoeffs.CHEBY1_CASCADE_ORDER2_CUT5000_F64_SOS,1);
            PrecisionCascade(fp, "cheby1_cascade_order4_cut1000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY1_CASCADE_ORDER4_CUT1000_F32_SOS,FilterCoeffs.CHEBY1_CASCADE_ORDER4_CUT1000_F64_SOS,2);
            PrecisionCascade(fp, "cheby1_cascade_order4_cut2000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY1_CASCADE_ORDER4_CUT2000_F32_SOS,FilterCoeffs.CHEBY1_CASCADE_ORDER4_CUT2000_F64_SOS,2);
            PrecisionCascade(fp, "cheby1_cascade_order4_cut5000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY1_CASCADE_ORDER4_CUT5000_F32_SOS,FilterCoeffs.CHEBY1_CASCADE_ORDER4_CUT5000_F64_SOS,2);
            PrecisionCascade(fp, "cheby1_cascade_order6_cut1000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY1_CASCADE_ORDER6_CUT1000_F32_SOS,FilterCoeffs.CHEBY1_CASCADE_ORDER6_CUT1000_F64_SOS,3);
            PrecisionCascade(fp, "cheby1_cascade_order6_cut2000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY1_CASCADE_ORDER6_CUT2000_F32_SOS,FilterCoeffs.CHEBY1_CASCADE_ORDER6_CUT2000_F64_SOS,3);
            PrecisionCascade(fp, "cheby1_cascade_order6_cut5000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY1_CASCADE_ORDER6_CUT5000_F32_SOS,FilterCoeffs.CHEBY1_CASCADE_ORDER6_CUT5000_F64_SOS,3);
            PrecisionCascade(fp, "cheby1_cascade_order8_cut1000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY1_CASCADE_ORDER8_CUT1000_F32_SOS,FilterCoeffs.CHEBY1_CASCADE_ORDER8_CUT1000_F64_SOS,4);
            PrecisionCascade(fp, "cheby1_cascade_order8_cut2000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY1_CASCADE_ORDER8_CUT2000_F32_SOS,FilterCoeffs.CHEBY1_CASCADE_ORDER8_CUT2000_F64_SOS,4);
            PrecisionCascade(fp, "cheby1_cascade_order8_cut5000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY1_CASCADE_ORDER8_CUT5000_F32_SOS,FilterCoeffs.CHEBY1_CASCADE_ORDER8_CUT5000_F64_SOS,4);
            PrecisionCascade(fp, "cheby2_cascade_order2_cut1000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY2_CASCADE_ORDER2_CUT1000_F32_SOS,FilterCoeffs.CHEBY2_CASCADE_ORDER2_CUT1000_F64_SOS,1);
            PrecisionCascade(fp, "cheby2_cascade_order2_cut2000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY2_CASCADE_ORDER2_CUT2000_F32_SOS,FilterCoeffs.CHEBY2_CASCADE_ORDER2_CUT2000_F64_SOS,1);
            PrecisionCascade(fp, "cheby2_cascade_order2_cut5000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY2_CASCADE_ORDER2_CUT5000_F32_SOS,FilterCoeffs.CHEBY2_CASCADE_ORDER2_CUT5000_F64_SOS,1);
            PrecisionCascade(fp, "cheby2_cascade_order4_cut1000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY2_CASCADE_ORDER4_CUT1000_F32_SOS,FilterCoeffs.CHEBY2_CASCADE_ORDER4_CUT1000_F64_SOS,2);
            PrecisionCascade(fp, "cheby2_cascade_order4_cut2000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY2_CASCADE_ORDER4_CUT2000_F32_SOS,FilterCoeffs.CHEBY2_CASCADE_ORDER4_CUT2000_F64_SOS,2);
            PrecisionCascade(fp, "cheby2_cascade_order4_cut5000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY2_CASCADE_ORDER4_CUT5000_F32_SOS,FilterCoeffs.CHEBY2_CASCADE_ORDER4_CUT5000_F64_SOS,2);
            PrecisionCascade(fp, "cheby2_cascade_order6_cut1000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY2_CASCADE_ORDER6_CUT1000_F32_SOS,FilterCoeffs.CHEBY2_CASCADE_ORDER6_CUT1000_F64_SOS,3);
            PrecisionCascade(fp, "cheby2_cascade_order6_cut2000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY2_CASCADE_ORDER6_CUT2000_F32_SOS,FilterCoeffs.CHEBY2_CASCADE_ORDER6_CUT2000_F64_SOS,3);
            PrecisionCascade(fp, "cheby2_cascade_order6_cut5000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY2_CASCADE_ORDER6_CUT5000_F32_SOS,FilterCoeffs.CHEBY2_CASCADE_ORDER6_CUT5000_F64_SOS,3);
            PrecisionCascade(fp, "cheby2_cascade_order8_cut1000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY2_CASCADE_ORDER8_CUT1000_F32_SOS,FilterCoeffs.CHEBY2_CASCADE_ORDER8_CUT1000_F64_SOS,4);
            PrecisionCascade(fp, "cheby2_cascade_order8_cut2000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY2_CASCADE_ORDER8_CUT2000_F32_SOS,FilterCoeffs.CHEBY2_CASCADE_ORDER8_CUT2000_F64_SOS,4);
            PrecisionCascade(fp, "cheby2_cascade_order8_cut5000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.CHEBY2_CASCADE_ORDER8_CUT5000_F32_SOS,FilterCoeffs.CHEBY2_CASCADE_ORDER8_CUT5000_F64_SOS,4);
            PrecisionCascade(fp, "ellip_cascade_order2_cut1000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.ELLIP_CASCADE_ORDER2_CUT1000_F32_SOS,FilterCoeffs.ELLIP_CASCADE_ORDER2_CUT1000_F64_SOS,1);
            PrecisionCascade(fp, "ellip_cascade_order2_cut2000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.ELLIP_CASCADE_ORDER2_CUT2000_F32_SOS,FilterCoeffs.ELLIP_CASCADE_ORDER2_CUT2000_F64_SOS,1);
            PrecisionCascade(fp, "ellip_cascade_order2_cut5000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.ELLIP_CASCADE_ORDER2_CUT5000_F32_SOS,FilterCoeffs.ELLIP_CASCADE_ORDER2_CUT5000_F64_SOS,1);
            PrecisionCascade(fp, "ellip_cascade_order4_cut1000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.ELLIP_CASCADE_ORDER4_CUT1000_F32_SOS,FilterCoeffs.ELLIP_CASCADE_ORDER4_CUT1000_F64_SOS,2);
            PrecisionCascade(fp, "ellip_cascade_order4_cut2000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.ELLIP_CASCADE_ORDER4_CUT2000_F32_SOS,FilterCoeffs.ELLIP_CASCADE_ORDER4_CUT2000_F64_SOS,2);
            PrecisionCascade(fp, "ellip_cascade_order4_cut5000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.ELLIP_CASCADE_ORDER4_CUT5000_F32_SOS,FilterCoeffs.ELLIP_CASCADE_ORDER4_CUT5000_F64_SOS,2);
            PrecisionCascade(fp, "ellip_cascade_order6_cut1000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.ELLIP_CASCADE_ORDER6_CUT1000_F32_SOS,FilterCoeffs.ELLIP_CASCADE_ORDER6_CUT1000_F64_SOS,3);
            PrecisionCascade(fp, "ellip_cascade_order6_cut2000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.ELLIP_CASCADE_ORDER6_CUT2000_F32_SOS,FilterCoeffs.ELLIP_CASCADE_ORDER6_CUT2000_F64_SOS,3);
            PrecisionCascade(fp, "ellip_cascade_order6_cut5000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.ELLIP_CASCADE_ORDER6_CUT5000_F32_SOS,FilterCoeffs.ELLIP_CASCADE_ORDER6_CUT5000_F64_SOS,3);
            PrecisionCascade(fp, "ellip_cascade_order8_cut1000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.ELLIP_CASCADE_ORDER8_CUT1000_F32_SOS,FilterCoeffs.ELLIP_CASCADE_ORDER8_CUT1000_F64_SOS,4);
            PrecisionCascade(fp, "ellip_cascade_order8_cut2000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.ELLIP_CASCADE_ORDER8_CUT2000_F32_SOS,FilterCoeffs.ELLIP_CASCADE_ORDER8_CUT2000_F64_SOS,4);
            PrecisionCascade(fp, "ellip_cascade_order8_cut5000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.ELLIP_CASCADE_ORDER8_CUT5000_F32_SOS,FilterCoeffs.ELLIP_CASCADE_ORDER8_CUT5000_F64_SOS,4);
            PrecisionCascade(fp, "bessel_cascade_order2_cut1000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BESSEL_CASCADE_ORDER2_CUT1000_F32_SOS,FilterCoeffs.BESSEL_CASCADE_ORDER2_CUT1000_F64_SOS,1);
            PrecisionCascade(fp, "bessel_cascade_order2_cut2000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BESSEL_CASCADE_ORDER2_CUT2000_F32_SOS,FilterCoeffs.BESSEL_CASCADE_ORDER2_CUT2000_F64_SOS,1);
            PrecisionCascade(fp, "bessel_cascade_order2_cut5000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BESSEL_CASCADE_ORDER2_CUT5000_F32_SOS,FilterCoeffs.BESSEL_CASCADE_ORDER2_CUT5000_F64_SOS,1);
            PrecisionCascade(fp, "bessel_cascade_order4_cut1000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BESSEL_CASCADE_ORDER4_CUT1000_F32_SOS,FilterCoeffs.BESSEL_CASCADE_ORDER4_CUT1000_F64_SOS,2);
            PrecisionCascade(fp, "bessel_cascade_order4_cut2000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BESSEL_CASCADE_ORDER4_CUT2000_F32_SOS,FilterCoeffs.BESSEL_CASCADE_ORDER4_CUT2000_F64_SOS,2);
            PrecisionCascade(fp, "bessel_cascade_order4_cut5000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BESSEL_CASCADE_ORDER4_CUT5000_F32_SOS,FilterCoeffs.BESSEL_CASCADE_ORDER4_CUT5000_F64_SOS,2);
            PrecisionCascade(fp, "bessel_cascade_order6_cut1000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BESSEL_CASCADE_ORDER6_CUT1000_F32_SOS,FilterCoeffs.BESSEL_CASCADE_ORDER6_CUT1000_F64_SOS,3);
            PrecisionCascade(fp, "bessel_cascade_order6_cut2000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BESSEL_CASCADE_ORDER6_CUT2000_F32_SOS,FilterCoeffs.BESSEL_CASCADE_ORDER6_CUT2000_F64_SOS,3);
            PrecisionCascade(fp, "bessel_cascade_order6_cut5000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BESSEL_CASCADE_ORDER6_CUT5000_F32_SOS,FilterCoeffs.BESSEL_CASCADE_ORDER6_CUT5000_F64_SOS,3);
            PrecisionCascade(fp, "bessel_cascade_order8_cut1000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BESSEL_CASCADE_ORDER8_CUT1000_F32_SOS,FilterCoeffs.BESSEL_CASCADE_ORDER8_CUT1000_F64_SOS,4);
            PrecisionCascade(fp, "bessel_cascade_order8_cut2000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BESSEL_CASCADE_ORDER8_CUT2000_F32_SOS,FilterCoeffs.BESSEL_CASCADE_ORDER8_CUT2000_F64_SOS,4);
            PrecisionCascade(fp, "bessel_cascade_order8_cut5000_f64_sos",CASCADE::filterFloat, CASCADE::filterDouble,FilterCoeffs.BESSEL_CASCADE_ORDER8_CUT5000_F32_SOS,FilterCoeffs.BESSEL_CASCADE_ORDER8_CUT5000_F64_SOS,4);

        } catch (IOException e) {
            System.err.println("Error writing CSV: " + e.getMessage());
        }
    }
}
