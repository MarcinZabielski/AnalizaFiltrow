import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import os

CSV_FILE = "./Precision/PrecisionDistributionsResults.csv"
OUT_DIR = "rms_hist_plots"

os.makedirs(OUT_DIR, exist_ok=True)

df = pd.read_csv(CSV_FILE)

source_file_alias = {
    "c_fixed_precision_results.csv": "C (stałopozycyjne)",
    "c_floating_precision_results.csv": "C (zmiennopozycyjne)",
    "java_precision_results.csv": "Java",
    "python_precision_results.csv": "Python"
}

structure_alias = {
    "DF1": "DF I",
    "DF2": "DF II",
    "TDF2": "TDF II",
    "CASCADE": "Cascade"
}

filter_name_alias = {
    "butter": "Butterworth",
    "cheby1": "Chebyshev I",
    "cheby2": "Chebyshev II",
    "ellip": "Eliptyczny",
    "bessel": "Bessel"
}

type_alias = {
    "q12": "Q4.12",
    "q24": "Q8.24",
    "float": "float",
    "float16": "np.float16",
    "float32": "np.float32"
}

column_alias = {
    "cutoff": "Częstotliwość graniczna",
    "structure": "Struktura",
    "filter_name": "Rodzaj filtru",
    "order": "Rząd",
    "type": "Typ zmiennej",
    "source_file": "Język"
}

df['source_file'] = df['source_file'].map(source_file_alias).fillna(df['source_file'])
df['structure'] = df['structure'].map(structure_alias).fillna(df['structure'])
df['filter_name'] = df['filter_name'].map(filter_name_alias).fillna(df['filter_name'])
df['type'] = df['type'].map(type_alias).fillna(df['type'])

df['rms'] = df['rms'].clip(upper=1.0)

def rms_to_dbfs(rms, full_scale=1.0):
    rms = np.maximum(rms, 1e-12)
    return 20 * np.log10(rms / full_scale)

df['rms_dbfs'] = rms_to_dbfs(df['rms'], full_scale=1.0)

x_columns = ['source_file', 'filter_name', 'type', 'structure', 'order', 'cutoff']

for col in x_columns:
    groups = df[col].unique()
    fig, axes = plt.subplots(len(groups), 1, figsize=(8, 2.3*len(groups)), sharex=None)

    if len(groups) == 1:
        axes = [axes]

    for ax, grp in zip(axes, groups):
        data = df[df[col] == grp]["rms_dbfs"]
        sns.histplot(
            data,
            bins=100,
            stat="density",
            edgecolor="none",
            color="#4682B4",
            alpha=0.7,
            ax=ax
        )

        if col == "cutoff":
            ax.set_title(f"{grp} Hz")
        else:
            ax.set_title(str(grp))

        ax.set_ylabel("Gęstość")
        ax.set_xlabel("Wartość skuteczna [dBFS]")
        ax.grid(True)

    plt.tight_layout()
    output_file = os.path.join(OUT_DIR, f"hist_{col}.svg")
    plt.savefig(output_file)
    plt.close()

print(f"Histogramy zapisane w folderze '{OUT_DIR}'")
