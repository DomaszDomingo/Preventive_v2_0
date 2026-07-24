from abc import ABC, abstractmethod
import pandas as pd

class BaseAlgorithm(ABC):
    name: str = "base"

    @abstractmethod
    def run(self, df: pd.DataFrame, value_column: str) -> dict:
        raise NotImplementedError

    def to_result_dict(
            self,
            *,
            filtered:list | None = None,
            anomalies:list | None = None,
            forecasts:list | None = None,
            changepoints:list | None = None,
            metadata:dict | None = None,
            status: str = "success",
            error_message: str = "",
    ) -> dict:
        
        return{ 
            "algorithm":self.name,
            "status":status,
            "errorMessage":error_message,
            "filtered": filtered or [],                 #wysyla filtered lub pustą tablice to zabezpieczenie bo jezeli nie byloby [] to w 
                                                        #przypadku gdy w JSON nie byloby filtered to wyslałoby null, co jest bardziej czytelne
                                                        #i wiem ze wysylam pusta tablice 
            "anomalies": anomalies or [],
            "forecasts": forecasts or[],
            "changepoints": changepoints or [],
            "metadata": metadata or {},
        }
    