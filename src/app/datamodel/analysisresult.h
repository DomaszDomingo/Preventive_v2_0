#ifndef ANALYSISRESULT_H
#define ANALYSISRESULT_H

#include <QString>
#include <QList>
#include <QVariantMap>

//struktury danych bez logiki

struct FilteredPoint {
    double timestamp;
    double value;
    double predicted;
};

struct AnomalyPoint {
    double timestamp;
    double value;
    double score;
};

struct ForecastPoint {
    double timestamp;
    double predicted;
    double upper;
    double lower;
};

struct ChangePoint {
    double timestamp;
};

class AnalysisResult
{
public:
    AnalysisResult() = default;

    static AnalysisResult fromJson (const QByteArray &jsonData);
    static AnalysisResult error (const QString &algorithm, const QString &message);

    QString algorithm() const;
    QString status() const;
    QString errorMessage() const;

    QList<FilteredPoint> filtered() const;
    QList<AnomalyPoint> anomalies() const;
    QList<ForecastPoint> forecasts() const;
    QList<ChangePoint> changepoints() const;
    QVariantMap metadata() const;

    bool isSuccess() const;
    bool hasFiltered() const;
    bool hasAnomalies() const;
    bool hasForecasts() const;
    bool hasChangepoints() const;

private:
    QString m_algorithm;
    QString m_status;
    QString m_errorMessage;

    QList<FilteredPoint> m_filtered;
    QList<AnomalyPoint> m_anomalies;
    QList<ForecastPoint> m_forecasts;
    QList<ChangePoint> m_changepoints;
    QVariantMap m_metadata;




};

#endif // ANALYSISRESULT_H
