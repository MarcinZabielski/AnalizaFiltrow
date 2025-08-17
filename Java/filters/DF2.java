package filters;

public class DF2 {
    public static void filterFloat(float[] x, float[] y, float[] b, float[] a, int N, int order) {
        float[] w = new float[64]; // założenie: max order = 64
        for (int n = 0; n < N; ++n) {
            w[0] = x[n];
            for (int i = 1; i < order; ++i)
                w[0] -= a[i] * w[i];
            y[n] = 0.0f;
            for (int i = 0; i < order; ++i)
                y[n] += b[i] * w[i];
            for (int i = order - 1; i > 0; --i)
                w[i] = w[i - 1];
        }
    }

    public static void filterDouble(double[] x, double[] y, double[] b, double[] a, int N, int order) {
        double[] w = new double[64];
        for (int n = 0; n < N; ++n) {
            w[0] = x[n];
            for (int i = 1; i < order; ++i)
                w[0] -= a[i] * w[i];
            y[n] = 0.0;
            for (int i = 0; i < order; ++i)
                y[n] += b[i] * w[i];
            for (int i = order - 1; i > 0; --i)
                w[i] = w[i - 1];
        }
    }
}
