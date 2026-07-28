import numpy as np
import pandas as pd

from .base import BaseAlgorithm

class KalmanFilter1D:
    """
    State - wektor dwuelementowy: 
    x - [ wartosc sygnalu, tempo zmiany(trend)]

    Zakladam, ze miedzy pomiarami wartosc zmienia sie w przybliżeniu liniowo 
    (value_next = value + velocity * dt), a samo velocity (trend) zmienia się powoli
    i losowo - jak  bardzo losowo, mowi nam process_noise(q)

    """
    def __init__(self, initial_value: float, initial_velocity: float = 0.0,
                 process_noise: float = 1e-4, measurement_noise: float = 1.0,
                 initial_uncertainty: float = 1.0):
        #wektor stanu 2x1 [wartosc, predkosc]
        self.x = np.array([[initial_value], [initial_velocity]], dtype = float)

        #macierz kowariancji 2x2 - jak bardzi (nie)pewny jestem stanu
        #duza wartosc na starcie = "nie wiem jeszcze nic, musze zaufac pierwszym pomiarom"
        self.P = np.eye(2) * initial_uncertainty

        #q-"sila" szumu procesu: jak bardzo trend moze sie sam zmieniac między pomiarami (
        #(np. przez realne, niemodelowane zmiany w maszynie)

        self.q = process_noise

        #R - wariancja szumu pomiarowego (jak bardzo nie ufam czujnikowi)
        self.R = measurement_noise

    def predict(self, dt:float):
        """
        Krok predykcji: przesuwam stan do przodu o dt (sekund/jednostek czasu),
        bez uzicua nowego pomiaru. Niepewność (P) rośnie, bo tylko ekstrapolujemy
        """

        #Macierz przejscia stanu: value_next = value + velocity*dt, velocity_bnext = velocity
        
        F = np.array([[1.0, dt],
                      [0.0,1.0]])

        #Standardowa macierz szumu procesu dla modelu "bialy szum przyspieszenia" :
        #im wieksze dt, tym wiecej niepewnosci dokładamy do stanu.

        Q = self.q * np.array([[dt**3/3.0, dt**2/2.0],
                              [dt**2/2.0, dt]])

        self.x = F @ self.x
        self.P = F @ self.P @F.T + Q

    def update (self, measurement: float):
        """
        Krok korekty: mam nowy pomiar - poprawiam stan w jego kierunku, proporcjonalnie do wzmocnienia Kalmana (K)
        """
        H = np.array([[1.0, 0.0]])                  #obserwuje tylko wartosc, nie predkosc

        z = np.array([[measurement]])
        y = z - H @ self.x                          #innowacja: pomiar minus predykcja
        S = H @ self.P @ H.T + self.R               #niepewnośc innowacji
        K = self.P @H.T @ np. linalg.inv(S)         #wzmocnienie Kalmana

        self.x = self.x + K @ y
        self.P = (np.eye(2) - K @ H) @ self.P
    @property
    def value(self) -> float:
        return float (self.x[0,0])

    @property
    def velocity(self) -> float:
        return float (self.x[1,0])

    @property
    def value_variance(self) -> float:
        #niepewnosc estymowanej wartosci - do liczenia pasm ufnosci prognozy
        return float (self.P[0,0])

class KalmanFilterAlgorithm(BaseAlgorithm):
    name = "kalman"

    def __init__(self, process_noise: float = 1e-4, measurement_noise: float = 1.0,
                forecast_steps: int = 20, confidence_z: float = 1.96):
        self.process_noise = process_noise
        self.measurement_noise = measurement_noise
        self.forecast_steps = forecast_steps
         #1.96 odchylenia standardowego ~95% przedział ufności (rozkład normalny)
        self.confidence_z = confidence_z
    def run (self, df: pd.DataFrame, value_column: str) ->dict:
        try:
            data = df[["timestamp", value_column]].dropna().sort_values("timestamp")

            if data.empty:
                return self.to_result_dict(
                    status="error",
                        error_message=f"Brak danych w kolumnie '{value_column}'.",
                    )
            timestamps = data["timestamp"].to_numpy(dtype=float)
            values = data[value_column].to_numpy(dtype=float)

            kf = KalmanFilter1D(
                initial_value=values[0],
                process_noise=self.process_noise,
                measurement_noise=self.measurement_noise,
            )

                #pierwszy punkt: nie bylo zadnej korekty, wiec predicted to wartosc startowa.

            filtered = [{
                "timestamp": float (timestamps[0]),
                "value": float(values[0]),
                "predicted":kf.value,
            }]
                #przechodzimy przez cale historyczne dane: predict (na podstawie dt do poprzedniego punktu),
                #potem update (korekta o realny pomiar).
                #Jest to jednoczesnie "estymacja stanu" i weryfikacja" - każdy element i ma realna wartosc, to co przewidzial model

            for i in range (1, len(timestamps)):
                dt = timestamps[i] - timestamps [i-1]

                if dt<=0:
                    continue #pomijam duplikaty/nieposortowane znaczniki czasu
                kf.predict(dt)
                kf.update(values[i])

                filtered.append({
                    "timestamp":float(timestamps[i]),
                    "value":float(values[i]),
                    "predicted":kf.value,
                })

                    #RSME (root mean square error) miedzy realnymi wartosciami, a tym co przewidzial model
                    # - liczbowa miara jakosci dopasowania (weryfikacja)
            predicted_arr = np.array([p["predicted"] for p in filtered])
            actual_arr = np.array([p["value"] for p in filtered])
            rmse = float (np.sqrt(np.mean((predicted_arr - actual_arr) ** 2 )))

                    #Prognoza: same predict() bez update() ( nie ma przyszlych pomiarow ) -
                    #wartosc jedzie dalej zgodnie z ostatnio wyestymowanym trendem, 
                    #a niepewnosc rośnie z  kazdym krokiem (sta coraz szersze upper/lower)

            forecasts = []
            avg_dt = float (np.mean(np.diff(timestamps))) if len (timestamps) > 1 else 1.0
            last_timestamp = float (timestamps[-1])

            for step in range (1,self.forecast_steps +1):
                kf.predict(avg_dt)
                forecast_timestamp = last_timestamp + step * avg_dt
                std_dev = float (np.sqrt(kf.value_variance))

                forecasts.append({
                    "timestamp": forecast_timestamp,
                    "predicted": kf.value,
                    "upper": kf.value + self.confidence_z * std_dev,
                    "lower": kf.value - self.confidence_z * std_dev,
                })

            return self.to_result_dict(
                filtered=filtered,
                forecasts=forecasts,
                metadata={
                    "process_noise": self.process_noise,
                    "measurement_noise": self.measurement_noise,
                    "rmse": rmse,
                    "final_velocity": kf.velocity,
                    "avg_dt": avg_dt,                    },
                )
        except Exception as e:
            return self.to_result_dict(status="error", error_message= str(e))
            
            