import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy.fft import fft

# === PARAMETRY ===
PATH = "../_Analiza(python)/Impulse/"
FILE_1 = "impulse_response_q12.csv"
FILE_2 = "impulse_response_Float.csv"
LABEL_1 = "Q4.12"
LABEL_2 = "Float"

SAMPLING_RATE = 48000  # Hz
BASE_TITLE = "Filtr elliptyczny: 4. rząd, częstotliwość graniczna 2000Hz"

# === Wczytaj dane ===
df1 = pd.read_csv(PATH + FILE_1)
df2 = pd.read_csv(PATH + FILE_2)

signal_columns = ['DF1', 'DF2', 'TDF2', 'CASCADE']
N = len(df1)
assert len(df1) == len(df2), "Długości plików muszą być identyczne"

# === Oś częstotliwości ===
freqs = np.fft.fftfreq(N, d=1/SAMPLING_RATE)
half = freqs[:N//2]
log_mask = half >= 10  # minimum 10 Hz

colors = {
    LABEL_1: '#93c331',
    LABEL_2: '#2364aa'
}

# === Pętla po każdej strukturze ===
for col in signal_columns:
    fig, axs = plt.subplots(3, 1, figsize=(10, 8))
    fig.suptitle(f"{BASE_TITLE} - {col.upper()}", fontsize=14)

    for df, label in zip([df1, df2], [LABEL_1, LABEL_2]):
        signal = df[col].values
        fft_vals = fft(signal)
        fft_half = fft_vals[:N//2]

        amplitude = 20 * np.log10(np.abs(fft_half))
        amplitude = np.clip(amplitude, -60, 0)

        phase = np.angle(fft_half)

        # 1. Impuls
        axs[0].plot(df['sample'], signal, label=label, color=colors[label])
        axs[0].set_title("Odpowiedż impulsowa")
        axs[0].set_xlabel("Próbka")
        axs[0].set_ylabel("Wartość")
        axs[0].grid(True)

        # 2. Amplituda
        axs[1].semilogx(half[log_mask], amplitude[log_mask], label=label, color=colors[label])
        axs[1].set_title("Charakterystyka częstotliwościowa")
        axs[1].set_xlabel("Częstotliwość [Hz]")
        axs[1].set_ylabel("Amplituda [dB]")
        axs[1].set_ylim([-60, 0])
        axs[1].grid(True, which="both", ls=":")

        # 3. Faza
        axs[2].semilogx(half[log_mask], phase[log_mask], label=label, color=colors[label])
        axs[2].set_title("Charakterystyka fazowa")
        axs[2].set_xlabel("Częstotliwość [Hz]")
        axs[2].set_ylabel("Faza [rad]")
        axs[2].set_ylim([-np.pi, np.pi])
        axs[2].grid(True, which="both", ls=":")

    for ax in axs:
        ax.legend()

    plt.tight_layout(rect=[0, 0, 1, 0.96])
    plt.savefig(f"compare_{col}.png", dpi=300, transparent=True)
    plt.show()
