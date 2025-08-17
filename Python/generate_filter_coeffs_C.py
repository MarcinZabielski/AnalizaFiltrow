import numpy as np
from scipy import signal
import os

# Generowanie współczynników dla języka C
# Generowanie plików .c i .h, w których zapisywane są współczynniki filtrów
# Pliki ze wspołczynnikami: filtercoeffs.c, filtercoeffs.h w katalogu _filtercoeffs

# Parametry filtrów
fs = 48000
filter_types = ['butter', 'cheby1', 'cheby2', 'ellip', 'bessel']
orders = [2, 4, 6, 8]
cutoffs = [1000, 2000, 5000]
dtypes = [np.float64, np.float32]
structures = ['DF1', 'DF2', 'TDF2', 'CASCADE']

HEADER_PATH = "filtercoeffs.h"
SOURCE_PATH = "filtercoeffs.c"

def format_coeff(val, dtype):
    if dtype == np.float32:
        if val == 1.0:
            return "1.0f"
        elif val == -1.0:
            return "-1.0f"
        elif val == 2.0:
            return "2.0f"
        elif val == -2.0:
            return "-2.0f"
        return f"{val:.10g}f"
    else:
        if val == 1.0:
            return "1.0"
        elif val == -1.0:
            return "-1.0"
        return f"{val:.17g}"

def write_array(name, array, dtype, file):
    dims = array.shape
    array_str = f"{'float' if dtype==np.float32 else 'double'} {name}"
    array_str += "[" + "][".join(str(d) for d in dims) + "] = {\n"
    for row in array:
        array_str += "    {" + ", ".join(format_coeff(val, dtype) for val in row) + "},\n"
    array_str += "};\n\n"
    file.write(array_str)

def main():
    with open(HEADER_PATH, "w") as hfile, open(SOURCE_PATH, "w") as cfile:
        # Nagłówek H
        hfile.write("// Auto-generated filter coefficient header\n")
        hfile.write("#ifndef FILTERCOEFFS_H\n")
        hfile.write("#define FILTERCOEFFS_H\n\n")

        # Źródło C
        cfile.write("// Auto-generated filter coefficient source\n\n")
        cfile.write('#include "filtercoeffs.h"\n\n')

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
                                    write_array(name_base + "_sos", sos, dtype, cfile)
                                    hfile.write(f"extern {'float' if dtype==np.float32 else 'double'} {name_base}_sos[{sos.shape[0]}][6];\n")
                                else:
                                    ba = np.vstack((b, a))
                                    write_array(name_base + "_ba", ba, dtype, cfile)
                                    hfile.write(f"extern {'float' if dtype==np.float32 else 'double'} {name_base}_ba[2][{len(b)}];\n")

                            except Exception as e:
                                print(f"Błąd: {ftype} {order} {cutoff} {dtype} {structure}: {e}")

        hfile.write("\n#endif // FILTER_COEFFS_H\n")

if __name__ == "__main__":
    main()
