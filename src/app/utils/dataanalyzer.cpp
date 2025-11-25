#include "dataanalyzer.h"
#include <QFileInfo>


SimulationStats DataAnalyzer::analyze(const QList<DataPoint> &data, const QString &filePath)
{
    SimulationStats stats;
    stats.fileName = filePath;
    stats.reset();

    if (data.isEmpty()){
        return stats;
    }

    stats.count = data.size();
    stats.startTime = data.first().timestamp;
    stats.endTime = data.last().timestamp;

    double sum = 0.0;

    for (const auto & point : data){
        if (point.value < stats.minValue) stats.minValue = point.value;
        if (point.value > stats.maxValue) stats.maxValue = point.value;
        sum += point.value;
    }

    if (stats.count > 0){
        stats.avgValue = sum/stats.count;
    }

    return stats;
}
