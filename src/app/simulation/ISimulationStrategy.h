#ifndef ISIMULATIONSTRATEGY_H
#define ISIMULATIONSTRATEGY_H

#include "../datamodel/DataPoint.h"
#include <QList>

class ISimulationStrategy{
public:
    virtual ~ISimulationStrategy() = default;

    virtual double process (qint64 elapsedTime, const QList<DataPoint> & data) const = 0;
};

#endif // ISIMULATIONSTRATEGY_H
