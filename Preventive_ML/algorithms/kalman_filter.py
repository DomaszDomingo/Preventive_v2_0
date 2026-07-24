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
        return float (self.x[0,0])
            