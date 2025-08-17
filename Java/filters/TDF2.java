package filters;

public class TDF2 {
    public static void filterFloat(float[] x, float[] y, float[] b, float[] a, int N, int order) {
        float[] w = new float[64];
        for (int n = 0; n < N; ++n) {
            float yn = w[0] + b[0] * x[n];
            for (int i = 0; i < order - 2; ++i) {
                w[i] = w[i + 1] + b[i + 1] * x[n] - a[i + 1] * yn;
            }
            w[order - 2] = b[order - 1] * x[n] - a[order - 1] * yn;
            y[n] = yn;
        }
    }

    public static void filterDouble(double[] x, double[] y, double[] b, double[] a, int N, int order) {
        double[] w = new double[64];
        for (int n = 0; n < N; ++n) {
            double yn = w[0] + b[0] * x[n];
            for (int i = 0; i < order - 2; ++i) {
                w[i] = w[i + 1] + b[i + 1] * x[n] - a[i + 1] * yn;
            }
            w[order - 2] = b[order - 1] * x[n] - a[order - 1] * yn;
            y[n] = yn;
        }
    }
}
