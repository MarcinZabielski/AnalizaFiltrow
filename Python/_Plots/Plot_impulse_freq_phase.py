import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy.fft import fft

# === PARAMETRY ===
PATH = "../_Analiza(python)/Impulse/"
FILE = "impulse_response_q24.csv"
FILEPATH = PATH+FILE         # np. "data.csv"
SAMPLING_RATE = 48000              # Hz
BASE_TITLE = "Filtr Eliptyczny Rząd 8. Częstotliwość graniczna 5000Hz"

# === Wczytaj dane ===
df = pd.read_csv(FILEPATH)
signal_columns = ['DF1', 'DF2', 'TDF2', 'CASCADE']
N = len(df)

# === Oś częstotliwości ===
freqs = np.fft.fftfreq(N, d=1/SAMPLING_RATE)
half = freqs[:N//2]
log_mask = half >= 10  # minimum 10 Hz

# === Pętla po każdej strukturze ===
for col in signal_columns:
    signal = df[col].values
    fft_vals = fft(signal)
    fft_half = fft_vals[:N//2]
    
    # Amplituda w dB
    amplitude = 20 * np.log10(np.abs(fft_half))
    amplitude = np.clip(amplitude, -60, 0)  # ogranicz do -60 dB
    
    # Faza w radianach
    phase = np.angle(fft_half)

    # — WYKRESY —
    fig, axs = plt.subplots(3, 1, figsize=(10, 10))
    fig.suptitle(f"{BASE_TITLE} - {col.upper()}", fontsize=14)

    # 1. Wartości próbek
    axs[0].plot(df['sample'], signal, color='black')
    axs[0].set_title("Signal")
    axs[0].set_xlabel("Sample")
    axs[0].set_ylabel("Value")
    axs[0].grid(True)

    # 2. Frequency Response (Amplituda)
    axs[1].semilogx(half[log_mask], amplitude[log_mask], color='blue')
    axs[1].set_title("Frequency Response")
    axs[1].set_xlabel("Frequency [Hz]")
    axs[1].set_ylabel("Amplitude [dB]")
    axs[1].set_ylim([-60, 0])
    axs[1].grid(True, which="both", ls=":")

    # 3. Phase Response
    axs[2].semilogx(half[log_mask], phase[log_mask], color='red')
    axs[2].set_title("Phase Response")
    axs[2].set_xlabel("Frequency [Hz]")
    axs[2].set_ylabel("Phase [rad]")
    axs[2].set_ylim([-np.pi, np.pi])
    axs[2].grid(True, which="both", ls=":")

    plt.tight_layout(rect=[0, 0, 1, 0.96])
    plt.savefig(f"plot_{col}.png")
    plt.show()
