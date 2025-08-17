import numpy as np
from scipy import signal
import os

# Generowanie współczynników dla języka Java
# Generowanie pliku .java, w którym zapisywane są współczynniki filtrów
# Plik ze wspołczynnikami: FilterCoeffs.java w katalogu Java/coeffs

fs = 48000
filter_types = ['butter', 'cheby1', 'cheby2', 'ellip', 'bessel']
orders = [2, 4, 6, 8]
cutoffs = [1000, 2000, 5000]
dtypes = [np.float64, np.float32]
structures = ['DF1', 'DF2', 'TDF2', 'CASCADE']

JAVA_PATH = "../Java/filters/FilterCoeffs.java"
PACKAGE_NAME = "filters"

def format_coeff(val, dtype):
    if dtype == np.float32:
        if val in [1.0, -1.0, 2.0, -2.0]:
            return f"{val:.1f}f"
        return f"{val:.10g}f"
    else:
        if val in [1.0, -1.0]:
            return f"{val:.1f}"
        return f"{val:.17g}"

def java_array(array, dtype):
    rows = []
    for row in array:
        line = ", ".join(format_coeff(val, dtype) for val in row)
        rows.append("        {" + line + "}")
    return "{\n" + ",\n".join(rows) + "\n    };"

def write_java_class(coeff_dict, path, package):
    with open(path, "w") as f:
        f.write("// Auto-generated Java filter coefficient class\n")
        f.write(f"package {package};\n\n")
        f.write("public class FilterCoeffs {\n\n")

        for name, (dtype, array) in coeff_dict.items():
            java_type = "float" if dtype == np.float32 else "double"
            f.write(f"    public static final {java_type}[][] {name.upper()} = ")
            f.write(java_array(array, dtype))
            f.write("\n\n")

        f.write("}\n")

def main():
    coeffs = {}

    for ftype in filter_types:
        for order in orders:
            for cutoff in cutoffs:
                for dtype in dtypes:
                    for structure in structures:
                        try:
                            if ftype == 'butter':
                                b, a = signal.butter(order, cutoff, btype='lowpass', fs=fs, output='ba')
                            elif ftype == 'cheby1':
                                b, a = signal.cheby1(order, 1, cutoff, btype='lowpass', fs=fs, output='ba')
                            elif ftype == 'cheby2':
                                b, a = signal.cheby2(order, 2, cutoff, btype='lowpass', fs=fs, output='ba')
                            elif ftype == 'ellip':
                                b, a = signal.ellip(order, 1, 20, cutoff, btype='lowpass', fs=fs, output='ba')
                            elif ftype == 'bessel':
                                b, a = signal.bessel(order, cutoff, btype='lowpass', fs=fs, norm='phase', output='ba')
                            else:
                                continue

                            b = b.astype(dtype)
                            a = a.astype(dtype)

                            suffix = 'f32' if dtype == np.float32 else 'f64'
                            name_base = f"{ftype}_{structure.lower()}_order{order}_cut{cutoff}_{suffix}"

                            if structure == 'CASCADE':
                                sos = signal.tf2sos(b, a).astype(dtype)
                                coeffs[name_base + "_sos"] = (dtype, sos)
                            else:
                                ba = np.vstack((b, a))
                                coeffs[name_base + "_ba"] = (dtype, ba)

                        except Exception as e:
                            print(f"Błąd: {ftype} {order} {cutoff} {dtype} {structure}: {e}")

    write_java_class(coeffs, JAVA_PATH, PACKAGE_NAME)

if __name__ == "__main__":
    main()
