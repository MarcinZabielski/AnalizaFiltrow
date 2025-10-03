import os
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy import stats

LIMIT = 1e155
ALPHA = 0.001
OUT_DIR = "precision_histograms"

RYSUNKI = False
if RYSUNKI:
    os.makedirs(OUT_DIR, exist_ok=True)

def load_data(files):
    dfs = []
    for f in files:
        if os.path.exists(f):
            df = pd.read_csv(
                f,
                dtype=str,
                na_values=["nan", "NaN", "NAN", "inf", "-inf", "Infinity", "-Infinity", "-nan(ind)", "nan(ind)"]
            )
            df["source_file"] = os.path.basename(f)

            if "error_vector" in df.columns:
                def parse_vector(x):
                    if pd.isna(x) or x.strip() == "":
                        return np.array([])
                    try:
                        arr = np.array([float(v) for v in x.split(";") if v.strip() != ""])
                        return arr
                    except Exception:
                        return np.array([])
                df["error_vector"] = df["error_vector"].apply(parse_vector)
            dfs.append(df)
    return pd.concat(dfs, ignore_index=True) if dfs else pd.DataFrame()


def clean_errors(error_vectors, limit=1.0):
    all_err = np.concatenate(error_vectors)
    return all_err[(all_err >= -limit) & (all_err <= limit)]


def best_distribution(errors, alpha=0.05):
    dists = ['norm']
    best_name = "brak dopasowania"
    best_p = -1
    best_params = None

    for dist_name in dists:
        try:
            params = getattr(stats, dist_name).fit(errors)
            D, p = stats.kstest(errors, dist_name, args=params)
            if p > best_p:
                best_p = p
                best_name = dist_name
                best_params = params
        except Exception:
            continue

    if best_p < alpha:
        return "brak dopasowania", best_p, None
    return best_name, best_p, best_params

 
path = "../_Analiza(python)/Precision/"
files = [
    path + "python_precision_results.csv",
    path + "c_floating_precision_results.csv",
    path + "c_fixed_precision_results.csv",
    path + "java_precision_results.csv",
]

data = load_data(files)
if data.empty:
    print("Brak danych.")
    exit()

data = data[data["signal"] == "impulse"]

group_cols = ["source_file", "filter_name", "type", "structure", "order", "cutoff", "signal"]
results = []
summary_counts = {}

for key, group in data.groupby(group_cols):
    
    errors = clean_errors(group["error_vector"].values, limit=LIMIT)
    
    mean_val = np.mean(errors)
    rms_val = np.sqrt(np.mean(errors**2))

    best_name, best_p, best_params = best_distribution(errors, alpha=ALPHA)
    print(f"Znaleziono dopasowanie do: {best_name} (grupa={key}, p={best_p:.4f}), Średnia: {mean_val:.6f}, Wartość skuteczna (RMS): {rms_val:.6f}")
    
    fname=""

    if RYSUNKI:
        plt.figure(figsize=(7,5))
        plt.hist(errors, bins=31, density=True, color="#4682B4", alpha=0.7, label="Histogram")

        if best_name != "brak dopasowania" and best_params is not None:
            x = np.linspace(errors.min(), errors.max(), 500)
            pdf = getattr(stats, best_name).pdf(x, *best_params)
            plt.plot(x, pdf, lw=2, label=f"{best_name} (p={best_p:.3f})")

        plt.title(f"{key}")
        plt.xlabel("Błąd")
        plt.ylabel("Gęstość")
        plt.legend()
        plt.grid(True, alpha=0.3)

        fname = os.path.join(OUT_DIR, "hist_" + "_".join(map(str, key)) + ".png")
        plt.savefig(fname, dpi=150)
        plt.close()

    results.append({
        "source_file": key[0],
        "filter_name": key[1],
        "type": key[2],
        "structure": key[3],
        "order": key[4],
        "cutoff": key[5],
        "best_fit": best_name,
        "p_value": best_p,
        "mean": mean_val,
        "rms": rms_val,
        "n_samples": len(errors),
        "hist_file": fname
    })

    summary_counts[best_name] = summary_counts.get(best_name, 0) + 1

df_results = pd.DataFrame(results)
df_results.to_csv("PrecisionDistributionsResults.csv", index=False)

print("\n--- PODSUMOWANIE ---")
for dist_name, count in summary_counts.items():
    print(f"{dist_name}: {count} filtrów")
