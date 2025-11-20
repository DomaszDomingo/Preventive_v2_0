#ifndef CONCRETESTRATEGIES_H
#define CONCRETESTRATEGIES_H
#include "ISimulationStrategy.h"

class DiscreteStepStrategy : public ISimulationStrategy {
public:
    double process (qint64 elapsedTime, const QList<DataPoint> &data) const override;
};

class LinearInterpolationStrategy : public ISimulationStrategy{
public:
    double process (qint64 elapsedTime, const QList<DataPoint> &data) const override;
};

#endif // CONCRETESTRATEGIES_H
