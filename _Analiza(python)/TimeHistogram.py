import os
import io
import numpy as np
import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import streamlit as st
import scipy.stats as stats
from fitter import Fitter

# Uruchomienie: python -m streamlit run TimeHistogram.py
# Zatrzymanie: Ctrl+C w terminalu

def load_data(files):
    dfs = []
    for f in files:
        if os.path.exists(f):
            df = pd.read_csv(f)
            df["source_file"] = os.path.basename(f)
            dfs.append(df)
    return pd.concat(dfs, ignore_index=True)

path = "../_Analiza(python)/Time/"

files = [
    path+"python_time_results.csv",
    path+"c_floating_time_results.csv",
    path+"c_fixed_time_results.csv",
    path+"java_time_results.csv"
]

data = load_data(files)

st.title("Histogram czasu filtrowania")

source = st.multiselect("Źródło danych (język)", sorted(data["source_file"].unique()))
filter_name = st.multiselect("Filter name", sorted(data["filter_name"].unique()))
ftype = st.multiselect("Type", sorted(data["type"].unique()))
structure = st.multiselect("Structure", sorted(data["structure"].unique()))
cutoff = st.multiselect("Cutoff", sorted(map(str, data["cutoff"].unique())))
order = st.multiselect("Order", sorted(map(str, data["order"].unique())))

bins = st.slider("Bins", min_value=5, max_value=100, value=10, step=1)

df = data.copy()

if source:
    df = df[df["source_file"].isin(source)]
if filter_name:
    df = df[df["filter_name"].isin(filter_name)]
if ftype:
    df = df[df["type"].isin(ftype)]
if structure:
    df = df[df["structure"].isin(structure)]
if cutoff:
    df = df[df["cutoff"].isin(map(int, cutoff))]
if order:
    df = df[df["order"].isin(map(int, order))]

times = df["time_seconds"].values

eps = 1e-9
times = times + np.random.normal(0, eps, size=len(times))

if df.empty:
    st.warning("Brak danych dla wybranych filtrów")
else:
    fig, ax = plt.subplots(figsize=(12,7))
    sns.histplot(
        times,
        bins=bins,
        kde=True,
        edgecolor="none",
        color="#4682B4",
        alpha=0.7,
        ax=ax
    )
    ax.set_title("Histogram czasów filtrowania wektora losowych wartości przez filtr", fontsize=14)
    ax.set_xlabel("Czas filtrowania [s]")
    ax.set_ylabel("Liczebność")
    ax.set_facecolor("none")
    ax.grid(True, alpha=0.3)
    st.pyplot(fig)
    st.caption(f"Liczba wartości użytych do histogramu: {len(df)}")
    
    buf_png = io.BytesIO()
    fig.savefig(buf_png, format="png")
    buf_png.seek(0)

    buf_svg = io.BytesIO()
    fig.savefig(buf_svg, format="svg")
    buf_svg.seek(0)
    
    st.download_button(
        label="Pobierz PNG",
        data=buf_png,
        file_name="Time_histogram.png",
        mime="image/png"
    )

    st.download_button(
        label="Pobierz SVG",
        data=buf_svg,
        file_name="Time_histogram.svg",
        mime="image/svg+xml"
    )
    
    # --- Analiza dopasowania do rozkładu Rayleigha ---
    if len(times) > 0:
        
        st.subheader("Dopasowanie rozkładu Weibull'a")

        # Dopasowanie MLE do weibull_min
        c, loc, scale = stats.weibull_min.fit(times)
        st.write(f"Parametry Weibull: c={c:.6f}, loc={loc:.6f}, scale={scale:.6f}")

        # Test Kolmogorova-Smirnova
        D, p_value = stats.kstest(times, 'weibull_min', args=(c, loc, scale))
        st.write(f"Test Kołmogorowa-Smirnowa: D = {D:.4f}, p-value = {p_value:.6f}")

        # Histogram + PDF
        fig_weib, ax_weib = plt.subplots(figsize=(12,7))
        sns.histplot(times, bins=bins, stat="density", kde=False, edgecolor="none", color="#4682B4", alpha=0.7, ax=ax_weib)
        x = np.linspace(times.min(), times.max(), 500)
        pdf = stats.weibull_min.pdf(x, c, loc, scale)
        ax_weib.plot(x, pdf, 'b-', lw=2, label=f"Dopasowana funkcja gęstości rozkładu Weibull'a \n (k={c:.4f}, loc={loc:.4f}, scale={scale:.4f})")
        ax_weib.set_xlabel("Czas filtrowania [s]")
        ax_weib.set_ylabel("Gęstość")
        ax_weib.set_title("Histogram z dopasowanym rozkładem Weibull")
        ax_weib.grid(True, alpha=0.3)
        ax_weib.legend()
        st.pyplot(fig_weib)
        
        buf_svg = io.BytesIO()
        fig_weib.savefig(buf_svg, format="svg")
        buf_svg.seek(0)
        
        st.download_button(
            label="Pobierz SVG",
            data=buf_svg,
            file_name="Time_histogram_pdf_weibull.svg",
            mime="image/svg+xml"
        )
        
        # --- Fitter: porównanie rozkładów ---
        st.subheader("Porównanie dopasowania różnych rozkładów")
        
        # Slider do wyboru N najlepszych
        n_best = st.slider("Pokaż najlepsze N dopasowań", min_value=1, max_value=5, value=5)
        
        with st.spinner("Dopasowywanie rozkładów..."):
            f = Fitter(times, distributions=['rayleigh','cauchy','laplace', 'weibull_min'])
            f.fit()
            
            # Ranking najlepszych N
            best = f.summary(Nbest=n_best)
            st.dataframe(best)
            
            # Checkbox histogram
            show_hist = st.checkbox("Pokaż histogram razem z dopasowanymi rozkładami", value=True)
            
            # Wykres
            fig4, ax4 = plt.subplots(figsize=(12,7))
            if show_hist:
                ax4.hist(times, bins=bins, density=True, color="#4682B4", edgecolor="none", alpha=0.5, label="Histogram")
            
            best_distributions = best.index.tolist()
            x = np.linspace(times.min(), times.max(), 500)
            for dist_name in best_distributions:
                params = f.fitted_param[dist_name]
                pdf = getattr(stats, dist_name).pdf(x, *params)
                ax4.plot(x, pdf, lw=2, label=dist_name)
            
            ax4.set_xlabel("Czas filtrowania [s]")
            ax4.set_ylabel("Gęstość")
            ax4.set_title(f"Porównanie najlepszych {n_best} dopasowań rozkładów")
            ax4.grid(True, alpha=0.3)
            ax4.legend()
            st.pyplot(fig4)
            
            buf_svg4 = io.BytesIO()
            fig4.savefig(buf_svg4, format="svg")
            buf_svg4.seek(0)
            st.download_button("Pobierz SVG", data=buf_svg4,
            file_name="Precision_Histogram_Fitter.svg", mime="image/svg+xml")