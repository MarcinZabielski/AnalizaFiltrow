import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Wczytaj dane z CSV
precision_data = pd.read_csv('../_Analiza/python_precision_results.csv')  # dane precyzji
time_data = pd.read_csv('../_Analiza/python_time_results.csv')          # dane szybkości

# Ustaw styl
sns.set(style="whitegrid")

# Stwórz 2 wykresy obok siebie
fig, axes = plt.subplots(1, 3, figsize=(16, 6))

# Wykres precyzji
# Średni błąd bezwzględny (MAE)
# typ vs MAE
sns.barplot(data=precision_data, x="dtype", y="MAE", ax=axes[0])
axes[0].set_title('Precyzja filtrów')
axes[0].set_xlabel('Typ')
axes[0].set_ylabel('Średni błąd bezwzględny')

# styp vs MSE
sns.barplot(data=precision_data, x="dtype", y="MSE", ax=axes[1])
axes[1].set_title('Precyzja filtrów')
axes[1].set_xlabel('Typ')
axes[1].set_ylabel('Błąd średniokwadratowy')

# Wykres szybkości
sns.barplot(data=time_data, x="dtype", y="time_sec", ax=axes[2])
axes[2].set_title('Szybkość filtrów')
axes[2].set_xlabel('Typ')
axes[2].set_ylabel('Czas filtrowania (s)')

plt.tight_layout()
plt.show()
