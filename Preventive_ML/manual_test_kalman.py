import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

from algorithms.kalman_filter import KalmanFilterAlgorithm
from data_loader import load_file


def plot_kalman_result(value_column: str, result: dict, true_value=None):
    """
    Rysuje wykres pokazujący:
    - surowe (zaszumione) dane historyczne (kropki)
    - to, co model wyestymował dla danych historycznych (linia "filtered")
    - prognozę do przodu wraz z pasmem ufności (linia + zacieniowany obszar)
    - opcjonalnie: prawdziwą wartość (tylko gdy testujemy na sztucznych danych)
    """
    filtered = result["filtered"]
    forecasts = result["forecasts"]

    t_hist = [p["timestamp"] for p in filtered]
    raw_hist = [p["value"] for p in filtered]
    pred_hist = [p["predicted"] for p in filtered]

    t_fore = [p["timestamp"] for p in forecasts]
    pred_fore = [p["predicted"] for p in forecasts]
    upper_fore = [p["upper"] for p in forecasts]
    lower_fore = [p["lower"] for p in forecasts]

    plt.figure(figsize=(10, 6))

    # Surowe dane historyczne - kropki, zeby widac bylo szum
    plt.scatter(t_hist, raw_hist, s=15, color="gray", alpha=0.5, label="Surowe dane (pomiar)")

    # To co model wyestymowal na podstawie historii
    plt.plot(t_hist, pred_hist, color="tab:blue", linewidth=2, label="Estymacja Kalmana (filtered)")

    # Prognoza w przyszlosc
    plt.plot(t_fore, pred_fore, color="tab:orange", linewidth=2, linestyle="--", label="Prognoza (forecast)")

    # Pasmo ufnosci prognozy - fill_between rysuje zacieniowany obszar miedzy dwiema liniami
    plt.fill_between(t_fore, lower_fore, upper_fore, color="tab:orange", alpha=0.2, label="Przedzial ufnosci (95%)")

    # Opcjonalnie: prawdziwa wartosc (znamy ja tylko bo to SZTUCZNE dane testowe)
    if true_value is not None:
        all_t = t_hist + t_fore
        plt.plot(all_t, true_value, color="black", linewidth=1, linestyle=":", label="Prawdziwa wartosc (znana tylko w teście)")

    plt.xlabel("Czas")
    plt.ylabel(value_column)
    plt.title("Filtr Kalmana - estymacja i prognoza")
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.tight_layout()
    plt.show()


# Wczytujemy PRAWDZIWE dane z pliku CSV zamiast generowac sztuczne.
# Sciezka wzgledna - uruchamiamy skrypt z katalogu Preventive_ML/,
# a plik lezy jeden poziom wyzej, w glownym katalogu projektu.
df = load_file("../temperature_data.csv")
print("Kolumny po normalizacji:", list(df.columns))


df["timestamp"] = df["timestamp"] / 1000.0


value_column = "temperature_c"

forecast_steps = 20



algo = KalmanFilterAlgorithm(process_noise=1e-3, measurement_noise=1.0, forecast_steps=forecast_steps)
result = algo.run(df, feature_columns=[value_column])

print("status:", result["status"])
print("metadata:", result["metadata"])

# Brak "prawdziwej" wartosci - to realne dane, wiec nie ma z czym porownywac poza samym pomiarem
plot_kalman_result(value_column, result)