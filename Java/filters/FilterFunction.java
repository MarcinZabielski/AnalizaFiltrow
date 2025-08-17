package filters;

public class FilterFunction {
    @FunctionalInterface
    public interface FloatFilter {
        void apply(float[] x, float[] y, float[] b, float[] a, int N, int order);
    }

    @FunctionalInterface
    public interface DoubleFilter {
        void apply(double[] x, double[] y, double[] b, double[] a, int N, int order);
    }

    @FunctionalInterface
    public interface FloatCascade {
        void apply(float[] x, float[] y, float[] sos, int N, int sections);
    }

    @FunctionalInterface
    public interface DoubleCascade {
        void apply(double[] x, double[] y, double[] sos, int N, int sections);
    }
}
