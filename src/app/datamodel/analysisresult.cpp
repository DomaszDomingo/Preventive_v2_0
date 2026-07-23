#include "analysisresult.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>





AnalysisResult AnalysisResult::fromJson(const QByteArray &jsonData)
{
    AnalysisResult result;

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    //sprawdzenie czy nie ma bledu parsowania
    if (parseError.error != QJsonParseError::NoError){
        result.m_status = "error";
        result.m_errorMessage = "Błąd parsowani JSON: " + parseError.errorString();
        return result;
    }

    QJsonObject root = doc.object();
    result.m_algorithm = root["algorithm"].toString();
    result.m_status = root["status"].toString("error");
    result.m_errorMessage = root ["errorMessage"].toString();

    //Filtered
    QJsonArray filteredArr = root ["filtered"].toArray();
    for (const QJsonValue &val : filteredArr) {
        QJsonObject obj = val.toObject();
        FilteredPoint pt;
        pt.timestamp = obj["timestamp"].toDouble();
        pt.value = obj["value"].toDouble();
        pt.predicted = obj["predicted"].toDouble();
        result.m_filtered.append(pt);
    }

    //Anomalies
    QJsonArray anomaliesArr = root ["anomalies"].toArray();
    for (const QJsonValue &val : anomaliesArr) {
        QJsonObject obj = val.toObject();
        AnomalyPoint pt;
        pt.timestamp = obj["timestamp"].toDouble();
        pt.value = obj["value"].toDouble();
        pt.score = obj["score"].toDouble();
        result.m_anomalies.append(pt);
    }

    QJsonArray forecastsArr = root ["forecasts"].toArray();
    for (const QJsonValue &val : forecastsArr) {
        QJsonObject obj = val.toObject();
        ForecastPoint pt;
        pt.timestamp = obj["timestamp"].toDouble();
        pt.predicted = obj["predicted"].toDouble();
        pt.lower = obj["lower"].toDouble();
        pt.upper = obj["upper"].toDouble();
        result.m_forecasts.append(pt);
    }

    QJsonArray changepointsArr = root ["changepoints"].toArray();
    for (const QJsonValue &val : changepointsArr) {
        QJsonObject obj = val.toObject();
        ChangePoint pt;
        pt.timestamp = obj["timestamp"].toDouble();
        result.m_changepoints.append(pt);
    }

    QJsonObject metaObj = root ["metadata"].toObject();
    result.m_metadata = metaObj.toVariantMap();

    return result;

}

AnalysisResult AnalysisResult::error(const QString &algorithm, const QString &message)
{
    AnalysisResult result;
    result.m_algorithm = algorithm;
    result.m_status = "error";
    result.m_errorMessage = message;
    return result;
}

QString AnalysisResult::algorithm() const { return m_algorithm; }
QString AnalysisResult::status() const { return m_status; }
QString AnalysisResult::errorMessage() const { return m_errorMessage; }

QList <FilteredPoint> AnalysisResult::filtered() const { return m_filtered; }
QList <AnomalyPoint> AnalysisResult::anomalies() const { return m_anomalies; }
QList <ForecastPoint> AnalysisResult::forecasts() const { return m_forecasts; }
QList <ChangePoint> AnalysisResult::changepoints() const { return m_changepoints; }
QVariantMap AnalysisResult ::metadata() const { return m_metadata; }

bool AnalysisResult::isSuccess() const { return m_status == "success"; }
bool AnalysisResult::hasFiltered() const { return !m_filtered.isEmpty(); }
bool AnalysisResult::hasAnomalies() const { return !m_anomalies.isEmpty(); }
bool AnalysisResult::hasForecasts() const { return !m_forecasts.isEmpty(); }
bool AnalysisResult::hasChangepoints() const { return !m_changepoints.isEmpty(); }

