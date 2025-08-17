import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter
from statsmodels.formula.api import ols
import statsmodels.api as sm
from statsmodels.graphics.factorplots import interaction_plot

# Analiza statystyczna precyzji filtrowania

# === 1. Wczytywanie danych ===
df_python = pd.read_csv("../_Analiza(python)/Precision/python_precision_results.csv", na_values=["nan", "NaN", "-nan(ind)", "inf", "-inf"])
df_c_fixed = pd.read_csv("../_Analiza(python)/Precision/c_fixed_precision_results.csv")
df_c_floating = pd.read_csv("../_Analiza(python)/Precision/c_floating_precision_results.csv", na_values=["nan", "NaN", "-nan(ind)", "inf", "-inf"])
df_java = pd.read_csv("../_Analiza(python)/Precision/java_precision_results.csv", na_values=["nan", "NaN", "-nan(ind)", "inf", "-inf"])

df_python["language"] = "Python"
df_c_fixed["language"] = "C_fixed"
df_c_floating["language"] = "C_floating"
df_java["language"] = "Java"

df = pd.concat([df_python, df_c_fixed, df_c_floating, df_java], ignore_index=True)

# === 2. Czyszczenie i zamiana nazw ===
df = df.dropna()

type_map = {
    "float16": "np.float16",
    "float32": "np.float32",
    "float64": "np.float64",
    "q12": "Q4.12",
    "q24": "Q8.24",
    "float": "float (32-bit)",
    "double": "double (64-bit)"
}
df["type"] = df["type"].replace(type_map)

name_map = {
    "butter": "Butterworth",
    "cheby1": "Czebyszew I",
    "cheby2": "Czebyszew II",
    "ellip": "Eliptyczny",
    "bessel": "Bessel",
}
df["filter_name"] = df["filter_name"].replace(name_map)

lang_map = {
    "Python": "Python",
    "C_fixed": "C (stałopozycyjne)",
    "C_floating": "C (zmiennopozycyjne)",
    "Java": "Java"
}
df["language"] = df["language"].replace(lang_map)

df["order"] = df["order"].astype(str)
df["cutoff"] = df["cutoff"].astype(str)

palette = {
    'Butterworth': '#93c331',
    'Czebyszew I': '#2364aa',
    'Czebyszew II': '#3da5d9',
    'Eliptyczny': '#bb342f',
    'Bessel': '#ea7317'
}

# Usuwamy wiersze z brakującymi danymi wyjściowymi
df = df.dropna(subset=["mae", "mse"])
df = df[df["type"] != "np.float64"]

df2 = df.copy()
df2["mae_clipped"] = df2["mae"].clip(upper=10000)

# === 3. Boxplot ===
plt.figure(figsize=(12, 6))

bx = sns.boxplot(
    x="order",
    y="mae_clipped",
    hue="filter_name",
    data=df2,
    palette=palette,  # możesz też dać własną paletę
    saturation=1.0,
)

plt.yscale("log")
plt.gca().yaxis.set_major_formatter(FuncFormatter(lambda y, _: '{:g}'.format(y)))
plt.title("Średni błąd bezwzględny a rząd filtra i typ filtra", fontsize=16)
plt.ylabel("Średni błąd bezwzględny", fontsize=12)
plt.xlabel("Rząd filtra", fontsize=12)
plt.legend(title="Typ filtra")
plt.xticks(rotation=0)

# Anotacje – mediany
grouped = df2.groupby(['order', 'filter_name'])['mae_clipped'].median().reset_index()

for i, row in grouped.iterrows():
    x = list(df['order'].unique()).index(row['order']) + {
        'Butterworth': -0.3,
        'Czebyszew I': -0.15,
        'Czebyszew II': 0.0,
        'Eliptyczny': 0.15,
        'Bessel': 0.3
    }.get(row['filter_name'], 0)

    bx.annotate(f"{row['mae_clipped']:.1e}",
                xy=(x, row['mae_clipped']),
                xytext=(0, 5),
                textcoords="offset points",
                ha='center',
                va='bottom',
                fontsize=9,
                color='black')

plt.tight_layout()
plt.savefig("mae_boxplot_order_filter.png", dpi=300, transparent=True)
plt.show()


# === 4. Heatmapa: Mediana MAE vs rząd i struktura ===
pivot = df2.pivot_table(values='mae_clipped', index='order', columns='structure', aggfunc='median')
cmap = sns.color_palette("Greens", as_cmap=True)
plt.figure(figsize=(10, 6))
sns.heatmap(pivot, annot=True, fmt=".8f", cmap=cmap, linewidths=0.5, linecolor='gray')
plt.title("Mediana ze średniego błędu bezwzględnego a rząd i struktura", fontsize=16)
plt.ylabel("Rząd", fontsize=12)
plt.xlabel("Struktura", fontsize=12)
plt.tight_layout()
plt.savefig("heatmap_mae_median_order_structure.png", dpi=300, transparent=True)
plt.show()

# === 5. ANOVA ===
model = ols('mae_clipped ~ C(language) + C(filter_name) + C(order) + C(structure) + C(type) + C(cutoff)', data=df2).fit()
anova_table = sm.stats.anova_lm(model, typ=2)

anova_table_rounded = anova_table.copy()
anova_table_rounded["PR(>F)"] = anova_table_rounded["PR(>F)"].apply(lambda x: f"{x:.2e}")
print("\n=== ANOVA (MAE) ===")
print(anova_table_rounded)

anova_table_sorted = anova_table.sort_values(by="F", ascending=False).dropna()

pretty_names = {
    "C(language)": "Język",
    "C(filter_name)": "Typ filtra",
    "C(order)": "Rząd filtra",
    "C(structure)": "Struktura",
    "C(type)": "Reprezentacja num.",
    "C(cutoff)": "Częstotliwość odcięcia"
}
anova_table_sorted.index = [pretty_names.get(i, i) for i in anova_table_sorted.index]

plt.figure(figsize=(10, 5))
sns.barplot(x=anova_table_sorted.index, y=anova_table_sorted["F"], palette="viridis")
plt.ylabel("Statystyka F", fontsize=12)
plt.xlabel("Czynnik", fontsize=12)
plt.title("Wpływ czynników na średni błąd bezwzględny (analiza wariancji)", fontsize=14)
plt.xticks(rotation=45)
plt.tight_layout()
plt.savefig("anova_mae_ranking.png", dpi=300, transparent=True)
plt.show()


# === Interaction plot ===
# Upewnij się, że order jest stringiem (dla czytelnej osi X)
df2['order'] = df2['order'].astype(str)

# Reset indeksu w każdej kolumnie przekazywanej do interaction_plot
x = df2['order'].reset_index(drop=True)
trace = df2['type'].reset_index(drop=True)
response = df2['mae_clipped'].reset_index(drop=True)

# Przygotuj kolory i markery
markers = ['o', 's', 'D', '^', 'v', 'x', '+', '*'][:df2['type'].nunique()]
colors = sns.color_palette("Set2", n_colors=df2['type'].nunique())

# Rysowanie wykresu interakcji
plt.figure(figsize=(10, 6))
interaction_plot(
    x,
    trace,
    response,
    markers=markers,
    colors=colors,
    ms=6
)

plt.yscale("log")
plt.ylabel("Średni błąd bezwzględny", fontsize=8)
plt.xlabel("Rząd filtra", fontsize=8)
plt.title("Interakcja: Rząd filtra vs reprezentacja numeryczna vs Średni błąd bezwzględny", fontsize=10)
plt.grid(True)
plt.tight_layout()
plt.savefig("interaction_order_type_mae.png", dpi=300, transparent=True)
plt.show()