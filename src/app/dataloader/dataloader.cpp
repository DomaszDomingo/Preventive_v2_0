#include "dataloader.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

DataLoader::DataLoader(QObject *parent) : QObject(parent) {}

void DataLoader::loadFromCSV(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Could not open file:" << filePath;
        return;
    }

    QTextStream in(&file);
    QList<DataPoint> dataList;

    // 1. Read Header to determine format
    QString headerLine = in.readLine();

    // HEURISTIC 1: Detect Separator
    // We count occurrences. If ';' is present, it's likely European CSV.
    QChar separator = ',';
    if (headerLine.count(';') > headerLine.count(',')) {
        separator = ';';
    } else if (headerLine.count('\t') > headerLine.count(',')) {
        separator = '\t';
    }

    // HEURISTIC 2: Map Columns
    // We split the header and look for keywords to identify columns.
    QStringList headers = headerLine.split(separator);
    int timeIndex = -1;
    int valueIndex = -1;

    for (int i = 0; i < headers.size(); ++i) {
        QString h = headers[i].toLower().trimmed();

        // Keywords for Time
        if (h == "time" || h == "timestamp" || h == "czas" || h == "t" || h.contains("sec"))
            timeIndex = i;

        // Keywords for Value
        else if (h == "value" || h == "signal" || h == "wartość" || h == "val" || h == "v")
            valueIndex = i;
    }

    // Fallback: If no headers found (or file has no header), assume 0=Time, 1=Value
    bool hasHeader = (timeIndex != -1 || valueIndex != -1);
    if (!hasHeader) {
        timeIndex = 0;
        valueIndex = 1;
        // Reset stream to beginning if we suspect the first line was actually data
        in.seek(0);
    }

    // 2. Parse Data
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;

        QStringList parts = line.split(separator);

        // Safety check: do we have enough columns?
        if (parts.size() <= std::max(timeIndex, valueIndex)) continue;

        // HEURISTIC 3: Universal Number Parsing
        // We replace ',' with '.' to standardize (C++ uses dot for doubles).
        // This handles "12,34" (EU) and "12.34" (US) correctly.
        QString timeStr = parts[timeIndex].replace(',', '.');
        QString valStr = parts[valueIndex].replace(',', '.');

        bool tOk, vOk;
        double t = timeStr.toDouble(&tOk);
        double v = valStr.toDouble(&vOk);

        if (tOk && vOk) {
            dataList.append({ (qint64)t, v });
        }
    }

    file.close();
    if (!dataList.isEmpty()) {
        emit dataLoaded(dataList);
    } else {
        qWarning() << "Plik załadowany, ale nie znaleziono żadnych prawidłowych punktów danych.";
    }
}
