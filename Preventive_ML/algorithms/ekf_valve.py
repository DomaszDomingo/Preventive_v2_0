"""
Extendetd Kalman Filter (EKF) do estymacji stanu zaworu regulacyjnego (otwarcie 0-100%)
na podstawie jednego lub wiecej pomiarow procesowych

Architektura jest podzielona na dwie warstwy: 
1. ExtendedKalmanFiterValve - generyczny rdzen EKF. Zna tylko 
stan [otwarcie, predkość zmiany otwarcia] i mechanikę predict/update
Nic nie wie o tym, co dokladnie jest mierzone

2. Measurement Channel - interfejs pojedynczego pomiaru. Każdy kanał mówi filtrowi
dwie rzeczy: "jaka wartosc bym zmierzyl, gdyby zawor byl otwarty w tym stopniu"
(predict/h(x)) i jak bardzo ta wartosc zmienia sie przy malej zmianie otwarcia
(jacobian/dh/dx) - to jest ta wlasnie Extended czesc EKF, bo ta zaleznosc jest nieliniowa
i trzeba ja linearyzowac na nowo w kazdym kroku

Dzieki temu podzialowi dodanie kolejnego pomiaru w przyszlosci (temperatura, pozycja trzpienia, drgania,
prąd siłownika ...) to napisanie nowej klasy dziedzicączej po MeasurementChannel i dopisanie
jej do listy 'channels' w EKFValveAlgorithm.run() - rdzen filtra nie wymaga żadnych zmian"""

from abc import ABC, abstractmethod

import numpy as np
import pandas as pd

from .base import BaseAlgorithm
from data_loader import timestamps_to_ms

def _clamp (value: float, lo: float, hi: float) -> float:
    return max(lo, min (hi,value))

###Rdzen filtra - nic nie wie o czujnikach

class ExtendedKalmanFilterValve:
    """
    Stan - wektor dwuelementowy:
    x = [otwarcie zaworu (ulamek 0.0 - 1.0), predkosc zmiany otwarcie (1/s)]
    
    Model procesu (predict) jest liniowy (otwarcie zmienia się w przyblizeniu 
    liniowo, prędkość to powolny losowy "dryf") - jedyna nieliniowość w tym kroku 
    to fizyczne ograniczenie 0-100%. Cała reszta nieliinoiwości (zwiazana z tym 
    Co i Jak meirzymy) jest ukryta klasach MeasurementChannel - ten rdzen 
    dostaje od kanału juz gotowe h(x) i Jakobian, wiec działa identycznie,
    niezależnie od tego ile i jakich kanałow jest w danym momencie podłączonych
    """

    def __init__(self, initial_opening: float, initial_opening_rate: float = 0.0,
        process_noise: float = 1e-6, initial_uncertainty: float = 0.1):
        #wektor stan 2x1 (otwarcie, prędkosc otwierania)
        self.x = np. array([[_clamp(initial_opening, 0.0, 1.0)],
                        [initial_opening_rate]],dtype=float)

        #macierz kowariancji 2x2 - początkowo duża niepewność, bo otwarcie zaworu
        #na starcie tylko zgadujemy (patrz _estimate_initial_opening() nizej)
        self.P = np.eye(2) *initial_uncertainty

        #q - szum procesu: jak bardzo predkość otwierania może sama dryfować
        #miedzy pomiarami (niemodelowana reakcja regulatora/operatora)
        self.q = process_noise
    def predict (self, dt:float):
        """Przesuwa stan do przodu o dt bez uzycia nowego pomiaru."""
        F = np.array([[1.0, dt],
                    [0.0, 1.0]])

        Q = self.q * np.array([[dt**3/3.0, dt**2/2.0],
                            [dt**2/2.0, dt]])

        self.x = F @ self.x
        self.P = F @ self.P @ F.T + Q

        #fizyczne ograniczenie zaworu (0-100% skoku.) To jest realna nieliniowość
        #modelu procesu - dla prostoty przycinamy tylko stan, a niepewnosc 
        #liczymy tak jakby nasycenia nie bylo (lekko zawyza P blisko granic - 
        #to bezpieczna strona pomyłki, nie niebezpieczna)

        self.x [0,0] = _clamp(self.x[0,0], 0.0, 1.0)

    def update(self, measurement: float, predicted: float, jacobian: float, noise_variance: float):
        """
        Jeden krok korekty EKF - generyczny, działa dla dowolnego kanału pomiarowego.
        Każdy MeasurementChannel dostarcza tylko sam pomiar (z), przewidywana wartość h(x),
        Jakobian dh/d(otwarcie i wariancje szumu R swojego czujnika)"""

        H = np.array([[jacobian, 0.0]])

        z = np.array([[measurement]])
        y = z - np.array([[predicted]])                         #innowacja: pomiar minus predykcja
        S = H @ self.P @H.T + noise_variance                    #niepewnosc innowacji
        K = self.P @ H.T @ np.linalg.inv(S)                     #wzmocnienie kalmana

        self.x = self.x + K @ y
        self.P = (np.eye(2) - K @ H) @ self.P
        self.x [0,0] = _clamp(self.x[0,0], 0.0, 1.0)

    def update_from_channel (self, channel: "MeasurementChannel", measurement: float, row: dict):
        """Wygodny skrot: liczy h(x) i jakobian z kanału, po czym woła update()."""
        opening = self.x [0,0]
        predicted = channel.predict(opening, row)
        jac = channel.jacobian(opening, row)
        self.update (measurement, predicted, jac, channel.noise_variance())

    @property
    def opening (self) -> float:
        return float(self.x[0,0])
    @property
    def opening_rate(self)-> float:
        return float(self.x[1,0])
    @property
    def opening_variance(self) -> float:
        return float(self.P[0,0])

# Kanały pomiarowe - tu jest cała wiedza o fizyce konkretnych czujników

class ValveCharacteristic (ABC):
    """Krzywa zaworu: jak procentowe otwarcie przeklada sie na ulamek Cv_max"""
    @abstractmethod
    def value (self, opening: float) -> float:
        """f(otwarcie) - ulamek cv_max przy danym otwarciu (0.0-1.0)"""
        raise NotImplementedError

    @abstractmethod
    def derivative (self, opening: float) -> float:
        #df/d(otwarcie) - potrzebne EKF do jakobianu
        raise NotImplementedError

    @abstractmethod
    def invert (self, ratio: float) -> float:
        """odwrotnosc f(): na podstawie stosunku (zmierzony przeplyw/cv_max*sqqrt(dP/SG))
        zgaduje otwarcie. Uzywane tylko raz do ustawienia stanu poczatkowego"""
        raise NotImplementedError

class LinearCharacteristic(ValveCharacteristic):
    #przeplyw rosnie proporcjonalnie do otwarcia f(o) = o

    def value (self, opening: float) -> float:
        return _clamp(opening, 0.0, 1.0 )

    def derivative (self, opening: float) -> float:
        return 1.0

    def invert (self, ratio: float) -> float:
        return _clamp (ratio, 0.0, 1.0)

class EqualPercentageCharacteristic(ValveCharacteristic):
    """Typowa charaktersytyka zaworów regulacyjnych: przy małym otwarciu mały przyrost
    skoku daje maly przyrost przepływu, przy duzym otwarciu duzy f(o) = R^(o-1)"""

    def __init__(self, rangeability: float = 50.0):
        self.rangeability = rangeability

    def value (self, opening: float) -> float:
        opening = _clamp(opening, 0.0, 1.0)
        return float (self.rangeability ** (opening - 1.0))

    def derivative(self, opening: float) -> float:
        opening = _clamp(opening, 0.0, 1.0)
        return float(np.log(self.rangeability) * self.rangeability **(opening - 1.0))

    def invert(self, ratio: float) -> float:
        if ratio <= 0:
            return 0.0
        return _clamp(1.0+ float(np.log(ratio) / np.log(self.rangeability)),0.0, 1.0)

def _build_characteristic(name: str, rangeability: float) -> ValveCharacteristic:
    if name == "equal_percentage":
        return EqualPercentageCharacteristic(rangeability)
    if name == "linear":
        return LinearCharacteristic()
    raise ValueError(f"Nieznana charakterystyka zaworu: {name}")

class MeasurementChannel (ABC):
    """
    Nazwy kolumn z DataFrame potrzebne temu kanałowi. Pierwsza kolumna to zawsze sam
    pomiar (to co filtr porównuje z przewidywaniem), kolejne to opcjonalne wejscia pomocnicze 
    do policzenia przewidywania (np. ciśnienie)
    """

    name: str

    @abstractmethod
    def required_columns(self) -> list [str]:
        """
        Nazwy kolumn z DataFrame potrzebne temu kanałowi. Pierwsza to zawsze
        sam pomiar(to co jest porownane z przewidywaniami), kolejne to opcjonalne 
        wejscia pomocniczne potrzebne do policzenia przewidywania (np. cisnienia)
        """
        raise NotImplementedError

    @abstractmethod
    def predict(self, opening: float, row: dict) -> float:
        #h(x) - przewidywana wartosc pomairdu dla otwarcia zaworu.
        raise NotImplementedError

    @abstractmethod
    def jacobian(self, opening: float, row: dict) -> float:
        #dh/d(otwarcie) - potrzebne EKF do linearyzacji w kazdym kroku.
        raise NotImplementedError

    @abstractmethod
    def noise_variance(self) -> float:
        """R - jak bardzo nie ufam konkretnemu czujnikowi"""
        raise NotImplementedError


    

class FlowPressureChannel(MeasurementChannel):
    """
    Kanał "przepływ + cisnienie roznicowe" - rownanie zaworu regulacyjnego
    """
    name = "flow_pressure"

    def __init__ (self, flow_column: str, pressure_column :str,
                    characteristic: ValveCharacteristic,
                    cv_max: float = 100.0, specific_gravity: float = 1.0,
                    measurement_noise: float = 1.0):
        self.flow_column = flow_column
        self.pressure_column = pressure_column
        self.characteristic = characteristic
        self.cv_max = cv_max
        self.specific_gravity = specific_gravity
        self.measurement_noise = measurement_noise

    def required_columns(self) -> list[str]:
        return [self.flow_column, self.pressure_column]

    def _sqrt_dp_(self, row: dict) -> float:
        #ujemne deltaP (np. przez szum pomiarowy) nie ma tu fizycznego sensu
        dp = max(row[self.pressure_column], 0.0)
        return float(np.sqrt(dp/self.specific_gravity))
    
    def predict (self, opening: float, row: dict) -> float:
        return self.cv_max * self.characteristic.value(opening) * self._sqrt_dp_(row)

    def jacobian (self, opening: float, row: dict) -> float:
        return self.cv_max * self.characteristic.derivative(opening) * self._sqrt_dp_(row)

    def noise_variance (self) -> float:
        return self.measurement_noise

def _estimate_initial_opening (cv_max: float, specific_gravity: float, dp0: float,
                               q0: float, characteristic: ValveCharacteristic) -> float:
     """
     Na stacie nie znam pozycji zaworu - zgaduje ją, odwracając równanie zaworu
     dla pierwszej próbki. To tylko punkt startowy filtra - stąd wysoka 
     initial_uncertainty w konstruktorze EKF, zeby filtr szybko zauwal pierwszym pomiarom
     """

     dp_eff = max (dp0, 1e-6)
     denom = cv_max * float (np.sqrt(dp_eff / specific_gravity))
     if denom <= 0:
         return 0.5
     return characteristic. invert (q0 / denom)


# Algorytm - spina rdzen EKF z lista kanałów w formacie zdgodnym z BaseAlgorithm

class EKFValveAlgorithm (BaseAlgorithm):
    name = "ekf_valve"

    def __init__(self, cv_max: float = 100.0, specific_gravity: float = 1.0,
                         valve_characteristic: str = "linear", rangeability: float = 50.0,
                         process_noise: float = 1e-6, measurement_noise: float = 1.0,
                         forecast_steps: int = 20, confidence_z: float = 1.96):
        self.cv_max = cv_max
        self.specific_gravity = specific_gravity
        self.valve_characteristic = valve_characteristic
        self.rangeability = rangeability
        self.process_noise = process_noise
        self.measurement_noise = measurement_noise
        self.forecast_steps = forecast_steps
        self.confidence_z = confidence_z
    def run (self, df: pd.DataFrame, feature_columns: list [str],
             target_column: str | None = None) -> dict:

        if len(feature_columns) < 2:
            return self.to_result_dict(
                status="error",
                error_message ="EKF Zaworu wymaga co najmniej dwoch kolumn: "
                                "feature-columns = [przepływ, cisnienie_roznicowe].",
            )

        flow_column, pressure_column = feature_columns[0], feature_columns [1]

        #Lista aktywnych kanałow pomiarowych. Teraz mam jeden (przeplyw + cisnienie)
        #Aby dodac kolejny pomiar w przyszlosci nalezy napisac nowa klase dziedziczącą
        #po MeasurementChannel (required_columns/ predict/jacobian/noise_variance)
        #i dopisac jej instancje do tej listy. Petla predict/update ponizej nie wymaga zanych
        #zmian - iteruje po channels, niezaleznie od tego ile ich jest

        characteristic = _build_characteristic(self.valve_characteristic, self.rangeability)
        channels: list[MeasurementChannel] = [
            FlowPressureChannel(
                flow_column, pressure_column, characteristic,
                cv_max= self.cv_max, specific_gravity=self.specific_gravity,
                measurement_noise=self.measurement_noise,
            ),
        ]
        primary_channel = channels[0] # kanał raportowany w "value/predicted (do wykresu)"

        try:
            needed_columns = ["timestamp"]
            for ch in channels:
                for col in ch.required_columns():
                    if col not in needed_columns:
                        needed_columns.append(col)

            data = df[needed_columns]. dropna().sort_values("timestamp")
            if data.empty:
                return self.to_result_dict(
                    status="error",
                    error_message="Brak kompletnych danych dla skonfigurowanych kanałów pomiarowych.",
                )
            #lista slowników {nazwa_kolumny: wartosc}, po jednym na probke -
            #wygodne, bo kazdy kanal moze wyciagnac wlasne kolumny po nazwie

            data = data.copy()
            data ["timestamp"] = timestamps_to_ms(data["timestamp"])
            #lista slownikow {nazwa_kolumny:wartosc}, po jednym na probke -
            #wygodne, bo kazdy kanal moze wyciagnac wlasne kolumny po nazwie
            
            rows = data.to_dict("records")

            initial_opening = _estimate_initial_opening(
                self. cv_max, self.specific_gravity,
                rows[0][pressure_column], rows[0][flow_column],
                characteristic
                )

            ekf = ExtendedKalmanFilterValve(
                initial_opening = initial_opening,
                process_noise = self.process_noise,
            )

            def channel_snapshot(row: dict) -> dict:
                #h(x) i pomiar dla kazdego kanału, przy aktualnyms tanie EKF
                return {
                    ch.name: {
                        "measured": row [ch.required_columns()[0]],
                        "predicted": ch.predict(ekf.opening, row),
                    }
                    for ch in channels
                }
            #pierwszy punkt: brak jeszcze zadnej korekty, predicted liczone
            #z początkowego (zgadywanego) otwarcia

            snapshot0 = channel_snapshot(rows[0])
            filtered = [{
                "timestamp": float (rows[0]["timestamp"]),
                "value": snapshot0[primary_channel. name]["measured"],
                "predicted": snapshot0[primary_channel.name]["predicted"],
                "estimatedOpeningPct": ekf.opening * 100,
                "channels": snapshot0,
            }]

            for i in range (1, len(rows)):
                dt = rows [i]["timestamp"] - rows [i - 1]["timestamp"]
                if dt <= 0:
                    continue #pomijam duplikaty/nieposortowane znaczniku czasu

                ekf.predict (dt)
                for ch in channels:
                    measure_col = ch.required_columns ()[0]
                    ekf.update_from_channel(ch, rows[i][measure_col], rows [i])

                snapshot = channel_snapshot(rows[i])
                filtered.append({
                    "timestamp": float (rows[i]["timestamp"]),
                    "value": snapshot[primary_channel.name]["measured"],
                    "predicted": snapshot[primary_channel.name]["predicted"],
                    "estimatedOpeningPct": ekf.opening * 100.0,
                    "channels":snapshot,
                })

            predicted_arr = np.array([p["predicted"] for p in filtered])
            actual_arr = np.array([p["value"] for p in filtered])
            rmse = float (np.sqrt(np.mean((predicted_arr - actual_arr) ** 2 )))

            #Prognoza : samo predict () bez update(), bo nie ma przyszlych pomiarow).
            #wejscia pomocnicze (np cisnienie nie sa znane na przyszlosc - jako uproszczenie
            # trzymam ostatni znany wiersz na stale (naiwne zalozenie, do poprawy, gdy sie pojawi osobny
            #model/prognoza dla tych wejsc)

            forecasts = []
            timestamps_arr = np.array([r["timestamp"] for r in rows], dtype=float)
            avg_dt = float (np.mean(np.diff(timestamps_arr))) if len (timestamps_arr) > 1 else 1.0
            last_row = rows [-1]

            for step in range (1, self.forecast_steps + 1):
                ekf.predict(avg_dt)
                forcast_timestamp = last_row["timestamp"] + step * avg_dt
                predicted = primary_channel.predict (ekf.opening, last_row)
                jac = primary_channel.jacobian(ekf.opening,last_row)
                std_dev = float (np.sqrt(max(( jac ** 2) * ekf.opening_variance, 0.0)))

                forecasts.append ({
                    "timestamp": forcast_timestamp,
                    "predicted": predicted,
                    "upper": predicted + self.confidence_z * std_dev,
                    "lower": predicted - self.confidence_z * std_dev,
                })

            return self.to_result_dict(
                filtered= filtered,
                forecasts=forecasts,
                metadata= {
                    "cv_max": self.cv_max,
                    "specific_gravity": self.specific_gravity,
                    "valve_characteristic": self.valve_characteristic,
                    "rangeability": self.rangeability,
                    "process_noise": self.process_noise,
                    "measurement_noise": self.measurement_noise,
                    "rmse": rmse,
                    "final_opening_pct": ekf.opening * 100.0,
                    "final_opening_rate": ekf.opening_rate,
                    "avg_dt": avg_dt,
                    "channels": [ch.name for ch in channels],
                },
            )            
            
        except Exception as e:
            return self.to_result_dict(status="error", error_message=str(e))
        

                    

            



    




