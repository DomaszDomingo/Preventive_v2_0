#ifndef STRATEGYFACTORY_H
#define STRATEGYFACTORY_H
#include "ISimulationStrategy.h"
#include <memory>

enum class StrategyType { Discrete, Linear };

class StrategyFactory
{
public:
    //Statyczna metoda tworząca strategię na podstawie typu.
    //Zwraca std::unique_ptr, co jasno komunikuje przeniesienie własności

    static std::unique_ptr<ISimulationStrategy> create (StrategyType type);
};

#endif // STRATEGYFACTORY_H
