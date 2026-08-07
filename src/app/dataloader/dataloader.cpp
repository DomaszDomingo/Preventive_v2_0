#include "dataloader.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDateTime>

//usuwa otaczajace cudzysłowy z pola CSV (eksoprty z PI cytuja kazta wartosc)
namespace {
QString stripQuotes ( const QString & field)
{
    QString s = field.trimmed();
    if (s.length() >= 2 && s.startsWith('"') && s.endsWith('"'))
        s = s.mid(1, s.length() - 2 );

    return s;
}

//probuje sparsowac znacznik czasu jako ficzbe (ms/s - pliki tekstowe).
//jesli to sie nie uda, probuje formatu daty yyyy-MM-dd HH:mm:ss, ulamek (jak w pi)
//gdzie czesc po przecinku to mikrosekundy o zmiennej dlugosci

qint64 parseTimestampMs (const QString &raw, bool *ok)
{
    QString s = stripQuotes(raw);

    //Najpierw proba czystej liczby - najczestszy przypadek

    QString numericCandidate = s;
    numericCandidate.replace(',', '.');
    double asNumber = numericCandidate.toDouble(ok);
    if (*ok)
        return static_cast<qint64>(asNumber);

//Nie liczba - probujemy date tekstowa

    QString datePart = s;
    QString fractionPart;
    int commaIndex = s.indexOf(',');
    if (commaIndex >= 0){
        datePart = s.left(commaIndex);
        fractionPart = s.mid(commaIndex + 1);
    }

    QDateTime dt = QDateTime::fromString (datePart.trimmed(), "yyyy-MM-dd HH:mm:ss");
    if (!dt.isValid()){
        *ok = false;
        return 0;
    }

    qint64 ms = dt.toMSecsSinceEpoch();

    if(!fractionPart.isEmpty()){
        //dopelniam do 6 cyfr (mikrosekundy), bierzemy pierwsze 3 jako milisekundy
        QString padded = fractionPart.leftJustified(6, '0');
        ms += padded.left(3).toInt();
    }

    *ok = true;
    return ms;
}

}

DataLoader::DataLoader(QObject *parent) : QObject(parent) {}

void DataLoader::loadFromCSV(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString msg = "Nie można otworzyć pliku: " + filePath;
        qCritical() << msg;
        emit errorOccured(msg);
        return;
    }

    QTextStream in(&file);
    QList<DataPoint> dataList;

    // 1. Odczytanie do okreslonego formatu
    QString headerLine = in.readLine();

    // Wykrycie seperatora
    // Licze ile jest ;
    QChar separator = ',';
    if (headerLine.count(';') >= headerLine.count(',')) {
        separator = ';';
    } else if (headerLine.count('\t') > headerLine.count(',')) {
        separator = '\t';
    }

    // Mapowanie kolumn
    // Dziele naglowek zeby zidenfytikować kolumny
    QStringList headers = headerLine.split(separator);
    int timeIndex = -1;
    int valueIndex = -1;

    for (int i = 0; i < headers.size(); ++i) {
        QString h = stripQuotes(headers[i]).toLower().trimmed();

        //Kluczowe slowa dla czasu
        if (h == "time" || h == "timestamp" || h == "czas" || h == "t" || h.contains("sec"))
            timeIndex = i;

        //Kluczowe slowa dla wartosci
        else if (h == "value" || h == "signal" || h == "wartość" || h == "val" || h == "v")
            valueIndex = i;
    }

    // jezeli nie znaleziono naglowkow (lub plik nie ma naglowka) zakladam czas = 0, wartosc =1
    bool hasHeader = (timeIndex != -1 || valueIndex != -1);
    if (!hasHeader) {
        timeIndex = 0;
        valueIndex = 1;
        // Reset strumienia jezeli peirwsza linia jest wlasciwa datą
        in.seek(0);
    }

    // 2. Parsowanie danych
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;

        QStringList parts = line.split(separator);

        // Sprawdzenie czy jest wystarczajaca ilosc kolimn
        if (parts.size() <= std::max(timeIndex, valueIndex)) continue;

        bool tOk = false, vOk = false;
        qint64 t = parseTimestampMs(parts[timeIndex], &tOk);

        QString valStr = stripQuotes(parts[valueIndex]);
        valStr.replace(',','.');
        double v = valStr.toDouble(&vOk);

        if (tOk && vOk){
            dataList.append({t,v});
        }


    }

    file.close();
    if (!dataList.isEmpty()) {

        qint64 t0 = dataList.first().timestamp;
        for (DataPoint &point : dataList){
            point.timestamp -= t0;
        }

        emit dataLoaded(dataList);
    } else {
        qWarning() << "Plik załadowany, ale nie znaleziono żadnych prawidłowych punktów danych.";
    }
}
