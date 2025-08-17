package filters;

public class CASCADE {
    public static void filterFloat(float[] x, float[] y, float[] sos, int N, int sections) {
        float[] temp = new float[N];
        float[] in = x;
        float[] out = temp;

        for (int s = 0; s < sections; ++s) {
            float b0 = sos[s * 6 + 0];
            float b1 = sos[s * 6 + 1];
            float b2 = sos[s * 6 + 2];
            float a0 = sos[s * 6 + 3];
            float a1 = sos[s * 6 + 4];
            float a2 = sos[s * 6 + 5];
            float w1 = 0.0f, w2 = 0.0f;

            for (int n = 0; n < N; ++n) {
                float wn = in[n] - a1 * w1 - a2 * w2;
                out[n] = b0 * wn + b1 * w1 + b2 * w2;
                w2 = w1;
                w1 = wn;
            }

            float[] tmp = in;
            in = out;
            out = tmp;
        }

        if (in != y) {
            System.arraycopy(in, 0, y, 0, N);
        }
    }

    public static void filterDouble(double[] x, double[] y, double[] sos, int N, int sections) {
        double[] temp = new double[N];
        double[] in = x;
        double[] out = temp;

        for (int s = 0; s < sections; ++s) {
            double b0 = sos[s * 6 + 0];
            double b1 = sos[s * 6 + 1];
            double b2 = sos[s * 6 + 2];
            double a0 = sos[s * 6 + 3];
            double a1 = sos[s * 6 + 4];
            double a2 = sos[s * 6 + 5];
            double w1 = 0.0, w2 = 0.0;

            for (int n = 0; n < N; ++n) {
                double wn = in[n] - a1 * w1 - a2 * w2;
                out[n] = b0 * wn + b1 * w1 + b2 * w2;
                w2 = w1;
                w1 = wn;
            }

            double[] tmp = in;
            in = out;
            out = tmp;
        }

        if (in != y) {
            System.arraycopy(in, 0, y, 0, N);
        }
    }
}
