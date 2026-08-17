import os               # moduł do operacji na ścieżkach plików
import pandas as pd     #biblioteka do pracy z danymi tabelarycznymi (podobne jak excel w pythonie) pd to skrot

TIME_KEYWORDS = ["time", "timestamp", "czas", "t", "time_ms"]
VALUE_KEYWORDS = ["value", "signal", "wartość", "val", "v",
                  "temperature", "temperature_c",
                  "pressure", "pressure_bar",
                  "flow", "flow_m3h",
                  "position", "position_pct"]

def load_file(file_path: str) -> pd.DataFrame:

    #sprawdzenie czy plik istnieje
    if not os.path.exists(file_path):
        raise FileNotFoundError(f"Plik nie istnieje: {file_path}")  # raise to obsluga wyjatkow w tym przypadku zlapanie ze nie ma takiego pliku

    #rozpoznanie formatu po rozszerzeniu
    extension = os.path.splitext(file_path)[1].lower()

    if extension == ".csv":
        df = _load_csv(file_path)       #wywolanie funkcji do csv
    elif extension in [".xlsx", ".xls"]:
        df = _load_xlsx(file_path)
    else:
        raise ValueError(f"Nieobsługiwany format pliku: {extension}")

    df = _normalize_columns(df)
    df = _convert_timestamp_column(df)

    return df

#wczytuje plik csv z automatyczna detekcją separatora
def _load_csv (file_path: str) -> pd.DataFrame:

    #otwarcie pliku w sposob bezpieczny
    with open(file_path, "r", encoding="utf-8-sig") as f:
        first_line = f.readline()

    #zliczanie wystąpienia każdego separatora w pierwszej linii
    sep_counts = {
        ";": first_line.count(";"),
        ",": first_line.count(","),
        "\t": first_line.count("\t")
    }

    separator = max(sep_counts, key=sep_counts.get)

    if sep_counts[separator] == 0:
        separator = ";"
    if separator in [";", "\t"]:
        decimal = ","           #europejski format np 42,19
    else:
        decimal = "."           #amerykanski format np 42.19
    df = pd.read_csv(file_path, sep=separator, decimal=decimal, encoding="utf-8-sig")

    return df

#wczytywanie pliku xlsx. Excel przechowuje typy danych wiec nie trzeba wykrywac separatorow

def _load_xlsx(file_path: str) -> pd.DataFrame:
    df = pd.read_excel(file_path, engine="openpyxl")
    return df

#normalizowanie kolumn
def _normalize_columns(df: pd.DataFrame) -> pd.DataFrame:
    df.columns = df.columns.str.lower().str.strip()

    #szukanie kolumny czasowej
    time_column = None
    for col in df.columns:
        if col in TIME_KEYWORDS:
            time_column = col
            break                   # znaleziono kolumne czasową

    if time_column is None:
        raise ValueError(
            f"Nie znaleziono kolumny czasowej."
            f"Nazwy kolumn w pliku: {list(df.columns)}."
            f"Oczekiwane: {TIME_KEYWORDS}"
        )

    if time_column != "timestamp":
        df = df.rename(columns={time_column: "timestamp"})
    return df

#zamienia kolumne "timestmap" na liczbe (panads datetime64), jesli nie jest juz numeryczna.
#pliki testowe(np. temperature_data.csv) maja czas juz jako liczbe - zostawiamy bez zmian
#prawdziwe eksporty maja czas jako date tekstową  np. 2026-07-31 08:37:03,144012"-
#przechowuje ja jako prawdziwa date(zamieniam na rzeczywistą liczbe), zeby latwo bylo synchronizowac
#kilka osobnych plikow po rzeczywistym czasie - patrz - load_and_merge_files() nizej
#zamiana na liczbe (ms) dzieje się dopiero w algorytmie tuz przed liczeniem dt - patrz timestamps_to_ms()
def _convert_timestamp_column (df: pd.DataFrame) -> pd.DataFrame:
    col = df["timestamp"]

    if pd.api.types.is_numeric_dtype(col):
        return df

    #napierw probuje formatu z PI (przecinek jako separator ułamka sekundY)
    parsed = pd.to_datetime(col, format="%Y-%m-%d %H:%M:%S,%f", errors="coerce")

    #jesli cos nie pasowalo do tego formatu, probujemy ogolnego rozpownania pandas

    if parsed.isna().any():
        parsed = pd.to_datetime(col,errors="coerce")

    if parsed.isna().any():
        raise ValueError(
            "Nie udało się rozpoznać formatu kolumny czasu "
            "(oczekiwano liczby lub daty w rozpoznawalnym formacie)."
        )

    df = df.copy()
    df["timestamp"] = parsed # epoch w ms jako float

    return df

##Zamienia kolumne "timestamp" (prawdziwa data albo juz liczba) na tablice liczb w milisekundach
#To jedyne miejsce w calym pipeline, gdzie prawdziwa data zamieniana jest na liczbe - wczytywanie 
#i laczenie plikow(load_and_merge_files) operuje na prawdziwych datach (latwiej synchronizwac)
#kilka zrodel danych), a dopiero algorytm (Kalman/EKF) potrzebuje liczby zeby policzyc
#dt = czas[i] - czas[i-1]
def timestamps_to_ms(series:pd.Series):
    if pd.api.types.is_datetime64_any_dtype(series):
        return(series.astype("datetime64[ns]").astype("int64")
               /1_000_000).to_numpy(dtype=float)
    return series.to_numpy(dtype=float)

#laczy kilka osobcnyhc plikow (kazdy z wlasna kolumna czasu i jedna kolumna wartosci - 
#typowy  pojedynczy eksport tagu PIVISION) w jeden DataFrame, dopasowujac wierszy po najblizszym
#czasie (pd.merge_asof). Osobne tagi w historianie prawie nigdy nie sa probkowane dokladnie w tych
#samych chwilach, dlatego to dopasowanie "po najblizszymc zasie", a nie proste sklejenie kolumna do kolumny
#wedlug pozycji w pliku
#
#sources:lista trojek(sciezka_do_pliku, nazwa_kolumnu_wartosci_w_tym_pliku,
#       docelowa_nazwa_tej_kolumny w polaczonych danych) - np. 
#       [("flow.csv"), "wartosc", "przeplyw"), "pressure.csv", "wartosc", "cisnienie")]
#tolerance: maksymalna dopuszczalna odlegosc czasowa miedzy dopasowanymi probkami
#       (np.pd_Timedelta(minutes=5)) - probki dalze niz to nie zostana ze soba polaczone 
#       (dostana NaN, ktory potem odfiltruje dropna() w algorytmie)
#       None - brak ograniczenia.

def load_and_merge_files(sources: list[tuple[str,str,str]],
                         tolerance: pd.Timedelta | None = None) -> pd.DataFrame:

    if not sources:
        raise ValueError("Potrzebny co najmniej jeden plik zrodlowy do połączenia.")
    merged = None
    for file_path, source_column, target_column in sources:
        df = load_file(file_path)

        if source_column not in df.columns:
            raise ValueError(
                f"Kolumna '{source_column}' nie istnieje w pliku {file_path}."
                f"Dostepne kolumny: {list(df.columns)}"
            )

        piece = (df[["timestamp", source_column]]
                 .rename(columns={source_column: target_column})
                 .dropna()
                 .sort_values("timestamp"))

        if merged is None:
            merged = piece
        else:
            merged = pd.merge_asof(merged, piece, on = "timestamp",
                                   direction="nearest", tolerance=tolerance)
    return merged


#blok testowy 

if __name__ == "__main__":
    import sys

    if len (sys.argv) < 2:
        print("Użycie: python data_loader.py <ścieżka_do_pliku>")
        print("Przykład: python data_loader.py ../temperature_data.csv")
        sys.exit(1)  #Zakoncz program z kodem bledy 1

    file_path = sys.argv[1]

    try:
        df = load_file(file_path)

        print (f"wczytano plik: {file_path}")
        print (f"liczba wierszy: {len(df)}")
        print (f"Kolumny: {list(df.columns)}")
        print()
        print("Pierwsze 5 wierszy: ")
        print(df.head())        #head zwraca pierwsze 5 wierszy
        print()
        print("Statystyki:")
        print (df.describe()) # describe zwraca min max mean std, itd.

    except (FileNotFoundError, ValueError) as e:

        print(f"Błąd:{e}")
        sys.exit(1)