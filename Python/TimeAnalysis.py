import numpy as np
from scipy import signal
from scipy.signal import sosfilt, tf2sos, lfilter, normalize
import time
import csv
import warnings
from Structures import DF1, DF2, TDF2, CASCADE

warnings.simplefilter("ignore", category=RuntimeWarning)

# Analiza szybkości filtrowania w języku python

def main():
    # Parametry testu
    fs = 48000
    duration_sec = 10 * 60  # 10 minut
    x_full = np.random.uniform(low=-1.0, high=1.0, size=fs * duration_sec)  # sygnał testowy (biały szum)

    # Parametry filtrów
    filter_types = ['butter', 'cheby1', 'cheby2', 'ellip', 'bessel']
    orders = [2, 4, 6, 8]
    cutoffs = [1000, 2000, 5000]
    dtypes = [np.float64, np.float32, np.float16]
    structures = {
        'DF1': DF1,
        'DF2': DF2,
        'TDF2': TDF2,
        'CASCADE': CASCADE
    }

    # Zapisz do CSV
    results_file = "python_time_results.csv"
    with open(results_file, mode="w", newline='') as f:
        writer = csv.writer(f)
        writer.writerow(['filter_type', 'order', 'cutoff', 'dtype', 'structure', 'time_sec'])

        for ftype in filter_types:
            for order in orders:
                for cutoff in cutoffs:
                    for dtype in dtypes:
                        x = x_full.astype(dtype)
                        # Projektowanie filtra
                        try:
                            if ftype == 'butter':
                                b, a = signal.butter(order, cutoff, btype='lowpass', fs=fs, output='ba', analog=False)
                            elif ftype == 'cheby1':
                                b, a = signal.cheby1(order, 1, cutoff, btype='lowpass', fs=fs, output='ba', analog=False)
                            elif ftype == 'cheby2':
                                b, a = signal.cheby2(order, 2, cutoff, btype='lowpass', fs=fs, output='ba', analog=False)
                            elif ftype == 'ellip':
                                b, a = signal.ellip(order, 1, 20, cutoff, btype='lowpass', fs=fs, output='ba', analog=False)
                            elif ftype == 'bessel':
                                b, a = signal.bessel(order, cutoff, btype='lowpass', fs=fs, output='ba', analog=False, norm="phase")
                            else:
                                continue

                            sos_sections = tf2sos(b, a)
                            b = b.astype(dtype)
                            a = a.astype(dtype)
                            sos_sections = [sos.astype(dtype) for sos in sos_sections]

                            for name, func in structures.items():
                                try:
                                    t0 = time.perf_counter()
                                    if name == 'CASCADE':
                                        y = func(x, sos_sections)
                                    else:
                                        y = func(x, b, a)
                                    t1 = time.perf_counter()
                                    time_sec = round(t1 - t0, 6)
                                    print(time_sec)
                                    writer.writerow([ftype, order, cutoff, dtype.__name__, name, time_sec])
                                except Exception as e:
                                    print(f" Błąd: {ftype}, order {order}, cutoff {cutoff}, {dtype}, {name}: {e}")

                        except Exception as e:
                            print(f" Błąd: {ftype}, order {order}, cutoff {cutoff}, {dtype}, {name}: {e}")
    
if __name__ == "__main__":
    main()