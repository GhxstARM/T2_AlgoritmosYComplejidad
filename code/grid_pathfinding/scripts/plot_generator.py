"""
Script para generar gráficos a partir de las mediciones experimentales.
Lee los resultados desde data/measurements/ y guarda los gráficos en data/plots/.

Métricas y figuras generadas:
  1. 01_tiempo_vs_tamano.png: Tiempo de ejecución vs tamaño (N * M) en escala log-log
     comparando los 3 paradigmas (Fuerza Bruta, DP y Greedy).
  2. 01b_tiempo_vs_tamano_por_tipo.png: Tiempo de ejecución vs tamaño desglosado por
     cada tipo de grilla (libre, murallas, minas, mixta).
  3. 02_memoria_vs_tamano.png: Uso de memoria (KB) vs tamaño (N * M) en escala log-log
     para los 3 paradigmas.
  4. 03_brecha_vs_tamano.png: Brecha de puntaje Delta = s(OPT) - s(Greedy) vs tamaño
     diferenciada por dominios D10 y D1000 y tipos de grilla.
  5. 04_tasa_falla_greedy.png: Tasa de falla de Greedy (% de casos con 'SIN SOLUCION')
     según tipo de grilla y dominio.
  6. 04b_tasa_falla_vs_tamano.png: Tasa de falla de Greedy en función del tamaño N * M.
"""

import os
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# Directorios de datos y salida
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
MEASUREMENTS_DIR = os.path.normpath(os.path.join(BASE_DIR, "..", "data", "measurements"))
PLOTS_DIR = os.path.normpath(os.path.join(BASE_DIR, "..", "data", "plots"))

# Paleta de colores estándar y accesible
COLORES = {
    "bruteforce": "#E64A19",   
    "dp": "#1976D2",           
    "greedy": "#388E3C",       
    "libre": "#2E7D32",
    "murallas": "#455A64",
    "minas": "#C2185B",
    "mixta": "#7B1FA2"
}

NOMBRES_ALG = {
    "bruteforce": "Fuerza Bruta",
    "dp": "Prog. Dinámica (DP)",
    "greedy": "Greedy (Voraz)"
}


def configurar_estilo():
    """Configura el estilo visual general de las figuras para informes LaTeX."""
    plt.rcParams.update({
        "font.size": 11,
        "axes.labelsize": 12,
        "axes.titlesize": 13,
        "xtick.labelsize": 10,
        "ytick.labelsize": 10,
        "legend.fontsize": 10,
        "figure.titlesize": 14,
        "grid.alpha": 0.4,
        "grid.linestyle": "--"
    })


def cargar_datos():
    """Carga los CSVs de mediciones con validación."""
    results_path = os.path.join(MEASUREMENTS_DIR, "results.csv")
    summary_path = os.path.join(MEASUREMENTS_DIR, "summary_instances.csv")

    if not os.path.exists(results_path) or not os.path.exists(summary_path):
        raise FileNotFoundError(
            f"No se encontraron los archivos de mediciones en {MEASUREMENTS_DIR}.\n"
            f"Asegúrese de ejecutar el programa 'grid_pathfinding' previamente."
        )

    df_results = pd.read_csv(results_path)
    df_summary = pd.read_csv(summary_path)

    return df_results, df_summary


def plot_tiempo_vs_tamano(df_results):
    """
    Gráfico 1: Tiempo promedio de ejecución vs tamaño (N * M) en escala logarítmica
    para los tres algoritmos.
    """
    fig, ax = plt.subplots(figsize=(8.5, 5.5))

    # Agrupar por algoritmo y tamaño para obtener tiempo promedio
    df_mean = df_results.groupby(["algorithm", "size"])["time_ms"].mean().reset_index()

    for alg in ["bruteforce", "dp", "greedy"]:
        sub = df_mean[df_mean["algorithm"] == alg].sort_values("size")
        if sub.empty:
            continue
        ax.plot(
            sub["size"],
            sub["time_ms"],
            marker="o",
            linewidth=2,
            markersize=6,
            label=NOMBRES_ALG.get(alg, alg),
            color=COLORES.get(alg, "black")
        )

    ax.set_xscale("log", base=2)
    ax.set_yscale("log")
    ax.set_xlabel("Tamaño de la Grilla $N \\times M$ (número total de celdas)")
    ax.set_ylabel("Tiempo promedio de ejecución (ms, escala log)")
    ax.set_title("Tiempo de Ejecución vs. Tamaño de Grilla ($N \\times M$)")
    ax.grid(True)
    ax.legend(loc="upper left")

    # Anotación explicativa de la cota de Fuerza Bruta
    ax.axvline(x=256, color="#D32F2F", linestyle=":", alpha=0.7)
    ax.text(
        260, 1e-2,
        r"Límite $N+M \leq 32$" + "\n(Fuerza Bruta se detiene)",
        color="#D32F2F",
        fontsize=9,
        bbox=dict(boxstyle="round,pad=0.3", fc="white", ec="#D32F2F", alpha=0.8)
    )

    plt.tight_layout()
    out_file = os.path.join(PLOTS_DIR, "01_tiempo_vs_tamano.png")
    plt.savefig(out_file, dpi=300)
    plt.close()
    print(f"Generado: {out_file}")


def plot_tiempo_por_tipo(df_results):
    """
    Gráfico 1b: Tiempo promedio de ejecución vs tamaño (N * M)
    en 4 subplots, uno por cada tipo de grilla.
    """
    tipos = ["libre", "murallas", "minas", "mixta"]
    nombres_tipo = {
        "libre": "Grillas Libres (100% transitables)",
        "murallas": "Grillas con Murallas",
        "minas": "Grillas con Minas",
        "mixta": "Grillas Mixtas (Murallas y Minas)"
    }

    fig, axes = plt.subplots(2, 2, figsize=(12, 9), sharex=True, sharey=True)
    axes = axes.flatten()

    for idx, tipo in enumerate(tipos):
        ax = axes[idx]
        df_tipo = df_results[df_results["grid_type"] == tipo]
        df_mean = df_tipo.groupby(["algorithm", "size"])["time_ms"].mean().reset_index()

        for alg in ["bruteforce", "dp", "greedy"]:
            sub = df_mean[df_mean["algorithm"] == alg].sort_values("size")
            if sub.empty:
                continue
            ax.plot(
                sub["size"],
                sub["time_ms"],
                marker="o",
                linewidth=1.8,
                markersize=5,
                label=NOMBRES_ALG.get(alg, alg),
                color=COLORES.get(alg, "black")
            )

        ax.set_xscale("log", base=2)
        ax.set_yscale("log")
        ax.set_title(nombres_tipo[tipo])
        ax.grid(True)
        if idx % 2 == 0:
            ax.set_ylabel("Tiempo promedio (ms)")
        if idx >= 2:
            ax.set_xlabel("Tamaño $N \\times M$")
        if idx == 0:
            ax.legend(loc="upper left")

    fig.suptitle("Tiempo de Ejecución por Tipo de Terreno ($N \\times M$)", fontsize=14)
    plt.tight_layout()
    out_file = os.path.join(PLOTS_DIR, "01b_tiempo_vs_tamano_por_tipo.png")
    plt.savefig(out_file, dpi=300)
    plt.close()
    print(f"Generado: {out_file}")


def plot_memoria_vs_tamano(df_results):
    """
    Gráfico 2: Uso de memoria (KB) vs tamaño (N * M) en escala logarítmica.
    Muestra la diferencia entre O(N*M*V) de DP y O(N+M) de Greedy/BF.
    """
    fig, ax = plt.subplots(figsize=(8.5, 5.5))

    df_mean = df_results.groupby(["algorithm", "size"])["memory_kb"].mean().reset_index()

    for alg in ["bruteforce", "dp", "greedy"]:
        sub = df_mean[df_mean["algorithm"] == alg].sort_values("size")
        if sub.empty:
            continue
        cota = "O(N × M × V)" if alg == "dp" else "O(N + M)"
        ax.plot(
            sub["size"],
            sub["memory_kb"],
            marker="s",
            linewidth=2,
            markersize=6,
            label=f"{NOMBRES_ALG.get(alg, alg)} - {cota}",
            color=COLORES.get(alg, "black")
        )

    ax.set_xscale("log", base=2)
    ax.set_yscale("log")
    ax.set_xlabel(r"Tamaño de la Grilla $N \times M$ (número total de celdas)")
    ax.set_ylabel("Consumo medido de memoria (KB, pico de heap, escala log)")
    ax.set_title(r"Consumo de Memoria vs. Tamaño de Grilla ($N \times M$)")
    ax.grid(True)
    ax.legend(loc="upper left")

    plt.tight_layout()
    out_file = os.path.join(PLOTS_DIR, "02_memoria_vs_tamano.png")
    plt.savefig(out_file, dpi=300)
    plt.close()
    print(f"Generado: {out_file}")


def plot_brecha_vs_tamano(df_summary):
    """
    Gráfico 3: Brecha Delta = s(OPT) - s(Greedy) vs tamaño de grilla.
    Dividido en dos paneles (D10 y D1000) debido a la escala de dos órdenes de magnitud.
    Solo considera casos donde Greedy encontró solución.
    """
    # Filtrar solo casos donde greedy encontró solución
    df_valid = df_summary[df_summary["greedy_found"] == 1].copy()

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5.5), sharex=True)

    paneles = [("D10", ax1, "Dominio D10 (Valores en [-10, 10], P = 10)"),
               ("D1000", ax2, "Dominio D1000 (Valores en [-1000, 1000], P = 1000)")]

    tipos = ["libre", "murallas", "minas", "mixta"]

    for dom, ax, titulo in paneles:
        df_dom = df_valid[df_valid["domain"] == dom]
        for tipo in tipos:
            sub = df_dom[df_dom["grid_type"] == tipo]
            if sub.empty:
                continue
            mean_gap = sub.groupby("size")["delta"].mean().reset_index().sort_values("size")
            ax.plot(
                mean_gap["size"],
                mean_gap["delta"],
                marker="o",
                linewidth=1.8,
                markersize=5,
                label=tipo.capitalize(),
                color=COLORES.get(tipo, "black")
            )

        ax.set_xscale("log", base=2)
        ax.set_xlabel("Tamaño $N \\times M$")
        ax.set_ylabel("Brecha Promedio $\\Delta = s(OPT) - s(Greedy)$")
        ax.set_title(titulo)
        ax.grid(True)
        ax.legend(loc="upper left")

    fig.suptitle("Brecha de Puntaje $\\Delta$ de la Solución Greedy vs. Tamaño de Grilla", fontsize=14)
    plt.tight_layout()
    out_file = os.path.join(PLOTS_DIR, "03_brecha_vs_tamano.png")
    plt.savefig(out_file, dpi=300)
    plt.close()
    print(f"Generado: {out_file}")


def plot_tasa_falla_greedy(df_summary):
    """
    Gráfico 4: Tasa de falla de Greedy (% 'SIN SOLUCION') por tipo de grilla y dominio.
    Gráfico de barras agrupadas.
    """
    # Calcular tasa de fallas (%)
    df_fails = df_summary.groupby(["grid_type", "domain"])["greedy_found"].agg(
        total="count",
        fails=lambda x: (x == 0).sum(),
        rate=lambda x: 100.0 * (x == 0).mean()
    ).reset_index()

    tipos = ["libre", "murallas", "minas", "mixta"]
    x = np.arange(len(tipos))
    width = 0.35

    fig, ax = plt.subplots(figsize=(8.5, 5.5))

    rates_d10 = []
    rates_d1000 = []

    for tipo in tipos:
        r10 = df_fails[(df_fails["grid_type"] == tipo) & (df_fails["domain"] == "D10")]
        rates_d10.append(r10["rate"].values[0] if not r10.empty else 0.0)

        r1000 = df_fails[(df_fails["grid_type"] == tipo) & (df_fails["domain"] == "D1000")]
        rates_d1000.append(r1000["rate"].values[0] if not r1000.empty else 0.0)

    rects1 = ax.bar(x - width / 2, rates_d10, width, label="Dominio D10", color="#1976D2")
    rects2 = ax.bar(x + width / 2, rates_d1000, width, label="Dominio D1000", color="#FF8F00")

    ax.set_ylabel("Tasa de Falla (%) [Caminos SIN SOLUCION]")
    ax.set_title("Tasa de Falla de la Heurística Greedy por Tipo de Terreno y Dominio")
    ax.set_xticks(x)
    ax.set_xticklabels([t.capitalize() for t in tipos])
    ax.set_ylim(0, 45)
    ax.grid(axis="y")
    ax.legend(loc="upper right")

    # Añadir valores sobre las barras
    def autolabel(rects):
        for rect in rects:
            height = rect.get_height()
            ax.annotate(
                f"{height:.1f}%",
                xy=(rect.get_x() + rect.get_width() / 2, height),
                xytext=(0, 3),
                textcoords="offset points",
                ha="center", va="bottom",
                fontsize=9
            )

    autolabel(rects1)
    autolabel(rects2)

    plt.tight_layout()
    out_file = os.path.join(PLOTS_DIR, "04_tasa_falla_greedy.png")
    plt.savefig(out_file, dpi=300)
    plt.close()
    print(f"Generado: {out_file}")


def plot_tasa_falla_vs_tamano(df_summary):
    """
    Gráfico 4b: Evolución de la tasa de falla de Greedy en función del tamaño N * M
    para los tipos de terreno con obstáculos (murallas y mixta).
    """
    df_obst = df_summary[df_summary["grid_type"].isin(["murallas", "mixta"])].copy()

    df_tam = df_obst.groupby(["grid_type", "size"])["greedy_found"].agg(
        rate=lambda x: 100.0 * (x == 0).mean()
    ).reset_index()

    fig, ax = plt.subplots(figsize=(8.5, 5.0))

    for tipo in ["murallas", "mixta"]:
        sub = df_tam[df_tam["grid_type"] == tipo].sort_values("size")
        ax.plot(
            sub["size"],
            sub["rate"],
            marker="o",
            linewidth=2,
            markersize=6,
            label=f"Grillas {tipo.capitalize()}",
            color=COLORES.get(tipo, "black")
        )

    ax.set_xscale("log", base=2)
    ax.set_xlabel("Tamaño de la Grilla $N \\times M$")
    ax.set_ylabel("Tasa de Falla (%)")
    ax.set_title("Evolución de la Tasa de Falla de Greedy según el Tamaño de Grilla")
    ax.grid(True)
    ax.set_ylim(-2, 70)
    ax.legend(loc="upper left")

    plt.tight_layout()
    out_file = os.path.join(PLOTS_DIR, "04b_tasa_falla_vs_tamano.png")
    plt.savefig(out_file, dpi=300)
    plt.close()
    print(f"Generado: {out_file}")


def main():
    os.makedirs(PLOTS_DIR, exist_ok=True)
    configurar_estilo()

    print("Cargando datos desde:", MEASUREMENTS_DIR)
    df_results, df_summary = cargar_datos()

    print(f"Total registros cargados: {len(df_results)} en results.csv, {len(df_summary)} en summary_instances.csv")
    print("Generando figuras en:", PLOTS_DIR)

    plot_tiempo_vs_tamano(df_results)
    plot_tiempo_por_tipo(df_results)
    plot_memoria_vs_tamano(df_results)
    plot_brecha_vs_tamano(df_summary)
    plot_tasa_falla_greedy(df_summary)
    plot_tasa_falla_vs_tamano(df_summary)

    print("\nTodos los gráficos han sido generados exitosamente en:", PLOTS_DIR)


if __name__ == "__main__":
    main()
