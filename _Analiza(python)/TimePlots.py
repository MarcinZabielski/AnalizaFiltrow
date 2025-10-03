import pandas as pd
import matplotlib.pyplot as plt
import re
import os

# Wczytaj plik CSV
df = pd.read_csv("./Time/TimeDistributionsResults2.csv")

# Filtrowanie tylko Weibull i wybrane rzędy
df = df[(df["best_fit"] == "weibull_min") & (df["order"].isin([2, 4, 6, 8]))].copy()

# Funkcja do parsowania parametrów (c = k, loc, scale)
def parse_params(param_str):
    if pd.isna(param_str):
        return None, None, None
    match = re.findall(r"([\w]+)\s+([-\d\.eE]+)", param_str)
    params = {k: float(v) for k, v in match}
    return params.get("c", None), params.get("loc", None), params.get("scale", None)

# Rozbij parametry na 3 kolumny
df[["k", "loc", "scale"]] = df["fit_params"].apply(
    lambda x: pd.Series(parse_params(x))
)

# Utwórz folder na wykresy
output_dir = "weibull_plots_scatter"
os.makedirs(output_dir, exist_ok=True)

# Grupowanie po konfiguracji
group_cols = ["source_file", "type", "structure"]
for config, group in df.groupby(group_cols):
    source, dtype, structure = config

    # Sortuj po rzędzie
    group = group.sort_values("order")

    # Rysowanie scatter plotów dla k, loc, scale
    plt.figure(figsize=(10, 6))
    plt.scatter(group["order"], group["k"], label="k", marker="o", s=140)
    plt.scatter(group["order"], group["loc"], label="loc", marker="s", s=140)
    plt.scatter(group["order"], group["scale"], label="scale", marker="^", s=140)

    plt.xlabel("Rząd filtru")
    plt.ylabel("Wartości parametrów")
    plt.title(f"Parametry Weibulla\n{source}, {dtype}, {structure}")
    plt.legend()
    plt.grid(True)

    # Oś X: tylko wartości dostępne w danych
    orders = group["order"].unique()
    plt.xticks(orders)
    plt.xlim(min(orders) - 1, max(orders) + 1)

    # Zapisz do pliku
    filename = f"{source}_{dtype}_{structure}.svg".replace("/", "_")
    plt.savefig(os.path.join(output_dir, filename))
    plt.close()

print(f"Wykresy zapisano w folderze: {output_dir}")
