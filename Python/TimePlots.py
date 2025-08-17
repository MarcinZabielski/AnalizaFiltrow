import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
from matplotlib import font_manager
import statsmodels.api as sm
from statsmodels.formula.api import ols
from statsmodels.graphics.factorplots import interaction_plot
from matplotlib.ticker import FuncFormatter, LogLocator

# Analiza statystyczna szybkości filtrowania

# === 1. Wczytywanie danych ===
df_python = pd.read_csv("../_Analiza(python)/Time/python_time_results.csv")
df_c_fixed = pd.read_csv("../_Analiza(python)/Time/c_fixed_time_results.csv")
df_c_floating = pd.read_csv("../_Analiza(python)/Time/c_floating_time_results.csv")
df_java = pd.read_csv("../_Analiza(python)/Time/java_time_results.csv")

# Dodaj kolumny z nazwą languagea
df_python["language"] = "Python"
df_c_fixed["language"] = "C_fixed"
df_c_floating["language"] = "C_floating"
df_java["language"] = "Java"

# Połączenie danych
df = pd.concat([df_python, df_c_fixed, df_c_floating, df_java], ignore_index=True)

# zmiana nazw
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

# Sprawdzenie kolumn
print("Kolumny dostępne w danych:", df.columns)
print(df.head())

# === 2. Czyszczenie danych ===
df = df.dropna()

# Konwersja na string (jeśli np. order jest floatem przez przypadek)
df["order"] = df["order"].astype(str)
df["cutoff"] = df["cutoff"].astype(str)

# paleta kolorów
palette = {
    "Python": '#93c331',
    "C (stałopozycyjne)": '#3da5d9',
    "C (zmiennopozycyjne)": '#2364aa',
    "Java": '#bb342f'
}

# === 3. Boxplot: typ filtra vs język ===
plt.figure(figsize=(12, 6))
bx = sns.boxplot(x="filter_name", y="time_seconds", hue="language", data=df, palette=palette, saturation=1.0)
plt.yscale("log")
plt.gca().yaxis.set_major_formatter(FuncFormatter(lambda y, _: '{:g}'.format(y)))
plt.title("Czas filtrowania a typ filtra i język implementacji", fontsize=18)
plt.ylabel("Czas [s]", fontsize=12)
plt.xlabel("Typ filtra", fontsize=12)
plt.legend(title="Język")
plt.xticks(rotation=45)

grouped = df.groupby(['filter_name', 'language'])['time_seconds'].median().reset_index()

for i, row in grouped.iterrows():
    # Znajdź gdzie jest słupek (x)
    x = list(df['filter_name'].unique()).index(row['filter_name']) + {
        'Python': -0.3,
        'C (stałopozycyjne)': -0.1,
        'C (zmiennopozycyjne)': 0.1,
        'Java': 0.3,
    }[row['language']]  # przesunięcie na boxplocie hue

    bx.annotate(f"{row['time_seconds']:.1f}",
                xy=(x, row['time_seconds']),
                xytext=(0, 5),
                textcoords="offset points",
                ha='center',
                va='bottom',
                fontsize=9,
                color='black')

plt.tight_layout()
plt.savefig("boxplot_typ_filtra.png", dpi=300, transparent=True)
plt.show()

# === 4. Heatmapa: rząd vs struktura ===
pivot = df.pivot_table(values='time_seconds', index='order', columns='structure', aggfunc='mean')
plt.figure(figsize=(10, 6))
sns.heatmap(pivot, annot=True, fmt=".3f", cmap="viridis")
plt.title("Średni czas filtrowania a rząd i struktura", fontsize=18)
plt.ylabel("Czas [s]", fontsize=12)
plt.xlabel("Struktura", fontsize=12)
plt.tight_layout()
plt.savefig("heatmap_rzad_struktura.png", dpi=300, transparent=True)
plt.show()


# === 5. Analiza wariancji (ANOVA) ===
model = ols('time_seconds ~ C(language) + C(filter_name) + C(order) + C(structure) + C(type) + C(cutoff)', data=df).fit()
anova_table = sm.stats.anova_lm(model, typ=2)

# Podświetl istotne wartości p < 0.05
print("\n=== ANOVA ===")
anova_table_rounded = anova_table.copy()
anova_table_rounded["PR(>F)"] = anova_table_rounded["PR(>F)"].apply(lambda x: f"{x:.2e}")
print(anova_table_rounded)

# Sortowanie według F-statystyki
anova_table_sorted = anova_table.sort_values(by="F", ascending=False)
anova_table_sorted = anova_table_sorted.dropna()

# Mapa nazw kategorii (czytelniejsze nazwy)
pretty_names = {
    "C(language)": "Język",
    "C(filter_name)": "Typ filtra",
    "C(order)": "Rząd filtra",
    "C(structure)": "Struktura",
    "C(type)": "Reprezentacja num.",
    "C(cutoff)": "Częstotliwość odcięcia"
}

# Zamiana indeksów na ładne opisy
anova_table_sorted.index = [pretty_names.get(i, i) for i in anova_table_sorted.index]

# Tworzenie wykresu
plt.figure(figsize=(10, 5))
colors = sns.color_palette("viridis", len(anova_table_sorted))  # żywsze kolory
sns.barplot(
    x=anova_table_sorted.index,
    y=anova_table_sorted["F"],
    palette=colors
)

plt.ylabel("Wartość statystyki F", fontsize=12)
plt.xlabel("Czynnik", fontsize=12)
plt.title("Wpływ czynników na czas filtrowania (analiza wariancji)", fontsize=14)
plt.xticks(rotation=45)

plt.tight_layout()
plt.savefig("anova_f_stat_ranking_custom.png", dpi=300, transparent=True)
plt.show()

# === 6. Interaction plot ===

# Dopasuj liczbę markerów/kolorów do liczby języków
languages = df["language"].unique()
num_langs = len(languages)

# Więcej markerów i kolorów na wszelki wypadek
markers = ['o', 's', 'D', '^', 'v', 'x', '+', '*', 'p', 'h'][:num_langs]
colors = sns.color_palette("Set1", n_colors=num_langs)

plt.figure(figsize=(10, 6))
interaction_plot(df['filter_name'], df['language'], df['time_seconds'],
                 markers=markers, colors=colors)
plt.title("Interakcja: Typ filtra w zależności od języka implementacji i czasu")
plt.tight_layout()
plt.savefig("interaction_plot_fixed.png", dpi=300, transparent=True)
plt.show()

# === 7. Dodatkowe: Boxplot type ===
plt.figure(figsize=(10, 6))
bx = sns.boxplot(x="type", y="time_seconds", hue="language", data=df, palette=palette, saturation=1.0)
plt.yscale("log")
plt.gca().yaxis.set_major_formatter(FuncFormatter(lambda y, _: '{:g}'.format(y)))
plt.title("Czas filtrowania a reprezentacja numeryczna i język")
plt.ylabel("Czas [s]", fontsize=12)
plt.xlabel("Reprezentacja numeryczna", fontsize=12)
plt.legend(title="Język")

grouped = df.groupby(['type', 'language'])['time_seconds'].median().reset_index()

for i, row in grouped.iterrows():
    # Znajdź gdzie jest słupek (x)
    x = list(df['type'].unique()).index(row['type']) + {
        'Python': -0.3,
        'C (stałopozycyjne)': -0.1,
        'C (zmiennopozycyjne)': 0.1,
        'Java': 0.3,
    }[row['language']]  # przesunięcie na boxplocie hue

    bx.annotate(f"{row['time_seconds']:.1f}",
                xy=(x, row['time_seconds']),
                xytext=(0, 5),
                textcoords="offset points",
                ha='center',
                va='bottom',
                fontsize=9,
                color='black')

plt.tight_layout()
plt.savefig("boxplot_repr.png", dpi=300, transparent=True)
plt.show()
