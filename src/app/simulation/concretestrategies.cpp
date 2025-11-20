#include "concretestrategies.h"
#include <algorithm>

double DiscreteStepStrategy::process(qint64 elapsedTime, const QList<DataPoint> &data) const
{
    if (data.isEmpty()) return 0.0;

    // Znajdź pierwszy element, którego timestamp jest WIĘKSZY niż elapsedTime
    auto it = std::upper_bound(data.constBegin(), data.constEnd(), elapsedTime, [](qint64 time, const DataPoint & point){
        return time < point.timestamp;
    });

     // Jeśli `it` to początek, znaczy że jesteśmy przed pierwszym punktem.
    if (it == data.constBegin()) {
        return data.first().value;
    }

    // W przeciwnym razie, `it-1` wskazuje na ostatni punkt, którego nie minęliśmy.
    return (it - 1)->value;
}


double LinearInterpolationStrategy::process(qint64 elapsedTime, const QList<DataPoint> &data) const
{
    if (data.isEmpty()) return 0.0;
    if (elapsedTime <= data.first().timestamp) return data.first().value;
    if (elapsedTime >= data.last().timestamp) return data.last().value;

    auto it = std::upper_bound(data.constBegin(), data.constEnd(), elapsedTime, [](qint64 time, const DataPoint & point){
        return time < point.timestamp;
    });

    const DataPoint & p2 = *it;
    const DataPoint & p1 = *(it - 1);

    //wzór na interpolację liniową
    double t = static_cast <double> (elapsedTime - p1.timestamp) / static_cast<double>(p2.timestamp - p1.timestamp);
    return p1.value + t * (p2.value - p1.value);
}
