#ifndef DATAANALYZER_H
#define DATAANALYZER_H

#include <QString>
#include <QList>
#include <limits>
#include "../datamodel/DataPoint.h"

struct SimulationStats {
    QString fileName;
    int count = 0;
    qint64 startTime = 0;
    qint64 endTime = 0;
    double minValue = 0.0;
    double maxValue = 0.0;
    double avgValue = 0.0;

    void reset (){
        count = 0;
        startTime = 0;
        endTime = 0;
        minValue = std::numeric_limits<double>::max();
        maxValue = std::numeric_limits<double>::lowest();
        avgValue = 0.0;
    }
};

class DataAnalyzer
{
public:
    static SimulationStats analyze (const QList<DataPoint> & data, const QString& filePath);
};

#endif // DATAANALYZER_H
