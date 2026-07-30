import argparse
import json
import sys

from algorithms.factory import AlgorithmFactory, AlgorithmType
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
    parser.add_argument("--measurement-noise", type=float, default=None)
    parser.add_argument("--forecast-steps", type=int, default=None)
    parser.add_argument("--confidence-z", type=float, default=None)

    return parser.parse_args()

def error_result(algorithm_name: str, message: str) ->dict:
    #ten sam kształt co BaseAlgorighm, to result_dict() - potrzebny osobno
    #bo w niektorych miejsach (np. nieznany typ algorytmu) nie mamy jeszcze zadnej 
    #instancji algorytmu, wiec nie ma czym wywołac to_result_dict()
    return {
        "algorithm": algorithm_name,
        "status": "error",
        "errorMessage": message,
        "filtered": [],
        "anomalies": [],
        "forecasts": [],
        "changepoints": [],
        "metadata": {},
    }
def print_result(result: dict) ->None:
    #Cały stdout musi byc czystym JSONEM - na tym polega parsowanie
    #po stronie C++ (analysisResult::fromJson). Zadnych innych print() w tym pliku
    print (json.dumps(result))

def main() -> int:
    args = parse_arg()

    try:
        algorithm_type = AlgorithmType(args.algorithm)
    except ValueError:
        print_result(error_result(args.algorithm, f"Nieznany typ algorytmu: {args.algorithm}"))
        return 0
    algorithm_kwargs = {
        "process_noise": args.process_noise,
        "measurement_noise": args.measurement_noise,
        "forecast_steps": args.forecast_steps,
        "confidence_z": args.confidence_z,
    }
    algorithm_kwargs = {k: v for k, v in algorithm_kwargs.items() if v is not None}

    algo = AlgorithmFactory.create(algorithm_type, **algorithm_kwargs)

    try:
        df = load_file(args.file)
    except (FileNotFoundError, ValueError) as e:
        print_result(error_result(algo.name, str(e)))
        return 0
    result = algo.run(df, feature_columns=args.feature_columns, target_column=args.target_column)
    print_result(result)
    return 0

if __name__ == "__main__":
    sys.exit(main())

    

    