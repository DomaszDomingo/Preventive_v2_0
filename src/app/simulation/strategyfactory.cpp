#include "strategyfactory.h"
#include "concretestrategies.h"


std::unique_ptr<ISimulationStrategy> StrategyFactory::create(StrategyType type)
{
    switch (type) {
    case StrategyType::Discrete:
        return std::make_unique<DiscreteStepStrategy>();
    case StrategyType::Linear:
        return std::make_unique<LinearInterpolationStrategy>();
    default:
        return nullptr;
    }
}
