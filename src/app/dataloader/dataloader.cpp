#include "dataloader.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

DataLoader::DataLoader(QObject *parent) : QObject(parent) {}

void DataLoader::loadFromCSV(const QString &filePath)
{
    QFile file (filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        emit errorOccured(tr ("Nie można otworzyć pliku: %1").arg(filePath));
        return;

    }

    QList<DataPoint> data;
    QTextStream in (&file);

    //Pomijamy nagłówek, jeśli istnieje
    //if (!in.atEnd()) in.readLine();

    while (!in.atEnd()){
        QString line = in.readLine();
        QStringList parts = line.split(",");
        if (parts.size() == 2){
            bool ok1, ok2;
            qint64 timestamp = parts[0].toLongLong(&ok1);
            double value = parts [1].toDouble(&ok2);

            if (ok1 && ok2){
                data.append({timestamp, value});
            }
        }
    }

    qInfo() << "Pomyślnie załadowano" << data.size() << "dane punktów.";
    emit dataLoaded (data);
}
