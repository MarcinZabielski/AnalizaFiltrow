package filters;

public class DF1 {
    public static void filterFloat(float[] x, float[] y, float[] b, float[] a, int N, int order) {
        for (int n = 0; n < N; ++n) {
            y[n] = 0.0f;
            for (int i = 0; i < order; ++i) {
                if (n - i >= 0) y[n] += b[i] * x[n - i];
                if (i > 0 && n - i >= 0) y[n] -= a[i] * y[n - i];
            }
        }
    }

    public static void filterDouble(double[] x, double[] y, double[] b, double[] a, int N, int order) {
        for (int n = 0; n < N; ++n) {
            y[n] = 0.0;
            for (int i = 0; i < order; ++i) {
                if (n - i >= 0) y[n] += b[i] * x[n - i];
                if (i > 0 && n - i >= 0) y[n] -= a[i] * y[n - i];
            }
        }
    }
}