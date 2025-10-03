import os
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy import stats

ALPHA = 0.01
OUT_DIR = "time_histograms"

RYSUNKI = False
if RYSUNKI:
    os.makedirs(OUT_DIR, exist_ok=True)

def load_data(files):
    dfs = []
    for f in files:
        if os.path.exists(f):
            df = pd.read_csv(f, dtype=str)
            df["source_file"] = os.path.basename(f)
            dfs.append(df)
    return pd.concat(dfs, ignore_index=True) if dfs else pd.DataFrame()


def best_distribution(times, alpha=0.05):
    dists = ['weibull_min']
    best_name = "brak dopasowania"
    best_p = -1
    best_params = None

    for dist_name in dists:
        try:
            params = getattr(stats, dist_name).fit(times)
            D, p = stats.kstest(times, dist_name, args=params)
            if p > best_p:
                best_p = p
                best_name = dist_name
                best_params = params
        except Exception:
            continue

    if best_p < alpha:
        return "brak dopasowania", best_p, None
    return best_name, best_p, best_params

path = "../_Analiza(python)/Time/"
files = [
    path+"python_time_results.csv",
    path+"c_floating_time_results.csv",
    path+"c_fixed_time_results.csv",
    path+"java_time_results.csv"
]

data = load_data(files)
if data.empty:
    print("Brak danych.")
    exit()

group_cols = ["source_file", "type", "structure", "order"]
results = []
summary_counts = {}

for key, group in data.groupby(group_cols):

    times = group["time_seconds"].astype(float).values
    
    eps = 1e-9 
    times = times + np.random.normal(0, eps, size=len(times))

    best_name, best_p, best_params = best_distribution(times, alpha=ALPHA)
    print(f"Znaleziono dopasowanie do: {best_name} (grupa={key}, p={best_p:.4f})")
    
    if best_name == "weibull_min" and best_params is not None:
        params_str = f"c {best_params[0]:.6f}, loc {best_params[1]:.6f}, scale {best_params[2]:.6f}"
    elif best_name == "rayleigh" and best_params is not None:
        params_str = f"loc {best_params[0]:.6f}, scale {best_params[1]:.6f}, "
    else:
        params_str = ""

    fname = ""
    if RYSUNKI:
        plt.figure(figsize=(7,5))
        plt.hist(times, bins=10, density=True, color="#4682B4", alpha=0.7, label="Histogram")
        if best_name != "brak dopasowania" and best_params is not None:
            x = np.linspace(times.min(), times.max(), 500)
            pdf = getattr(stats, best_name).pdf(x, *best_params)
            plt.plot(x, pdf, lw=2, label=f"{best_name} (p={best_p:.3f})")
        plt.title(f"{key}")
        plt.xlabel("Czas [s]")
        plt.ylabel("Gęstość")
        plt.legend()
        plt.grid(True, alpha=0.3)
        fname = os.path.join(OUT_DIR, "hist_" + "_".join(map(str, key)) + ".png")
        plt.savefig(fname, dpi=150)
        plt.close()

    results.append({
        "source_file": key[0],
        #"filter_name": key[1],
        "type": key[1],
        "structure": key[2],
        "order": key[3],
        #"cutoff": key[5],
        "best_fit": best_name,
        "p_value": best_p,
        "n_samples": len(times),
        "fit_params": params_str,
        "hist_file": fname
    })
    
    summary_counts[best_name] = summary_counts.get(best_name, 0) + 1

df_results = pd.DataFrame(results)
df_results.to_csv("TimeDistributionsResults.csv", index=False)

print("\n--- PODSUMOWANIE ---")
for dist_name, count in summary_counts.items():
    print(f"{dist_name}: {count} filtrów")
