from enum import Enum

from .base import BaseAlgorithm
from .kalman_filter import KalmanFilterAlgorithm
from.ekf_valve import EKFValveAlgorithm

class AlgorithmType (Enum):
    """
    Wyliczenie dostepnych typów algorytmów - odpowiednik enum uzywanego w c++,
    kazdy nowy algorytm dostanie swoja wartość
    """

    KALMAN = "kalman"
    EKF_Valve = "ekf_valve"

class AlgorithmFactory:
    """
    Odpowiednik C++ strategyfactory- w jednym miejscu decyduje kala klase algorytmu utworzyc na podstawie typu
    Dzieki temu reszta kodu nie musi znac wszystkich klas algorytmów
    """
    @staticmethod
    def create (algorithm_type: AlgorithmType, **kwargs) -> BaseAlgorithm:
        """
        ***kwargs pozwala funkcji nie wiedzieć ile parametrów funkcji ma byc w niej wykorzystanych i nie musi znac ich nazwy
        """

        if algorithm_type == AlgorithmType.KALMAN:
            return KalmanFilterAlgorithm(**kwargs)

        if algorithm_type == AlgorithmType.EKF_Valve:
            return EKFValveAlgorithm(**kwargs)
        
        raise ValueError (f"Nieznany typ algorytmu: {algorithm_type}")
        