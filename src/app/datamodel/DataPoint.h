#ifndef DATAPOINT_H
#define DATAPOINT_H

#include <qglobal.h>

struct DataPoint {
    qint64 timestamp;
    double value;
};

#endif // DATAPOINT_H
