import argparse
import json
import sys

from algorithms.factory import AlgorithmFactory, AlgorithmType
from data_loader import load_file, load_and_merge_files

def parse_arg():
    parser = argparse.ArgumentParser(
        description="Uruchamia wybrany algorytm ML na danych historycznych i zwraca wynik jako JSON."
    )
    parser.add_argument("--file",action="append", required=True, help="Ścieżka do pliku z danymi historycznym. Podaj kilka razy "
                        "zeby połaczyć osobne eksporty (np rozne tagi PIvision) w jeden zestaw danych).")
    parser.add_argument("--source-column", action="append", default= None,
                        help="Nazwa kolumny wartosci w danym pliku (w tej samej kolejnosci co " \
                        "--file).") 
    parser.add_argument("--target-column", action="append", default=None,
                        help="Nazwa pod jaka kolumna wartosci z danego pliku ma trafic do " \
                        "polaczonych danych (w tej samej kolejnosci co --file). Wymagane tylko gdy podano" \
                        "wiecej niz jeden --file.")
    parser.add_argument("--algorithm", required=True, help="Typ algorytmu, np. 'kalman'.")
    parser.add_argument("--feature-columns", nargs="+", required=True,
                        help="Kolumny wejściowe algorytmu (jedna lub wiecej).")
    parser.add_argument("--target-column-name", default=None,
                        help="Kolumna, która ma przewidzieć algorytm (opcjonalnie).")

    # Parametry specyficzne dla filtra kalmana - domyślnieNone, zeby w razie braku
    # podania uzyc wartości domyślnych z konstruktora KalmanFilterAlgorithm.
    # Gdy dojdzie kolejny algorytm (RF/EKF), dojda tu jego wlasne argumenty.

    parser.add_argument("--process-noise", type=float, default=None)
    parser.add_argument("--measurement-noise", type=float, default=None)
    parser.add_argument("--forecast-steps", type=int, default=None)
    parser.add_argument("--confidence-z", type=float, default=None)

    #parametry specyficzne dla EKF zaworu

    parser.add_argument("--cv-max", type=float, default=None)
    parser.add_argument("--specific-gravity", type=float, default=None)
    parser.add_argument("--valve-characteristic", type=str, default=None,
                        choices=["linear", "equal_percentage"])
    parser.add_argument("--rangeability", type=float, default=None)
       

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
        "cv_max":args.cv_max,
        "specific_gravity": args.specific_gravity,
        "valve_characteristic": args.valve_characteristic,
        "rangeability": args.rangeability,
    }
    algorithm_kwargs = {k: v for k, v in algorithm_kwargs.items() if v is not None}

    algo = AlgorithmFactory.create(algorithm_type, **algorithm_kwargs)

    try:
        if len(args.file) == 1:
            df = load_file(args.file[0])
        else:
            if not args.source_column or not args.target_column \
                    or len(args.source_column) != len(args.file) \
                    or len(args.target_column) != len(args.file):
                print_result(error_result(algo.name,
                                          "Przy kilku plikach (--file podanym kilka razy) trzeba podac tyle samo "
                                          "--source-column i --target-column, ile jest plikow. ",
                ))
                return 0
            sources = list(zip(args.file, args.source_column, args.target_column))
            df = load_and_merge_files(sources)

    except (FileNotFoundError, ValueError) as e:
        print_result(error_result(algo.name, str(e)))
        return 0
    result = algo.run(df, feature_columns=args.feature_columns, 
                      target_column=args.target_column_name)
    print_result(result)
    return 0

if __name__ == "__main__":
    sys.exit(main())

    

    