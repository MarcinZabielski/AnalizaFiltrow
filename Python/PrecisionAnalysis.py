import numpy as np
from scipy import signal
from scipy.signal import sosfilt, tf2sos
import csv
import warnings
from Structures import DF1, DF2, TDF2, CASCADE

warnings.simplefilter("ignore", category=RuntimeWarning)

# Analiza precyzji filtrowania w języku python

def main():
    # Parametry testu
    fs = 48000

    # Sygnały testowe
    impulse = np.zeros(4096)
    impulse[0] = 1
    np.random.seed(12345)
    random_signal = np.random.uniform(low=-1.0, high=1.0, size=4096)
    
    signals = {
        'impulse': impulse,
        'rand': random_signal
    }

    # Parametry filtrów
    filter_types = ['butter', 'cheby1', 'cheby2', 'ellip', 'bessel']
    orders = [2, 4, 6, 8]
    cutoffs = [1000, 2000, 5000]
    dtypes = [np.float32, np.float16]

    structures = {
        'DF1': DF1,
        'DF2': DF2,
        'TDF2': TDF2,
        'CASCADE': CASCADE
    }

    # Zapisz do CSV
    results_file = "python_precision_results.csv"
    with open(results_file, mode='w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(['filter_name', 'order', 'cutoff', 'structure', 'signal', 'type', 'error_vector'])

        for ftype in filter_types:
            for order in orders:
                for cutoff in cutoffs:
                    # Projektowanie filtra
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
                            b, a = signal.bessel(order, cutoff, btype='lowpass', fs=fs, output='ba', norm="phase")
                        else:
                            continue

                        sos_sections_64 = tf2sos(b, a)
                        b_64 = np.array(b, dtype=np.float64)
                        a_64 = np.array(a, dtype=np.float64)

                        for signal_name, signal_data in signals.items():
                            for struct_name, func in structures.items():
                                try:
                                    # Referencyjny sygnał na float64
                                    if struct_name == 'CASCADE':
                                        y_ref = func(signal_data.astype(np.float64), sos_sections_64)
                                    else:
                                        y_ref = func(signal_data.astype(np.float64), b_64, a_64)

                                    for dtype in dtypes:
                                        x = signal_data.astype(dtype)
                                        b_dtype = b_64.astype(dtype)
                                        a_dtype = a_64.astype(dtype)
                                        sos_dtype = [sos.astype(dtype) for sos in sos_sections_64]

                                        try:
                                            if struct_name == 'CASCADE':
                                                y = func(x, sos_dtype)
                                            else:
                                                y = func(x, b_dtype, a_dtype)

                                            error = y.astype(np.float64) - y_ref.astype(np.float64)
                                            error_str = ";".join(map(str, error.tolist()))

                                            writer.writerow([ftype, order, cutoff, struct_name, signal_name, dtype.__name__, error_str])

                                        except Exception as e:
                                            print(f" Error [{ftype}-{order}-{cutoff}Hz-{struct_name}-{dtype}]: {e}")
                                            writer.writerow([ftype, order, cutoff, struct_name, signal_name, dtype.__name__, 'ERROR'])

                                except Exception as e:
                                    print(f" Ref calc error [{ftype}-{order}-{cutoff}Hz-{struct_name}]: {e}")

                    except Exception as e:
                        print(f" Coeff gen error: {ftype}-{order}-{cutoff}: {e}")

if __name__ == "__main__":
    main()
