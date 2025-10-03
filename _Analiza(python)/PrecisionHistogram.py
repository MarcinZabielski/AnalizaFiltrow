import os
import io
import numpy as np
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import streamlit as st
from scipy import stats
from fitter import Fitter

# Uruchomienie: python -m streamlit run PrecisionHistogram.py
# Zatrzymanie: Ctrl+C w terminalu

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
    if not dfs:
        return pd.DataFrame()

    return pd.concat(dfs, ignore_index=True)


path = "../_Analiza(python)/Precision/"
files = [
    path + "python_precision_results.csv",
    path + "c_floating_precision_results.csv",
    path + "c_fixed_precision_results.csv",
    path + "java_precision_results.csv",
]

data = load_data(files)

st.title("Histogram dla wektora błędów")

if data.empty:
    st.error("Nie znaleziono plików lub brak poprawnych danych.")
    st.stop()


def opts_str(series: pd.Series):
    return sorted(series.dropna().astype(str).unique())


def opts_int(series: pd.Series):
    vals = []
    for x in series.dropna().unique():
        try:
            vals.append(int(x))
        except Exception:
            pass
    return sorted(vals)

source = st.multiselect("Źródło danych (język)", opts_str(data["source_file"]))
filter_name = st.multiselect("Filter name", opts_str(data["filter_name"]))
ftype = st.multiselect("Type", opts_str(data["type"]))
structure = st.multiselect("Structure", opts_str(data["structure"]))
order = st.multiselect("Order", opts_int(data["order"]))
signal = st.multiselect("Signal", opts_str(data["signal"]))
cutoff = st.multiselect("Cutoff", sorted(data["cutoff"].dropna().unique()))

bins = st.number_input(
    "Bins", 
    min_value=3, 
    max_value=1000, 
    value=31, 
    step=1
)
bins = int(bins)

exp_clip = st.slider(
    "Przycięcie wartości w wektorze do x (10^x)", 
    min_value=0, 
    max_value=200, 
    step=1, 
    value=200
)
clip_value = 10 ** exp_clip

limit_str = st.text_input("Usuń wartości poza przedzialem [-x, x]", value=1)
try:
    limit_value = float(limit_str)
except ValueError:
    limit_value = 1e200

df = data.copy()

if source:
    df = df[df["source_file"].astype(str).isin(source)]
if filter_name:
    df = df[df["filter_name"].astype(str).isin(filter_name)]
if ftype:
    df = df[df["type"].astype(str).isin(ftype)]
if structure:
    df = df[df["structure"].astype(str).isin(structure)]
if order:
    df = df[df["order"].astype(float).isin(order)]
if signal:
    df = df[df["signal"].astype(str).isin(signal)]
if cutoff:
    try:
        cutoff_vals = [float(c) for c in cutoff]
        df = df[df["cutoff"].astype(float).isin(cutoff_vals)]
    except Exception:
        pass

if not df.empty:
    all_errors = np.concatenate([
        np.clip(vec, clip_value*(-1), clip_value) for vec in df["error_vector"].values
    ])

    all_errors = all_errors[(all_errors >= -limit_value) & (all_errors <= limit_value)]
else:
    all_errors = np.array([])
    
print(all_errors.min(), all_errors.max())

use_kde = st.checkbox("Oblicz KDE", value=False)
normalize = st.checkbox("Normalizacja histogramu (stat=density)", value=True)

if len(all_errors) == 0:
    st.warning("Brak poprawnych wartości w dataframe do histogramu.")
else:
    fig, ax = plt.subplots(figsize=(12, 7))

    try:
        sns.histplot(
            all_errors,
            stat="density" if normalize else "count",
            bins=bins,
            kde=use_kde,
            edgecolor="none",
            color="#4682B4",
            alpha=0.7,
            ax=ax
        )
    except Exception as e:
        if use_kde:
            st.warning(f"KDE nie mogło zostać obliczone (powód: {e}). Rysuję sam histogram.")
        sns.histplot(
            all_errors,
            stat="density" if normalize else "count",
            bins=bins,
            kde=False,
            edgecolor="none",
            color="#4682B4",
            alpha=0.7,
            ax=ax
        )

    ax.set_xlabel("Błąd")
    ax.set_ylabel("Liczba wystąpień" if not normalize else "Gęstość")
    ax.set_facecolor("none")
    ax.grid(True, alpha=0.3)
    st.pyplot(fig)
    st.caption(f"Liczba wartości użytych do histogramu: {len(all_errors)}")

    buf_png = io.BytesIO()
    fig.savefig(buf_png, format="png")
    buf_png.seek(0)

    buf_svg = io.BytesIO()
    fig.savefig(buf_svg, format="svg")
    buf_svg.seek(0)

    st.download_button(
        label="Pobierz PNG",
        data=buf_png,
        file_name="Precision_histogram.png",
        mime="image/png"
    )

    st.download_button(
        label="Pobierz SVG",
        data=buf_svg,
        file_name="Precision_histogram.svg",
        mime="image/svg+xml"
    )
    
    # --- średnia i wartość skuteczna ---
    st.subheader("Średnia i wartość skuteczna")
    mean_val = np.mean(all_errors)
    rms_val = np.sqrt(np.mean(all_errors**2))
    st.write(f"Średnia: {mean_val:.6f}")
    st.write(f"Wartość skuteczna: {rms_val:.6f}")
    
    # --- Analiza dopasowania do rozkładu normalnego ---
    st.subheader("Dopasowanie do rozkładu normalnego")

    # Dopasowanie parametrów μ (loc) i σ (scale) metodą MLE
    mu, sigma = stats.norm.fit(all_errors)
    st.write(f"Estymowany parametr μ (średnia): {mu:.4f}")
    st.write(f"Estymowany parametr σ (odchylenie standardowe): {sigma:.4f}")

    # --- Test Kolmogorowa-Smirnowa ---
    D, p_value = stats.kstest(all_errors, 'norm', args=(mu, sigma))
    st.write(f"Test Kołmogorowa-Smirnowa: D={D:.4f}, p-value={p_value:.4f}")
    n = len(all_errors)

    # Histogram + dopasowana krzywa PDF
    fig2, ax2 = plt.subplots(figsize=(12,7))
    sns.histplot(
        all_errors, 
        bins=bins, 
        stat="density", 
        kde=False,
        edgecolor="none",
        color="#4682B4", 
        alpha=0.7, 
        ax=ax2
    )

    x = np.linspace(all_errors.min(), all_errors.max(), 500)
    pdf = stats.norm.pdf(x, loc=mu, scale=sigma)
    ax2.plot(x, pdf, 'b-', lw=2, 
            label=f"Dopasowana funkcja gęstości rozkładu normalnego\n (μ={mu:.4f}, σ={sigma:.4f})")

    ax2.set_title("Histogram z dopasowaną funkcją gęstości rozkładu normalnego", fontsize=14)
    ax2.set_xlabel("Błąd")
    ax2.set_ylabel("Gęstość")
    ax2.grid(True, alpha=0.3)
    ax2.legend()
    st.pyplot(fig2)

    buf_svg = io.BytesIO()
    fig2.savefig(buf_svg, format="svg")
    buf_svg.seek(0)

    st.download_button(
        label="Pobierz SVG",
        data=buf_svg,
        file_name="Precision_histogram_pdf.svg",
        mime="image/svg+xml"
    )
