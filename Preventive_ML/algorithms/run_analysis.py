import argparse
import json
import sys

from algorithms.factory import AlgorithmFactory
from data_loader import load_file

def parse_arg():
    parser = argparse.ArgumentParser(
        description="Uruchamia wybrany algorytm ML na danych historycznych i zwraca wynik jako JSON."
    )
    parser.add_argument("--file", required=True, help="Ścieżka do pliku CSV/XLS z danymi historycznymi.")
    parser.add_argument("--algorithm", required=True, help="Typ algorytmu, np. 'kalman'.")
    parser.add_argument("--feature-columns", nargs="+", required=True,
                        help="Kolumny wejściowe algorytmu (jedna lub wiecej).")
    parser.add_argument("--target-column", default=None,
                        help="Kolumna, która ma przewidzieć algorytm (opcjonalnie).")

    # Parametry specyficzne dla filtra kalmana - domyślnieNone, zeby w razie braku
    # podania uzyc wartości domyślnych z konstruktora KalmanFilterAlgorithm.
    # Gdy dojdzie kolejny algorytm (RF/EKF), dojda tu jego wlasne argumenty.

    parser.add_argument("--process-noise", type=float, default=None)
    parser.add_argument("--measure-noise", type=float, default=None)
    parser.add_argument("forecast-steps", type=int, default=None)
    parser.add_argument("--confidence-z", type=float, default=None)

    return parser.parse_args()

def error_result(algorithm_name: str, message: str) ->dict:
    